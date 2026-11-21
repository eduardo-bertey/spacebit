# Seguridad del repo: bases, parches y estimación

## 1. Sobre qué se basa el esquema

Este repo contiene dos esquemas de pagos con privacidad derivados de **MatRiCT** (IEEE S&P 2022, ePrint 2021/545):

| | Fuente | Base matemática |
|---|---|---|
| **Au** (auditable) | `auditor/` | firmas de anillo sobre **retículos (module-SIS / module-LWE)** + compromisos + prueba binaria de montos |
| **Plus** (común) | `plus/` | ídem, sin auditor; dos entradas por transacción |

El anillo permite gastar sin revelar qué llave del grupo se usó. La dureza viene de problemas de retículos: encontrar un vector corto en una red definida por el módulo `R_q = Z_q[X]/(X^d+1)`. Se considera **post-cuántico** (a diferencia de ECDSA-256, roto por Shor).

### Parámetros reales (leídos de los `.h`)

| | Au | Plus |
|---|---|---|
| `d` (dimensión del anillo) | 64 | 256 |
| `q` (módulo) | 2147221513 (~31 bits) | 3489661057 (~31.7 bits) |
| clave secreta | 38 polinomios × 64 coef. ternarios | 8 polinomios × 256 coef. ternarios |
| entropía de la clave | ~3855 bits | ~3246 bits |
| llaves por cuenta | 1 | 2 |
| anillo publicado | 100 (probado hasta 1000) | 50 (probado hasta 16384) |

La clave **no** es de 256 bits: los 256 bits son el seed AES-256 y el tamaño del serial/hash (`CRYPTO_BYTES=32`). La secreta es un vector largo de coeficientes `{-1,0,1}`.

## 2. Parches aplicados en este repo

1. **Imagen/serial cero rechazado** — `auditor/verify.c` y `create/native/au_src/verify.c`: si todos los coeficientes del serial son cero, `verify()` devuelve 0. Cierra el ataque con `h=0`, donde el serial sale cero para todos y el doble gasto se vuelve invisible (`th_b.c`).
2. **Texto firmado dentro del hash** — `create/native/au_src/hash.c`: `matrict_set_extra()` mete bytes extra en el desafío; cambiar el texto entre `spend` y `verify` rompe la prueba.
3. **Escalado automático del tope Au** — `auditor/gnorm.c` y `create/native/au_src/gnorm.c`: `TG` base (100 llaves) × `N_SPENT/100`, con acarreo de 128 bits. Sin esto, 1000 llaves no pueden firmar.
4. **Escalado automático del tope Plus** — `plus/param.h`: `BG_0 = 37162531 × N_SPENT / 50`.
5. **RNG portable** — `test/randombytes_portable.c`, para que los `.exe` no dependan de `/dev/urandom` en Windows.

## 3. El port a Rust

Hay **dos crates**, ambos por FFI (no reimplementan la criptografía):

| Crate | Contenido | Build |
|---|---|---|
| `create/` | Au: `src/main.rs`, `src/lib.rs`, `native/shim.c`, `native/au_src/`, `tests/v4.rs` | `build.rs` compila los C con `cc`; SHAKE-256 local en `native/shake256.c` (sin `-lXKCP`) |
| `create_plus/` | Plus: `src/main.rs`, `src/lib.rs`, `native/shim.c`, `tests/plus.rs` | `build.rs` compila los C de `plus/`; hereda `BG_0` escalado |

Ambos compilan a `.exe` de Windows con `x86_64-pc-windows-gnu` y release optimizado (`lto`, `strip`, `-O3`). Los tests son 6 (Au) + 3 (Plus) y se ejecutan en un job manual (`test-rust.yml`).

Los workflows **de build no ejecutan tests**: solo compilan y suben artifacts.

## 4. Pruebas ya corridas

| Prueba | Resultado |
|---|---|
| `auditor.exe` (100 llaves) | 5/5 honestas; los 4 intentos de firma falsa rechazados; falsificador 0/100×3; observador ve los montos |
| `plus.exe` (50 llaves) | 1000/1000; serial cero y prueba alterada rechazados |
| `au_1k.exe` (1000 llaves) | firma y verifica OK |
| `plus_1k.exe` (1000 llaves, `BG_0=743250620`) | firma y verifica OK |
| falsificación fuera de rango (×10 a ×1000) | 400/400 rechazadas |
| falsificación agresiva (bordes de norma, serial ajeno, anillo sustituido, prueba desde cero) | 350/350 rechazadas |
| tests Rust Au / Plus | 6/6 y 3/3 |

**Interpretación:** el atacante sí puede correr `spend()` y producir una prueba válida; lo que no puede es hacer que pase la **verificación** después de manipularla. La barrera no es solo la norma, sino que la prueba está ligada al hash del desafío y a las ecuaciones del anillo.

## 5. Estimación de seguridad (lo que NO está probado)

Los tests anteriores son de **comportamiento**, no pruebas de seguridad formal. Para estimar bits de seguridad se usan los scripts de `main/repos_originales/matrict_plus/asymmetric_sis_estimate/`:

- `model_BKZ.py` — modelo BKZ, `delta_BKZ(b)`, costos SVP clásico/cuántico.
- `MSIS_security.py` — estimación de ataques **asymmetric MSIS** (variantes 0/1/2).
- `proba_util.py` — pesos gaussianos.
- `ASIS_est.sage` — parámetros publicados de MatRiCT+ (`d=256`, `q=3489661057`, `B1..B5`, `N=11`).

En `security/` hay copias adaptadas a Python puro (sin Sage) y:

- `audit_config.py` — verifica parches, entropías y fórmulas de escalado leyendo los `.h` reales.
- `estimate.py` — corre el estimador MSIS con límite de tiempo.

El workflow `security.yml` publica los reportes como artifact y en el resumen del job, con `timeout-minutes: 60`.

### Límites de la estimación

- Los parámetros **publicados** (anillo 100 Au / 50 Plus) tienen el análisis del paper.
- Los anillos **500/1000 con topes escalados** son experimentales: `reportes/formula_anillos.md` indica que δ ≈ 1.0045 **no fue reestimado** y que hace falta auditoría externa para usar dinero real.
- La estimación `au` de `estimate.py` es **aproximada**: deriva las cotas de los topes del código, no del paper.
- Ningún test demuestra ausencia de ataques desconocidos; solo cubre los vectores probados.

## 6. Cómo correrlo

```bash
python security/audit_config.py        # auditoria estatica (rapido)
python security/estimate.py plus       # estimacion MSIS publicada
python security/estimate.py au         # estimacion aproximada del Au
```

En GitHub: Actions → **Seguridad** → Run workflow. Cada job tiene límite de 60 minutos.
