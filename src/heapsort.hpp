// heapsort.hpp

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


class MonticuloBinarioMin {
public:
    explicit MonticuloBinarioMin(EstadisticasHeap& e) : est_(e) {}

   
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

    void insertar(std::string palabra) {
        a_.push_back(std::move(palabra));
        flotar(a_.size() - 1);
    }

    
    std::string extraerMin() {
        if (a_.empty()) throw std::out_of_range("monticulo vacio");
        std::string menor = std::move(a_[0]);
        if (a_.size() > 1) {
            a_[0] = std::move(a_.back());  
        }
        a_.pop_back();
        if (!a_.empty()) hundir(0);        
        return menor;
    }

    // Altura del arbol completo: floor(log2 n) 
    int altura() const {
        int h = 0;
        for (std::size_t n = a_.size(); n > 1; n /= 2) ++h;
        return h;
    }

    // Verifica la propiedad de heap en todo el arreglo 
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


inline std::vector<std::string> ordenar(std::vector<std::string> datos,
                                        EstadisticasHeap& e) {
    MonticuloBinarioMin h(std::move(datos), e);
    std::vector<std::string> salida;
    salida.reserve(h.tamano());          
    while (!h.vacio()) salida.push_back(h.extraerMin());
    return salida;
}

// Variante EXTRA: HeapSort in situ con max-heap (memoria auxiliar O(1)).
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
    for (std::size_t i = n / 2; i-- > 0;) hundirMax(a, i, n, e);  
    for (std::size_t fin = n - 1; fin > 0; --fin) {
        std::swap(a[0], a[fin]); 
        ++e.intercambios;
        hundirMax(a, 0, fin, e);  
    }
}

} 
