// quicksort.hpp
// QuirckSort: Mediana de tres + partición de Hoare

#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

struct EstadisticasQuick {
    std::uint64_t comparaciones = 0;   
    std::uint64_t intercambios = 0;    
    std::uint64_t particiones = 0;     
    int profundidadMaxima = 0;         
};

namespace quick {


enum class Pivote { MedianaDeTres, Primero };

using Indice = std::ptrdiff_t;

inline bool menor(const std::string& a, const std::string& b, EstadisticasQuick& e) {
    ++e.comparaciones;
    return a < b;
}

inline void intercambiar(std::vector<std::string>& v, Indice i, Indice j,
                         EstadisticasQuick& e) {
    std::swap(v[i], v[j]);  
    ++e.intercambios;
}


inline void ordenarTres(std::vector<std::string>& v, Indice lo, Indice medio,
                        Indice hi, EstadisticasQuick& e) {
    if (menor(v[medio], v[lo], e)) intercambiar(v, medio, lo, e);
    if (menor(v[hi], v[lo], e)) intercambiar(v, hi, lo, e);
    if (menor(v[hi], v[medio], e)) intercambiar(v, hi, medio, e);
}


inline Indice particionar(std::vector<std::string>& v, Indice lo, Indice hi,
                          EstadisticasQuick& e, Pivote modo) {
    const Indice medio = lo + (hi - lo) / 2;
    if (modo == Pivote::MedianaDeTres) ordenarTres(v, lo, medio, hi, e);
    // Se COPIA el pivote porque su casilla puede cambiar durante los swaps.
    const std::string pivote = (modo == Pivote::MedianaDeTres) ? v[medio] : v[lo];

    Indice i = lo - 1;
    Indice j = hi + 1;
    while (true) {
        do { ++i; } while (menor(v[i], pivote, e));   
        do { --j; } while (menor(pivote, v[j], e));   
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

}  
