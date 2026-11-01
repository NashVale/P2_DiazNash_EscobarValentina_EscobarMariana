// heapsort.hpp
// ---------------------------------------------------------------------------
// B) HeapSort implementado desde cero con un MONTICULO BINARIO (Binary Heap).
//
// El monticulo es un arbol binario COMPLETO guardado en un arreglo:
//      padre(i) = (i - 1) / 2      hijoIzq(i) = 2i + 1      hijoDer(i) = 2i + 2
// No necesita punteros: la forma del arbol queda implicita en los indices.
//
// Propiedad de min-heap: cada padre es <= que sus hijos, luego la raiz
// (posicion 0) siempre tiene la palabra alfabeticamente menor.
//
// Version principal (la que pide el enunciado):
//   1. Construir el heap con TODAS las palabras (algoritmo de Floyd, O(n)).
//   2. Extraer el minimo n veces y agregarlo a una lista de salida.
//      Cada extraccion cuesta O(log n)  ->  total O(n log n).
//
// Version extra "in situ": usa un MAX-heap dentro del mismo arreglo y va
// dejando el maximo al final. Mismo tiempo O(n log n), pero memoria extra
// O(1) en lugar de O(n). Sirve para comparar en el analisis de memoria.
// ---------------------------------------------------------------------------
#pragma once
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

struct EstadisticasHeap {
    std::uint64_t comparaciones = 0;
    std::uint64_t intercambios = 0;
};

// ===========================================================================
// Estructura de datos: monticulo binario de minimos sobre std::vector.
// ===========================================================================
class MonticuloBinarioMin {
public:
    explicit MonticuloBinarioMin(EstadisticasHeap& e) : est_(e) {}

    // Construye el heap a partir de datos ya existentes (Floyd, O(n)).
    // Recibe el vector por valor: si el llamador usa std::move, no se copia.
    MonticuloBinarioMin(std::vector<std::string> datos, EstadisticasHeap& e)
        : a_(std::move(datos)), est_(e) {
        construir();
    }

    bool vacio() const { return a_.empty(); }
    std::size_t tamano() const { return a_.size(); }

    const std::string& minimo() const {
        if (a_.empty()) throw std::out_of_range("monticulo vacio");
        return a_[0];
    }

    // Insercion individual: al final del arreglo y luego "flotar". O(log n)
    void insertar(std::string palabra) {
        a_.push_back(std::move(palabra));
        flotar(a_.size() - 1);
    }

    // Saca la raiz (el minimo). O(log n)
    std::string extraerMin() {
        if (a_.empty()) throw std::out_of_range("monticulo vacio");
        std::string menor = std::move(a_[0]);
        if (a_.size() > 1) {
            a_[0] = std::move(a_.back());  // el ultimo sube a la raiz...
        }
        a_.pop_back();
        if (!a_.empty()) hundir(0);        // ...y se hunde hasta su lugar
        return menor;
    }

    // Altura del arbol completo: floor(log2 n) (util para la sustentacion).
    int altura() const {
        int h = 0;
        for (std::size_t n = a_.size(); n > 1; n /= 2) ++h;
        return h;
    }

    // Verifica la propiedad de heap en todo el arreglo (para pruebas).
    bool esValido() const {
        for (std::size_t i = 1; i < a_.size(); ++i)
            if (a_[i] < a_[padre(i)]) return false;
        return true;
    }

private:
    std::vector<std::string> a_;
    EstadisticasHeap& est_;

    static std::size_t padre(std::size_t i) { return (i - 1) / 2; }
    static std::size_t izq(std::size_t i) { return 2 * i + 1; }
    static std::size_t der(std::size_t i) { return 2 * i + 2; }

    // Floyd: hunde cada nodo interno, desde el ultimo padre hasta la raiz.
    // La mitad de los nodos son hojas (no se tocan) y la mayoria de nodos
    // internos estan cerca del fondo, por eso el total es O(n), no O(n log n).
    void construir() {
        if (a_.size() < 2) return;
        for (std::size_t i = a_.size() / 2; i-- > 0;) hundir(i);
    }

    void hundir(std::size_t i) {
        const std::size_t n = a_.size();
        while (true) {
            std::size_t menor = i;
            const std::size_t l = izq(i), r = der(i);
            if (l < n) { ++est_.comparaciones; if (a_[l] < a_[menor]) menor = l; }
            if (r < n) { ++est_.comparaciones; if (a_[r] < a_[menor]) menor = r; }
            if (menor == i) return;
            std::swap(a_[i], a_[menor]);
            ++est_.intercambios;
            i = menor;
        }
    }

    void flotar(std::size_t i) {
        while (i > 0) {
            ++est_.comparaciones;
            if (!(a_[i] < a_[padre(i)])) return;
            std::swap(a_[i], a_[padre(i)]);
            ++est_.intercambios;
            i = padre(i);
        }
    }
};

namespace heap {

// HeapSort principal: construye el heap y extrae n veces hacia una lista.
inline std::vector<std::string> ordenar(std::vector<std::string> datos,
                                        EstadisticasHeap& e) {
    MonticuloBinarioMin h(std::move(datos), e);
    std::vector<std::string> salida;
    salida.reserve(h.tamano());          // una sola reserva de memoria
    while (!h.vacio()) salida.push_back(h.extraerMin());
    return salida;
}

// ---------------------------------------------------------------------------
// Variante EXTRA: HeapSort in situ con max-heap (memoria auxiliar O(1)).
// ---------------------------------------------------------------------------
inline void hundirMax(std::vector<std::string>& a, std::size_t i,
                      std::size_t n, EstadisticasHeap& e) {
    while (true) {
        std::size_t mayor = i;
        const std::size_t l = 2 * i + 1, r = 2 * i + 2;
        if (l < n) { ++e.comparaciones; if (a[mayor] < a[l]) mayor = l; }
        if (r < n) { ++e.comparaciones; if (a[mayor] < a[r]) mayor = r; }
        if (mayor == i) return;
        std::swap(a[i], a[mayor]);
        ++e.intercambios;
        i = mayor;
    }
}

inline void ordenarInSitu(std::vector<std::string>& a, EstadisticasHeap& e) {
    const std::size_t n = a.size();
    if (n < 2) return;
    for (std::size_t i = n / 2; i-- > 0;) hundirMax(a, i, n, e);  // Floyd
    for (std::size_t fin = n - 1; fin > 0; --fin) {
        std::swap(a[0], a[fin]);  // el maximo va a su posicion final
        ++e.intercambios;
        hundirMax(a, 0, fin, e);  // reparar el heap en a[0 .. fin-1]
    }
}

}  // namespace heap
