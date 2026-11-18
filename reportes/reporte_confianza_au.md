# Test de confianza — MatRiCT-Au en anillos de 500 y 1000 llaves

Fecha: 2026-11-17. Código: MatRiCT-Au oficial (GitLab raykzhao) con dos ajustes
de expansión metidos en el código:

1. La tabla del observador (gentdrow) venía escrita a mano hasta la posición 99;
   se extendió con la fórmula del paper (Apéndice E.1) para que el observador
   vea todas las posiciones del anillo.
2. El tope de norma TG quedó como fórmula: TG × N/100 (la norma suma los
   aportes de todas las llaves; con el tope fijo de 100, el anillo de 1000 no
   terminaba ni una ronda en 10 minutos; con la fórmula, los 6 casos corrieron
   en 38 segundos).

## Casos puntuales (idéntico resultado en 500 y en 1000)

| Caso | Posición real | Lo que abrió el observador | Verifica |
|---|---|---|---|
| Transacción en la posición 300, billete 100.000, paga 40.000 | 300 | 300, pago 40.000, cambio 60.000 | SÍ |
| Posición 256, billete 5.000, paga 1.234 | 256 | 256, pago 1.234, cambio 3.766 | SÍ |
| Usuario random (salió 453), billete y pago random | 453 | 453, montos exactos | SÍ |
| Última posición del anillo (499 / 999), paga todo, cambio 0 | 499 / 999 | exacta, cambio 0 | SÍ |
| Control honesto: billete 500, paga 300 | 88 | 88, pago 300, cambio 200 | SÍ |

## La mentira

Billete de **200** que intenta pagar **300** (el cambio sería −100; en el
sistema queda como 2⁶⁴−100):

- Anillo 500: verificación **0 — rechazada**.
- Anillo 1000: verificación **0 — rechazada**.

El observador puede abrir lo que la transacción declara, pero la transacción
no pasa la verificación pública: el pago no vale. El control honesto con los
mismos números de pago (billete 500, paga 300) sí pasa: lo que frena es el
sobrepago, no las cifras.

## Lote de confianza — falsos positivos

20 transacciones honestas con posición y montos al azar por anillo,
comparando lo real contra lo que abre el observador:

- Anillo 500: **20/20 perfectas, 0 falsos positivos, 0 honestas rechazadas**.
- Anillo 1000: **20/20 perfectas, 0 falsos positivos, 0 honestas rechazadas**.

Falso positivo sería que el observador señale a otra posición o abra montos
distintos de los reales: no ocurrió nunca en las 40 del lote. Sumando los
casos puntuales: 50 transacciones honestas, las 50 exactas; 2 intentos de
mentira, los 2 rechazados.

## Parte 2 — Trampas (6 trampas × 3 rondas × 2 anillos = 36 rondas)

| Trampa | Anillo 500 | Anillo 1000 |
|---|---|---|
| Sobrepago por 1: billete 1000, paga 1001 | v=0 en 3/3 | v=0 en 3/3 |
| Sobrepago grande: billete 200, paga 300 | v=0 en 3/3 | v=0 en 3/3 |
| Prueba adulterada después de firmar (un coeficiente cambiado) | v=0 en 3/3 | v=0 en 3/3 |
| Firmar el billete ajeno con la llave de otro miembro | v=0 en 3/3 | v=0 en 3/3 |
| Cambio inflado: billete 500, paga 300, declara cambio 300 | infirmable 3/3 | infirmable 3/3 |
| Control honesto (posición y montos random) | v=1 en 3/3, observador exacto | v=1 en 3/3, observador exacto |

"Infirable" quiere decir que el firmante nunca logra producir la prueba:
con corte de 180 s por ronda, las 6 rondas de esa trampa expiraron. La
diferencia con el sobrepago: ahí las cuentas cierran en el papel (el cambio
envuelto compensa) y es la verificación la que mata la prueba; con el cambio
inflado las cuentas no cierran ni en el papel (300 + 300 ≠ 500), así que la
prueba directamente no puede existir.

## Totales del test completo

- Casos puntuales: 12 rondas (6 por anillo).
- Lote de confianza: 40 rondas (20 por anillo).
- Trampas: 36 rondas (18 por anillo).
- **Total: 88 rondas. Transacciones honestas: 56, las 56 exactas y aceptadas.
  Ataques: 32, ninguno pasó** (26 rechazados en la verificación, 6
  imposibles de firmar).

## Alcance

Esto prueba comportamiento: el observador no miente y no acusa inocentes, y
el sobrepago no pasa. Los topes escalados mantienen el significado
estadístico del control original; un set de parámetros de producción para
500/1000 debería además reestimar la seguridad con las herramientas del
paper (factor de Hermite δ ≈ 1,0045), que es un paso separado.

Código de las pruebas: `test_casos.c` (casos por argumento) y el test
original con 20 rondas, en los directorios de build `au500test` y
`au1000test`.
