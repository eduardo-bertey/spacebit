# Build y tests

## Estructura (solo dos variantes)

```
auditor/    fuentes C del esquema Au + test_firma_falsa.c
plus/       fuentes C del esquema Plus + tests del serial
create/     app Rust (FFI sobre su propio native/au_src)
reportes/   documentos
test/build/build.sh   unico script de compilacion
.github/workflows/    solo compilan y suben artifacts
```

## Un solo script

```bash
bash test/build/build.sh <variante> <main.c> <salida.exe>
```

Los dos comandos del workflow:

```bash
bash test/build/build.sh auditor test.c auditor.exe
bash test/build/build.sh plus test.c plus.exe
```

Excluye los `main()` de prueba y `randombytes.c`; usa
`test/randombytes_portable.c` para que el `.exe` no se cuelgue en Windows
buscando `/dev/urandom`.

## Tests locales

```bash
# arnes de firma falsa / montos / rangos (auditor)
bash test/build/build.sh auditor test_firma_falsa.c firma_falsa.exe
firma_falsa.exe 100 7 1000000 900000 0     # honesto
firma_falsa.exe 100 7 1000000 900000 1     # pago falso
firma_falsa.exe 100 7 1000000 900000 2     # firmante fuera del anillo

# Plus con serial cero (sustituye keygen.c por keygen_zero.c)
bash test/build/build_plus.sh plus tp1.c plussn_serial_cero.exe zero

# Rust
cd create && cargo test
```

## Falsificador (rango alto)

`auditor/test_falsificador.c` arma una ronda honesta y lanza N intentos por
estrategia con valores altos al azar: serial aleatorio, `zc`/`zb` cerca del
tope de norma, `x` aleatorio, clave ajena y anillo sustituido. Cuenta cuántos
pasan `verify()`:

```bash
bash test/build/build.sh auditor test_falsificador.c falsificador.exe
falsificador.exe 200 424242
```

Para probar anillo 500/1000 usá las variantes que están en `main/`
(ignoradas por git, no se versionan):

```bash
bash test/build/build.sh main/trabajados/au_con_parche_y_tests/au500test test_falsificador.c falso500.exe
bash test/build/build.sh main/trabajados/au_con_parche_y_tests/au1000test test_falsificador.c falso1000.exe
```

Esperado: `ACEPTADAS=0` en las 6 estrategias. Subir el tope `TG × N/100`
agranda la región de norma, pero el atacante todavía tiene que cerrar las
ecuaciones del anillo y el desafío hash; un valor alto al azar por sí solo
no las cierra. El riesgo real de escalar a 500/1000 es de **soundness** de
los parámetros (δ ≈ 1.0045 sin reestimar, ver `reportes/`).
