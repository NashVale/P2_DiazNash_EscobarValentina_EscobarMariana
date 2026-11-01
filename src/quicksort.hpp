// quicksort.hpp
// ---------------------------------------------------------------------------
// A) QuickSort implementado desde cero sobre un arreglo dinamico
//    (std::vector<std::string>). No se usa std::sort ni ninguna funcion de
//    ordenamiento de la libreria estandar.
//
// Idea (divide y venceras):
//   1. Elegir un pivote.
//   2. Particionar: dejar a la izquierda los elementos <= pivote y a la
//      derecha los >= pivote.
//   3. Ordenar recursivamente cada lado. No hace falta "combinar": el arreglo
//      queda ordenado en su lugar (in situ).
//
// Decisiones de diseno (para la sustentacion):
//
// 1. Pivote por MEDIANA DE TRES: se ordenan v[lo], v[medio], v[hi] y el
//    pivote es el del medio. Con un pivote fijo (primer o ultimo elemento),
//    una entrada ya ordenada produce particiones de tamano 0 y n-1 en cada
//    paso  ->  ~n^2/2 comparaciones, O(n^2). Con la mediana de tres, en una
//    entrada ordenada el pivote es justo la mediana del rango.
//
// 2. Particion de HOARE: dos indices avanzan desde los extremos hacia el
//    centro y solo intercambian cuando ambos encuentran un elemento "del lado
//    equivocado". Hace ~3 veces menos intercambios que la particion de
//    Lomuto, y como ambos indices se detienen en elementos IGUALES al pivote,
//    un arreglo con muchas palabras repetidas se sigue dividiendo por la
//    mitad (Lomuto degeneraria a O(n^2) en ese caso).
//
// 3. Recursion sobre la parte MAS PEQUENA y ciclo (while) sobre la grande.
//    La parte pequena tiene como mucho la mitad de los elementos, asi que la
//    profundidad de la pila queda acotada por log2(n) incluso en el peor
//    caso: espacio auxiliar O(log n) garantizado, sin riesgo de desbordar
//    la pila.
// ---------------------------------------------------------------------------
#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

struct EstadisticasQuick {
    std::uint64_t comparaciones = 0;   // comparaciones entre palabras
    std::uint64_t intercambios = 0;    // swaps realizados
    std::uint64_t particiones = 0;     // rondas de particion
    int profundidadMaxima = 0;         // niveles maximos de recursion
};

namespace quick {

// Primero: pivote ingenuo (solo para demostrar el peor caso).
enum class Pivote { MedianaDeTres, Primero };

using Indice = std::ptrdiff_t;

inline bool menor(const std::string& a, const std::string& b, EstadisticasQuick& e) {
    ++e.comparaciones;
    return a < b;
}

inline void intercambiar(std::vector<std::string>& v, Indice i, Indice j,
                         EstadisticasQuick& e) {
    std::swap(v[i], v[j]);  // O(1): intercambia los punteros internos del string
    ++e.intercambios;
}

// Deja v[lo] <= v[medio] <= v[hi]. El valor del medio sera el pivote.
inline void ordenarTres(std::vector<std::string>& v, Indice lo, Indice medio,
                        Indice hi, EstadisticasQuick& e) {
    if (menor(v[medio], v[lo], e)) intercambiar(v, medio, lo, e);
    if (menor(v[hi], v[lo], e)) intercambiar(v, hi, lo, e);
    if (menor(v[hi], v[medio], e)) intercambiar(v, hi, medio, e);
}

// Particion de Hoare. Devuelve j tal que:
//   todo v[lo..j] <= pivote   y   todo v[j+1..hi] >= pivote,  con lo <= j < hi.
inline Indice particionar(std::vector<std::string>& v, Indice lo, Indice hi,
                          EstadisticasQuick& e, Pivote modo) {
    const Indice medio = lo + (hi - lo) / 2;
    if (modo == Pivote::MedianaDeTres) ordenarTres(v, lo, medio, hi, e);
    // Se COPIA el pivote porque su casilla puede cambiar durante los swaps.
    const std::string pivote = (modo == Pivote::MedianaDeTres) ? v[medio] : v[lo];

    Indice i = lo - 1;
    Indice j = hi + 1;
    while (true) {
        do { ++i; } while (menor(v[i], pivote, e));   // busca uno >= pivote
        do { --j; } while (menor(pivote, v[j], e));   // busca uno <= pivote
        if (i >= j) return j;
        intercambiar(v, i, j, e);
    }
}

inline void ordenarRango(std::vector<std::string>& v, Indice lo, Indice hi,
                         EstadisticasQuick& e, int profundidad, Pivote modo) {
    if (profundidad > e.profundidadMaxima) e.profundidadMaxima = profundidad;
    while (lo < hi) {
        ++e.particiones;
        const Indice j = particionar(v, lo, hi, e, modo);
        // Recursion en la parte pequena, iteracion en la grande.
        if (j - lo < hi - j) {
            ordenarRango(v, lo, j, e, profundidad + 1, modo);
            lo = j + 1;
        } else {
            ordenarRango(v, j + 1, hi, e, profundidad + 1, modo);
            hi = j;
        }
    }
}

// Punto de entrada: ordena v alfabeticamente, en el mismo arreglo (in situ).
inline void ordenar(std::vector<std::string>& v, EstadisticasQuick& e,
                    Pivote modo = Pivote::MedianaDeTres) {
    if (v.size() < 2) return;
    ordenarRango(v, 0, static_cast<Indice>(v.size()) - 1, e, 1, modo);
}

}  // namespace quick
