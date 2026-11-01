# Ordenamiento de un dataset grande en C++

**ST0245 – Estructuras de Datos y Algoritmos · Universidad EAFIT**
Práctica: *Sorting Large Dataset (C++)*

Integrante: _(tu nombre completo)_

El programa ordena alfabéticamente 100.000 palabras con tres estrategias, cada una basada en una estructura de datos distinta, y compara su desempeño real:

| Estrategia | Estructura de datos | Archivo |
|---|---|---|
| A) QuickSort | Arreglo dinámico `std::vector<std::string>` | `src/quicksort.hpp` |
| B) HeapSort | Montículo binario (min-heap) | `src/heapsort.hpp` |
| C) Árbol AVL + inorden | Árbol binario de búsqueda balanceado | `src/avl.hpp` |
| Extra: HeapSort in situ | Max-heap dentro del mismo arreglo | `src/heapsort.hpp` |

Todo está implementado desde cero: no se usa `std::sort`, `std::stable_sort`, `std::priority_queue`, `std::make_heap`, `std::set` ni `std::map`.

## Estructura del proyecto

```
.
├── data/
│   └── dataset.txt            100.000 palabras en orden aleatorio
├── src/
│   ├── main.cpp               Menú, mediciones, análisis comparativo y experimentos
│   ├── quicksort.hpp          QuickSort (mediana de tres + partición de Hoare)
│   ├── heapsort.hpp           Montículo binario + HeapSort (y variante in situ)
│   ├── avl.hpp                Árbol AVL (inserción con rotaciones + inorden)
│   ├── utilidades.hpp         Lectura del archivo, verificación, estadísticas
│   ├── memoria.hpp            Medidor de memoria dinámica
│   └── memoria.cpp            (reemplaza new/delete para contar bytes)
├── tests/
│   └── pruebas.cpp            Pruebas automáticas (casos límite + aleatorias)
├── tools/
│   └── generar_dataset.cpp    Crea dataset.txt a partir de words_alpha.txt
└── results/                   Salidas: CSV, archivos ordenados, consola
```

## Requisitos

Un compilador de C++17: g++ 9 o superior (Linux, o MinGW-w64 en Windows), clang++ 10 o superior (macOS), o Visual Studio 2019 o superior.

## Compilar

Desde la carpeta raíz del proyecto:

**Linux / macOS / Windows con MinGW (g++)**
```bash
g++ -std=c++17 -O2 -Wall -Wextra -o ordenador src/main.cpp src/memoria.cpp
```

**macOS con clang**
```bash
clang++ -std=c++17 -O2 -o ordenador src/main.cpp src/memoria.cpp
```

**Windows con Visual Studio** (en la "Developer Command Prompt")
```bat
cl /std:c++17 /O2 /EHsc src\main.cpp src\memoria.cpp /Fe:ordenador.exe
```

> Compila siempre con `-O2` (o `/O2`). Sin optimización los tiempos salen varias veces más altos y no representan el algoritmo. El programa avisa si detecta que se compiló sin optimización.

## Ejecutar

Siempre desde la carpeta raíz, para que encuentre `data/dataset.txt`.

```bash
./ordenador            # menú interactivo (el que se usa en la sustentación)
./ordenador --todo     # corre todo: los 3 algoritmos + comparación + experimentos
```

En Windows: `ordenador.exe` o `.\ordenador.exe`.

Opciones:

| Opción | Qué hace | Por defecto |
|---|---|---|
| `--dataset ruta` | Archivo de entrada | `data/dataset.txt` |
| `--reps N` | Repeticiones por algoritmo (se reporta media, desviación, mínimo y máximo) | `5` |
| `--salida carpeta` | Dónde guardar resultados | `results` |
| `--todo` | Ejecuta todo sin menú | — |

Menú interactivo:

```
1. QuickSort
2. HeapSort (monticulo binario)
3. Arbol AVL + inorden
4. Los tres + analisis comparativo (+ HeapSort in situ)
5. Experimento de escalabilidad (n = 10k .. 100k)
6. Experimento de tipos de entrada y pivote
0. Salir
```

## Qué mide y reporta el programa

Para cada estrategia:

- **Tiempo de ejecución** con `std::chrono::steady_clock`, repetido N veces sobre una copia nueva de los datos (la copia no entra en la medición). Se reportan la media, la desviación estándar, el mínimo y el máximo. HeapSort y AVL también muestran el tiempo de cada fase (construir/extraer, insertar/recorrer).
- **Memoria estimada** con `sizeof`: estructura usada × número de elementos + texto de las palabras largas + estructuras auxiliares.
- **Memoria medida**: bytes realmente reservados durante el algoritmo, contados reemplazando los operadores globales `new`/`delete` (`src/memoria.cpp`). Así se puede contrastar la estimación con la realidad.
- **Contadores**: comparaciones, intercambios, rotaciones del AVL, profundidad de recursión, altura del árbol y del heap.
- **Verificación**: que la salida esté ordenada, que contenga exactamente las mismas palabras que la entrada (comprobado con una tabla hash, sin usar ningún ordenamiento) y que las salidas de todos los algoritmos sean idénticas. También se valida la estructura: propiedad de heap y propiedades AVL (orden BST, balance y alturas).

Archivos que genera en `results/`:

- `resultados.csv`: tiempo y memoria de cada estrategia
- `ordenado_quicksort.txt`, `ordenado_heapsort.txt`, `ordenado_avl.txt`, `ordenado_heapsort_insitu.txt`: el dataset ordenado
- `escalabilidad.csv`: tiempo para n = 10.000 … 100.000 (mínimo de las repeticiones, que filtra el ruido de otros procesos)
- `casos_entrada.csv`: entrada aleatoria, ordenada, inversa y con repetidos; efecto del pivote

Para guardar también lo que sale en consola:
```bash
./ordenador --todo > results/salida_consola.txt
```

## Pruebas automáticas

```bash
g++ -std=c++17 -O2 -o pruebas tests/pruebas.cpp src/memoria.cpp
./pruebas
```

Ejecuta más de 24.000 comprobaciones: casos límite (vacío, 1 elemento, todos iguales, ya ordenado, inverso, prefijos, mayúsculas, cadenas largas), 3.000 arreglos aleatorios con muchos repetidos comparados contra un ordenamiento por inserción, validación de las estructuras y detección de fugas de memoria. Debe terminar con `TODAS LAS PRUEBAS PASARON`.

Verificación externa opcional (Linux/macOS): la salida coincide con el `sort` del sistema en orden de bytes.

```bash
tr -d '\r' < data/dataset.txt | LC_ALL=C sort | cmp - results/ordenado_quicksort.txt && echo IDENTICO
```

## Generar el dataset (reproducible)

`words_alpha.txt` viene ordenado alfabéticamente, así que se toma una muestra aleatoria de 100.000 palabras con Fisher-Yates parcial y semilla fija:

```bash
g++ -std=c++17 -O2 -o generar_dataset tools/generar_dataset.cpp
./generar_dataset words_alpha.txt data/dataset.txt 100000 2026
```

Con la misma semilla se obtiene exactamente el mismo archivo en cualquier sistema operativo. Las pruebas de este repositorio usan el `dataset.txt` entregado por el profesor; el generador queda para cumplir y reproducir el paso de crear el dataset.

> El `dataset.txt` del profesor tiene finales de línea de Windows (`\r\n`). El lector del programa elimina el `\r`; sin eso, en Linux/macOS cada palabra quedaría con un carácter invisible al final.

## Resumen de resultados

Medido con g++ 13.3 `-O2`, n = 100.000, 5 repeticiones (los valores cambian según la máquina; correr `./ordenador --todo` para obtener los propios):

| Estrategia | Tiempo medio | Memoria extra medida | Complejidad (tiempo · espacio extra) |
|---|---|---|---|
| QuickSort | 34,8 ms | ≈ 0 MB | O(n log n) promedio, O(n²) peor · O(log n) |
| Árbol AVL | 56,7 ms | 8,44 MB | O(n log n) · O(n) |
| HeapSort | 68,9 ms | 3,05 MB | O(n log n) · O(n) |
| HeapSort in situ (extra) | 70,8 ms | 0 MB | O(n log n) · O(1) |

El análisis completo está en el informe.
