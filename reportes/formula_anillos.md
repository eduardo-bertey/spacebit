# Fórmula de estimación por cantidad de llaves — anillo común (MatRiCT+) y auditable (MatRiCT-Au)

Medido el 2026-11-17 en esta VM (1,5 núcleos), 3 rondas por tamaño, tiempos en ciclos
convertidos a segundos a 3 GHz (la convención del paper). Los tamaños de prueba son
`sizeof` exactos del código compilado (la prueba en memoria, sin serializar).

## Tabla medida — Auditable (MatRiCT-Au, 1 entrada)

| Llaves | Prueba en memoria | Firmar (mediana) | Verificar | Auditor |
|---|---|---|---|---|
| 10 | 280.576 B | ~0,10 s | ~0,013 s | ✓ índice y montos |
| 50 | 301.056 B | ~0,08 s | ~0,018 s | ✓ |
| 100 | 326.656 B | ~0,21 s | ~0,025 s | ✓ |
| 500¹ | 531.456 B | ~2,5 s | ~0,083 s | ✓ anillo completo 0–499 |
| 1.000 | 787.456 B | **no termina** (muro TG) | — | — |

¹ Con el arreglo del gadget: la tabla `tg_0` del repo trae 99 entradas y el resto
queda en cero (el auditor era ciego desde la posición 100). Generando las entradas
con la fórmula del paper (Ap. E.1: NTT de t̄ por el polinomio de bits del índice)
el auditor abre cualquier posición; verificado en 300, 499, posiciones al azar
(102, 41, 345) y 6 casos de montos (incluye monto de 64 bits y cambio 0).

## Tabla medida — Común (MatRiCT+, 2 entradas)

| Llaves | Prueba en memoria | Firmar (mediana) | Verificar |
|---|---|---|---|
| 10 | 186.400 B | ~0,06 s | ~0,0014 s |
| 20 | 206.880 B | ~0,09 s | ~0,0022 s |
| 50 | 268.320 B | ~0,15 s | ~0,0048 s |
| 100 | 370.720 B | **no termina** en 240 s (rechazo) | — |
| 250 | 677.920 B | **no termina** en 240 s | — |
| 500 | 1.189.920 B | **no termina** en 240 s | — |
| 16.384 | 33,7 MB | **no termina** (setup+cuentas < 40 s; se traba firmando) | — |
| 65.536 | 134.383.648 B | no corre en esta VM (RAM) | — |

## Las fórmulas (N = cantidad de llaves)

**Auditable (Au)**
- Prueba en memoria = **275.456 + 512·N** bytes — exacta en los 5 puntos medidos.
- Prueba serializada (Tabla 3 del paper, logarítmica): 18 KB (N=2) · 31 KB (64) ·
  59 KB (4.096) · 148 KB (2²¹). Interpolada: ~34 KB (100) · ~45 KB (500) · ~99 KB (65.536).
- RAM del test ≈ **57.888·N** bytes (cuentas + claves) → 3,79 GB a 65.536 llaves.
- Firmar ≈ **5 ms por llave** (tramo 100→500); verificar ≈ 0,012 + 0,00014·N s;
  auditar ≈ 0,01 s, casi fijo.

**Común (Plus)**
- Prueba en memoria = **165.920 + 2.048·N** bytes — exacta en los 7 puntos medidos
  (más 28.704 B constantes de las salidas CN/CK/SN).
- RAM del test ≈ **126.976·N** bytes en arreglos → **8,32 GB a 65.536 llaves**
  (a 16.384 el proceso tocó 3,1 GB).
- Firmar ≈ **3 ms por llave** y verificar ≈ 0,09 ms por llave, solo hasta N=50.

## Los dos muros (medidos, no teóricos)

- **Au**: el chequeo de norma de la firma usa un tope fijo (TG) calibrado para
  N ≤ 100; a 1.000 rechaza casi siempre y no termina. Funciona hasta ~500
  (con el gadget del auditor extendido por fórmula).
- **Plus**: los topes de rechazo (BG_0/BG_1 y el control de f) están calibrados
  para N ≤ 50; desde N=100 la firma no completa ni una ronda en 240 s.
  El diseño publicado del Plus llega hasta 50; el del Au declara su set hasta 100.

## Proyección a 65.536 llaves (2¹⁶)

Válida solo **con un set de parámetros recalibrado** con las fórmulas del paper
(los tamaños sí son extrapolación exacta de las fórmulas lineales):

| | Auditable | Común |
|---|---|---|
| Prueba en memoria | 33,8 MB | 134,4 MB |
| Prueba serializada | ~99 KB | s/d (el repo no serializa) |
| RAM del test | ~3,8 GB | ~8,3 GB |
| Firmar | ~5,5 min (a 5 ms/llave) | no estimable sin set nuevo |
| Verificar | ~9 s | no estimable sin set nuevo |

Sin recalibración: el Au no pasa de ~500 y el Plus no pasa de ~50, a cualquier
tamaño mayor la firma entra en rechazo permanente.

---

# FÓRMULA EN EL CÓDIGO — expansión automática (2026-11-17, medido)

## El diagnóstico (Plus, N=500, instrumentado)
- El chequeo Bernoulli acepta ~29% (234/818): es su tasa de diseño, NO es la traba.
- La traba era UN solo chequeo: g_0. Valor real medido hasta 455.404.780 contra tope fijo BG_0 = 37.162.531 (~12×).
- Por qué: g_0 = f_0·(x − f_0) y f_0 acumula las máscaras de TODAS las llaves → el valor crece LINEAL con N.

## La fórmula (metida en param.h, build ~/workspace/ringtest/plusauto/)
    #define BG_0 (37162531LL * N_SPENT / 50LL)
- 37.162.531 = tope de los autores para 50 llaves; por llave = 743.250.
- Cambiás N_SPENT y el tope se calcula solo. Nada más se tocó.

## Verificación
- N=500: antes 0 rondas en 240 s → con la fórmula: 3 rondas en 4,08 s, correctness 1.
- N=16.384: 3 rondas en 4 min 21 s total, correctness 1 en las tres.
  Firmar: 6,9 s / 12,2 s / 94,2 s (varía según intentos). Verificar: ~1,5–2,0 s.
- La varianza al firmar sugiere que otro tope (probablemente BG_1, máximo sobre N+1
  polinomios) empieza a morder en rondas malas: candidato a la misma clase de
  fórmula (factor logarítmico) si se quiere firmar parejo a 16k.

---

# PISO DEL MARGEN — Au, anillo 100 (2026-11-17, medido)

Barrido del factor sobre TG (3 rondas honestas por paso, corte 60 s):
- 1,00 → completa; 0,90 → 2 s; 0,88 → 5 s; 0,86 → 8 s; 0,85 → 37 s
  (agoniza); 0,84 → 0 rondas en 60 s: el piso está entre 0,84 y 0,85.
- Margen por llave: fábrica 1,8474×10²³; mínimo práctico (0,86)
  1,5888×10²³; en 0,85 → 1,5703×10²³; en 0,84 → 1,5518×10²³ (muere).
- El fracaso al bajar el tope se ve como TIEMPO DE FIRMA que explota
  (más intentos descartados), no en la verificación.
- El margen de fábrica es fino: solo ~16% sobre el piso práctico.
