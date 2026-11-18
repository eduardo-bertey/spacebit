# Reporte de seguridad — Parte 3 (2026-11-17, noche)

Continúa las partes 1 y 2 (test de confianza Au, mensajes 1131–1134).

## Lotes grandes de falsos positivos (Au)
- Anillo 500: **100/100 rondas perfectas, 0 falsos positivos, 0 honestas rechazadas.**
- Anillo 1000: **100/100 perfectas, 0 falsos positivos, 0 honestas rechazadas.**

## Variante de billetes múltiples (Au)
- **M=2** (gastar 2 billetes por transacción, cada uno en su propio anillo de 100): **20/20 perfectas, 0 falsos positivos.** Primera vez que se prueba esta base; todo lo anterior era M=1.
- **M=3**: el programa se cae (segfault) sin completar una ronda. Los autores publican solo M=1 y M=2; M=3 exige recalcular parámetros.

## Pruebas con el parámetro h (la imagen/serial del Au)
La imagen (serial) del Au sale de serialgen: imagen = secreta × h, con h un polinomio público fijo (256 coeficientes), igual en todos los nodos, creado en el setup. No viaja dentro de la prueba.

- **h distinto entre firma y verificación: 5/5 rechazadas.** h está atado a la verificación.
- **h = 0 (consistente en todos): verificaba 5/5, pero la imagen salía TODA CERO en todos los billetes** (medido: 0 coeficientes distintos de cero). Todas las huellas idénticas → el doble gasto se vuelve invisible. La verificación comprueba consistencia, no calidad: con h=0 los términos se anulan de los dos lados y las ecuaciones cierran vacías.
- **h = identidad: verifica**, pero la imagen queda rala (~15 coeficientes) y determinada por la secreta vía una transformada pública. Parámetro degenerado = peligroso.

## PARCHE imagen-cero (Au) — aplicado y verificado
A pedido del usuario: si la imagen viene toda cero, la verificación la rechaza.
El parche se agregó al inicio de `verify()` (revisa los coeficientes del serial; si todos son cero, devuelve 0). Aplicado en las copias de trabajo de 100/500/1000, en la variante M=2 y en las fuentes de la librería Rust (`au_src/verify.c`).

Resultados con el parche puesto:
- h = 0: **v=0 en 5/5** (antes pasaba).
- Fábrica 100: lote honesto 20/20, trampas 6×3 rechazadas/infirmables, control honesto 3/3.
- Anillo 500: lote 20/20 perfectas, 0 falsos positivos; trampas igual.
- Anillo 1000: lote 20/20 perfectas, 0 falsos positivos; trampas igual.
- h distinto al verificar: sigue rechazando 5/5. h identidad: sigue verificando (su imagen no es cero).

## El Plus y el mismo hueco
El Plus no tiene h ni serialgen: su serial son bytes al azar que genera `keygen` al crear la cuenta y quedan cocidos en la llave pública (pk se construye corrida por G(sn)).

- **Serial revelado cambiado a cero después de firmar: rechazado 5/5.** El serial está atado; no se puede intercambiar por otro.
- **Todas las cuentas creadas con serial cero (keygen forzado, consistente): verifica igual** (1000/1000 rondas del test stock). Es el mismo hueco en forma análoga: con todos los seriales en cero, toda huella es la misma y el doble gasto queda invisible. Diferencia con el Au: acá el serial lo pone el azar de keygen, no un parámetro público — solo se llega con un keygen roto o malicioso. El mismo parche (rechazar serial todo cero en verify) lo cerraría.

## Cómo se comprueba el monto (del código)
- Al crear el billete (`mint`), el monto se sella **bit a bit** dentro del compromiso, junto con el azar del dueño.
- Al gastar (`spend`), el balance se comprueba **bit por bit con acarreo**: en cada posición, bits que entran menos bits que salen; si no cierra, el acarreo se rompe y la prueba no se puede armar. Cada bit sellado prueba que es 0 o 1 (eso bloquea el monto "negativo" que daría la vuelta a 2⁶⁴).
- El verificador nunca ve montos: en `verify.c` no aparece ningún monto, solo cajas selladas y pruebas de bits.
- Dinero infinito no sale de crear billetes nuevos: el billete que se gasta ya tiene que existir en la lista pública de cuentas, ser tuyo (llave) y su monto sellado fija el total. Aceptar una transacción exige además el estado: imagen no repetida (anti doble gasto) y cuentas del anillo existentes.

## Alcance
Estas pruebas miden comportamiento (lo honesto pasa, lo tramposo no, el observador no miente). La seguridad de los parámetros escalados (500/1000) sigue sin reestimación formal (δ ≈ 1.0045 del paper) y un uso con dinero real requiere auditoría externa.
