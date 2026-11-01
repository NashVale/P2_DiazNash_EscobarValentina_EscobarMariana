// avl.hpp
// ---------------------------------------------------------------------------
// C) Arbol AVL implementado desde cero.
//
// Un AVL es un arbol binario de busqueda (BST) donde, en TODO nodo, la
// diferencia de alturas entre el subarbol izquierdo y el derecho (factor de
// balance) es -1, 0 o +1. Eso garantiza altura O(log n) (como maximo
// ~1.44 * log2(n)), y por tanto insertar cuesta O(log n) siempre, sin
// importar el orden de llegada de los datos.
//
// Proceso pedido por el enunciado:
//   1. Insertar las 100.000 palabras en el arbol.      n * O(log n)
//   2. Recorrido INORDEN (izq, raiz, der).             O(n)
//   3. Guardar el resultado en una lista.             -> lista ordenada
//
// Cuando una insercion desbalancea un nodo (factor +-2) hay 4 casos:
//   Izq-Izq  -> rotacion simple a la derecha
//   Der-Der  -> rotacion simple a la izquierda
//   Izq-Der  -> doble: izquierda en el hijo, luego derecha en el nodo
//   Der-Izq  -> doble: derecha en el hijo, luego izquierda en el nodo
//
// Palabras repetidas: se guarda un contador en el nodo en lugar de crear
// otro nodo; en el inorden la palabra se escribe "cuenta" veces.
// ---------------------------------------------------------------------------
#pragma once
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

struct EstadisticasAVL {
    std::uint64_t comparaciones = 0;
    std::uint64_t rotacionesSimples = 0;
    std::uint64_t rotacionesDobles = 0;
};

class ArbolAVL {
    struct Nodo {
        std::string clave;
        int altura = 1;          // altura del subarbol (hoja = 1)
        int cuenta = 1;          // repeticiones de la palabra
        Nodo* izq = nullptr;
        Nodo* der = nullptr;
        explicit Nodo(std::string&& k) : clave(std::move(k)) {}
    };

public:
    explicit ArbolAVL(EstadisticasAVL& e) : est_(e) {}
    ~ArbolAVL() { destruir(raiz_); }

    ArbolAVL(const ArbolAVL&) = delete;             // no se copia el arbol
    ArbolAVL& operator=(const ArbolAVL&) = delete;

    void insertar(const std::string& palabra) {
        std::string copia = palabra;
        raiz_ = insertar(raiz_, std::move(copia));
    }
    void insertar(std::string&& palabra) {
        raiz_ = insertar(raiz_, std::move(palabra));
    }

    // Recorrido inorden: agrega las palabras en orden alfabetico a 'salida'.
    void inorden(std::vector<std::string>& salida) const { inorden(raiz_, salida); }

    std::size_t nodos() const { return nodos_; }
    std::size_t elementos() const { return elementos_; }
    int altura() const { return h(raiz_); }

    // Comprueba orden BST + balance AVL + alturas en todo el arbol.
    bool esValido() const {
        bool ok = true;
        validar(raiz_, nullptr, nullptr, ok);
        return ok;
    }

    static constexpr std::size_t bytesPorNodo() { return sizeof(Nodo); }

private:
    Nodo* raiz_ = nullptr;
    std::size_t nodos_ = 0;
    std::size_t elementos_ = 0;
    EstadisticasAVL& est_;

    static int h(const Nodo* n) { return n ? n->altura : 0; }
    static int factor(const Nodo* n) { return h(n->izq) - h(n->der); }
    static void actualizarAltura(Nodo* n) {
        n->altura = 1 + std::max(h(n->izq), h(n->der));
    }

    //        y                 x
    //       / \               / \          .
    //      x   C    --->     A   y
    //     / \                   / \        .
    //    A   B                 B   C
    static Nodo* rotarDerecha(Nodo* y) {
        Nodo* x = y->izq;
        y->izq = x->der;
        x->der = y;
        actualizarAltura(y);   // primero y (ahora esta abajo)
        actualizarAltura(x);
        return x;
    }

    static Nodo* rotarIzquierda(Nodo* x) {
        Nodo* y = x->der;
        x->der = y->izq;
        y->izq = x;
        actualizarAltura(x);
        actualizarAltura(y);
        return y;
    }

    Nodo* balancear(Nodo* n) {
        const int fb = factor(n);
        if (fb > 1) {                         // cargado a la izquierda
            if (factor(n->izq) < 0) {         // caso Izq-Der
                n->izq = rotarIzquierda(n->izq);
                ++est_.rotacionesDobles;
            } else {
                ++est_.rotacionesSimples;     // caso Izq-Izq
            }
            return rotarDerecha(n);
        }
        if (fb < -1) {                        // cargado a la derecha
            if (factor(n->der) > 0) {         // caso Der-Izq
                n->der = rotarDerecha(n->der);
                ++est_.rotacionesDobles;
            } else {
                ++est_.rotacionesSimples;     // caso Der-Der
            }
            return rotarIzquierda(n);
        }
        return n;                             // ya estaba balanceado
    }

    // Insercion recursiva: baja como en un BST y, al regresar, actualiza
    // alturas y rebalancea cada ancestro. Devuelve la nueva raiz del subarbol.
    Nodo* insertar(Nodo* n, std::string&& palabra) {
        if (n == nullptr) {
            ++nodos_;
            ++elementos_;
            return new Nodo(std::move(palabra));
        }
        const int c = palabra.compare(n->clave);
        ++est_.comparaciones;
        if (c < 0) {
            n->izq = insertar(n->izq, std::move(palabra));
        } else if (c > 0) {
            n->der = insertar(n->der, std::move(palabra));
        } else {
            ++n->cuenta;          // repetida: no cambia la forma del arbol
            ++elementos_;
            return n;
        }
        actualizarAltura(n);
        return balancear(n);
    }

    static void inorden(const Nodo* n, std::vector<std::string>& salida) {
        if (n == nullptr) return;
        inorden(n->izq, salida);
        for (int k = 0; k < n->cuenta; ++k) salida.push_back(n->clave);
        inorden(n->der, salida);
    }

    // Postorden: primero hijos, luego el nodo. Profundidad <= altura (~20).
    static void destruir(Nodo* n) {
        if (n == nullptr) return;
        destruir(n->izq);
        destruir(n->der);
        delete n;
    }

    static int validar(const Nodo* n, const std::string* min,
                       const std::string* max, bool& ok) {
        if (n == nullptr) return 0;
        if ((min && !(*min < n->clave)) || (max && !(n->clave < *max))) ok = false;
        const int hi = validar(n->izq, min, &n->clave, ok);
        const int hd = validar(n->der, &n->clave, max, ok);
        if (hi - hd > 1 || hd - hi > 1) ok = false;
        if (n->altura != 1 + std::max(hi, hd)) ok = false;
        return 1 + std::max(hi, hd);
    }
};
