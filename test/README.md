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
