# Mi práctica #2 de Estructuras de Datos - Ordenamiento un dataset grande
Por Nash Díaz

El programa ordena alfabéticamente las 100.000 palabras de `data/dataset.txt` con tres estrategias implementadas desde cero (sin `std::sort` ni contenedores ordenados de la librería estándar) y compara su tiempo y memoria.

| Estrategia | Estructura | Archivo |
|---|---|---|
| QuickSort | `std::vector<std::string>` | `src/quicksort.hpp` |
| HeapSort | Montículo binario | `src/heapsort.hpp` |
| Árbol AVL + inorden | Árbol binario de búsqueda balanceado | `src/avl.hpp` |

## Estructura

```
data/      dataset.txt (100.000 palabras)
src/       código fuente
tests/     pruebas automáticas
tools/     generador del dataset a partir de words_alpha.txt
results/   resultados (CSV, archivos ordenados, salida de consola)
docs/      informe
```

## Compilar y ejecutar

Desde la carpeta del proyecto:

```bash
g++ -std=c++17 -O2 -o ordenador src/main.cpp src/memoria.cpp
./ordenador          # menú interactivo
./ordenador --todo   # corre todo y guarda los resultados en results/
```

En Windows el ejecutable es `ordenador.exe`. Opciones adicionales: `--reps N` (repeticiones, por defecto 5) y `--dataset ruta`.

## Pruebas

```bash
g++ -std=c++17 -O2 -o pruebas tests/pruebas.cpp src/memoria.cpp
./pruebas
```

## Generar el dataset

```bash
g++ -std=c++17 -O2 -o generar_dataset tools/generar_dataset.cpp
./generar_dataset words_alpha.txt data/dataset.txt 100000 2026
```
## NOTA: En Windows con MinGW hay que agregar -static:

g++ -std=c++17 -O2 -static -o ordenador src/main.cpp src/memoria.cpp
g++ -std=c++17 -O2 -static -o pruebas tests/pruebas.cpp src/memoria.cpp
## Resultados

n = 100.000, media de 5 repeticiones:

| Estrategia | Tiempo | Memoria extra |
|---|---|---|
| QuickSort | 34,8 ms | ≈ 0 MB |
| Árbol AVL | 56,7 ms | 8,44 MB |
| HeapSort | 68,9 ms | 3,05 MB |

Los tiempos dependen de la máquina. Es un README bastante general ya que el análisis completo está en en el informe.
