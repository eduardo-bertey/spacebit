# Código — Parte 3 (2026-11-17)

- `au_fabrica_parcheado/`: fuentes Au configuración de fábrica (100 llaves, M=1)
  con el PARCHE imagen-cero en verify.c. Incluye test.c (lote honesto),
  test_trampas.c (trampas), th_a.c (h distinto al verificar),
  th_b.c (h=0), th_c.c (h=identidad).
- `plus_serial/`: tp1.c (serial revelado en cero -> rechazado),
  keygen_zero.c (keygen forzado a serial cero -> verifica, el hueco análogo),
  verify.c del Plus SIN parche (referencia).
- `rust_au_src/verify.c`: el verify con el mismo parche para la librería Rust.

El parche: al inicio de verify(), si todos los coeficientes del serial
(imagen) son cero, devuelve 0.
