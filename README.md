# spacebit

Implementación de **MatRiCT** (IEEE S&P 2022): pagos con privacidad sobre **retículos (module-SIS / module-LWE)**, post-cuánticos.

## Dos variantes

| Variante | Fuente C | App Rust | exe |
|---|---|---|---|
| **Au** (auditable) | `auditor/` | `create/` | `auditor.exe`, `matrict_rust_app.exe` |
| **Plus** (común) | `plus/` | `create_plus/` | `plus.exe`, `matrict_plus_app.exe` |

Cada variante tiene **un exe en C y otro en Rust**. Los workflows solo compilan y suben artifacts; no ejecutan tests en la nube.

## Seguridad en resumen

- La clave secreta **no** es de 256 bits: son coeficientes ternarios `{-1,0,1}`. Au: 38×64 = 2432 coef. (~3855 bits). Plus: 8×256 = 2048 coef. (~3246 bits). Los 256 bits son el seed AES-256 y el serial/hash.
- `d`/`q`: Au 64 / 2147221513; Plus 256 / 3489661057.
- Topes que escalan solos: **Au** `TG × N_SPENT/100` (en `gnorm.c`), **Plus** `BG_0 = 37162531 × N_SPENT/50` (en `param.h`). Con eso 1000 llaves pueden firmar.
- Parches: `verify.c` rechaza serial/imagen todo cero; `hash.c` mete texto extra en el desafío.
- Detalle completo: `security/README.md`.

## Tests (ya corridos)

| Test | Resultado |
|---|---|
| `auditor.exe` (100 llaves) | 5/5 honestas; firma falsa rechazada; falsificador 0/100×3; observador ve los montos |
| `plus.exe` (50 llaves) | 1000/1000 |
| `au_1k.exe` / `plus_1k.exe` (1000 llaves) | firma y verifica OK |
| falsificación fuera de rango (×10–×1000) | 400/400 rechazadas |
| falsificación agresiva (bordes, serial ajeno, anillo, desde cero) | 350/350 rechazadas |
| tests Rust Au / Plus | 6/6 y 3/3 |

## Workflows

- `build-binarios.yml`: compila los `.exe` C (MSYS2/MinGW).
- `build-rust.yml`: compila los `.exe` Rust (target Windows GNU).
- `test-rust.yml`: manual, compila los binarios de test.
- `security.yml`: auditoría de parámetros + estimación MSIS; publica reportes, `timeout-minutes: 60`.

Ver `LEEME.md` para el índice detallado y `test/README.md` para los comandos.
