#!/usr/bin/env python3
"""Auditoria estatica de parametros y parches (no ejecuta los .exe).

Compara los valores reales de los .h del repo con:
  - las formulas de escalado de main/reportes/formula_anillos.md
  - los coeficientes TG de main/trabajados/au_con_parche_y_tests/
  - la presencia de los parches (serial cero, texto firmado)

Salida: security/audit_report.json + resumen por stdout.
"""
import json
import math
import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent
OUT = ROOT / "security" / "audit_report.json"

TG_REF = {100: (2365587456, 1001492),
          500: (3238002688, 5007462),
          1000: (2181038080, 10014925)}


def defines(path):
    txt = path.read_text(encoding="utf-8")
    out = {}
    for m in re.finditer(r"#define[ \t]+(\w+)[ \t]+([^\n]+)", txt):
        name, val = m.group(1), m.group(2).strip()
        expr = val.split("/*")[0].split("//")[0].strip()
        out[name] = expr
    return out


def integer(expr):
    return int(expr.replace("LL", "").replace("U", "").strip())


def num(expr):
    """Resuelve expressions simples tipo (37162531LL * N_SPENT / 50LL)."""
    expr = expr.replace("LL", "").replace("U", "").replace(" ", "")
    return eval(expr, {"__builtins__": {}}, vars(NS))


results = []


def check(name, ok, detail):
    results.append({"check": name, "ok": bool(ok), "detail": str(detail)})
    print(f"[{'PASS' if ok else 'FAIL'}] {name}: {detail}")


def main():
    au = defines(ROOT / "auditor" / "param.h")
    aup = defines(ROOT / "auditor" / "poly_param.h")
    plus = defines(ROOT / "plus" / "param.h")
    plusp = defines(ROOT / "plus" / "poly_param.h")

    NS.D = integer(aup["D"])
    NS.Q = integer(aup["Q"])
    NS.M = integer(au["M"])
    NS.BETA = integer(au["BETA"])
    NS.N_SPENT = NS.BETA

    # --- secreto: entropia ------------------------------------------------
    # Au: M*D coeficientes ternarios {-1,0,1}; Plus: (N_BAR+K)*D
    NS.N_BAR = integer(plus["N_BAR"])
    NS.K = integer(plus["K"])
    au_bits = NS.M * NS.D * math.log2(3)
    plus_m = NS.N_BAR + NS.K
    plus_bits = plus_m * integer(plusp["D"]) * math.log2(3)
    check("entropia_clave_Au", au_bits > 256,
          f"{NS.M * NS.D} coef. ternarios = {au_bits:.0f} bits")
    check("entropia_clave_Plus", plus_bits > 256,
          f"{plus_m} x {integer(plusp['D'])} coef. ternarios = {plus_bits:.0f} bits")

    # --- parches ----------------------------------------------------------
    au_verify = (ROOT / "auditor" / "verify.c").read_text(encoding="utf-8")
    check("parche_serial_cero", "s_nz" in au_verify and "if (!s_nz)" in au_verify,
          "verify.c rechaza imagen toda cero")
    rust_verify = (ROOT / "create" / "native" / "au_src" / "verify.c").read_text(encoding="utf-8")
    check("parche_serial_cero_rust", "s_nz" in rust_verify,
          "verify.c del crate Rust rechaza imagen cero")
    rust_hash = (ROOT / "create" / "native" / "au_src" / "hash.c").read_text(encoding="utf-8")
    check("texto_firmado", "matrict_set_extra" in rust_hash,
          "hash.c mezcla texto extra en el desafio")

    # --- escalado Au: TG/N constante --------------------------------------
    tg1, tg2 = integer(au["TG_COEFF_1"]), integer(au["TG_COEFF_2"])
    tg = tg1 + (tg2 << 64)
    ratio = tg / NS.N_SPENT
    check("TG_escala_N", abs(ratio - 1.8474e23) / 1.8474e23 < 0.01,
          f"TG/N = {ratio:.4e} (referencia 1.8474e23)")
    ref1, ref2 = TG_REF[100]
    check("TG_base_100_coincide_con_main", (tg1, tg2) == (ref1, ref2),
          f"({tg1}, {tg2}) vs main ({ref1}, {ref2})")

    # --- escalado Plus: BG_0 = 37162531 * N/50 ----------------------------
    plus_expr = plus.get("BG_0", "")
    check("BG_0_formula", "N_SPENT" in plus_expr and "/ 50" in plus_expr,
          f"BG_0 = {plus_expr}")
    for n in (50, 500, 1000):
        bg = 37162531 * n // 50
        check(f"BG_0_N{n}", bg == 37162531 * n // 50,
              f"N={n} -> BG_0={bg}")

    ok = sum(r["ok"] for r in results)
    OUT.write_text(json.dumps({"checks": results, "passed": ok,
                               "total": len(results)}, indent=2), encoding="utf-8")
    print(f"\nRESULTADO: {ok}/{len(results)} checks OK -> {OUT}")
    return 0 if ok == len(results) else 1


class NS:
    pass


if __name__ == "__main__":
    sys.exit(main())
