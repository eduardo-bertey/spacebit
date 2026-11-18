# Proyecto: auditor (Au) + plus + app Rust

## Estructura

```
auditor/    fuentes C del Au con el parche imagen-cero y sus tests
plus/       fuentes C del Plus (BG_0 = 37162531*N/50) y tests del serial
create/     app Rust que embebe al Au por FFI (con sus propios native/)
reportes/   informes
test/       build.sh + RNG portable
.github/    workflows: solo compilan, no ejecutan tests
```

Dos variantes nomas: **auditor** y **plus**. Un `.exe` por variante.

## Parches

- Imagen cero: `verify()` rechaza serial todo cero (auditor y
  `create/native/au_src/verify.c`).
- Texto firmado: `matrict_set_extra()` en el `hash.c` de `create/native/`,
  cambiar el texto entre `spend` y `verify` rompe la prueba.
- RNG portable para Windows (`test/randombytes_portable.c`).
- `create/build.rs` compila los C con `cc` y SHAKE-256 local (sin `-lXKCP`).

## Workflows

- `build-binarios.yml`: `auditor.exe` y `plus.exe` (MSYS2/MinGW), subidos
  como artifacts separados.
- `build-rust.yml`: `cargo build --release` en `create/`, sube el binario.

## Tests

- `auditor/test_firma_falsa.c`: modos honesto, pago falso, fuera de rango,
  clave falsa, barrido de montos y monto falso.
- `plus/`: `test.c`, `tp1.c`, `keygen_zero.c`.
- `create/tests/v4.rs`: tests Rust de ronda, auditoría, montos, texto
  alterado, serial cero, firma falsa y firmante fuera del anillo.

Ver `test/README.md` para los comandos.
