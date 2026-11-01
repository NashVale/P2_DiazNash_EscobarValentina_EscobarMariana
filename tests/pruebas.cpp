// pruebas.cpp

#include <cstdint>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "../src/avl.hpp"
#include "../src/heapsort.hpp"
#include "../src/memoria.hpp"
#include "../src/quicksort.hpp"
#include "../src/utilidades.hpp"


using std::vector;

static int fallos = 0, pruebas = 0;

static void comprobar(bool ok, const string& que) {
    ++pruebas;
    if (!ok) {
        ++fallos;
        std::cout << "  [FALLA] " << que << "\n";
    }
}

// Referencia: ordenamiento por insercion O(n^2)
static vector<string> referencia(vector<string> v) {
    for (std::size_t i = 1; i < v.size(); ++i) {
        string x = v[i];
        std::size_t j = i;
        while (j > 0 && x < v[j - 1]) { v[j] = v[j - 1]; --j; }
        v[j] = x;
    }
    return v;
}

static void probarTodos(const vector<string>& entrada, const string& nombre) {
    const vector<string> esperado = referencia(entrada);

    { vector<string> v = entrada; EstadisticasQuick e; quick::ordenar(v, e);
      comprobar(v == esperado, "QuickSort (mediana de 3): " + nombre); }
    { vector<string> v = entrada; EstadisticasQuick e;
      quick::ordenar(v, e, quick::Pivote::Primero);
      comprobar(v == esperado, "QuickSort (pivote primero): " + nombre);
      
      int cota = 1; for (std::size_t n = v.size(); n > 1; n /= 2) ++cota;
      comprobar(e.profundidadMaxima <= cota + 1, "Profundidad QuickSort acotada: " + nombre); }
    { EstadisticasHeap e; vector<string> s = heap::ordenar(entrada, e);
      comprobar(s == esperado, "HeapSort: " + nombre); }
    { vector<string> v = entrada; EstadisticasHeap e; heap::ordenarInSitu(v, e);
      comprobar(v == esperado, "HeapSort in situ: " + nombre); }
    { EstadisticasAVL e; ArbolAVL a(e);
      for (const auto& s : entrada) a.insertar(s);
      vector<string> s; a.inorden(s);
      comprobar(s == esperado, "AVL inorden: " + nombre);
      comprobar(a.esValido(), "AVL propiedades: " + nombre);
      comprobar(a.elementos() == entrada.size(), "AVL cantidad de elementos: " + nombre); }
}

int main() {
    const std::size_t memInicial = memoria::actual();
    std::cout << "Casos limite...\n";
    probarTodos({}, "vacio");
    probarTodos({"hola"}, "un elemento");
    probarTodos({"b", "a"}, "dos elementos");
    probarTodos({"a", "b"}, "dos ordenados");
    probarTodos(vector<string>(50, "igual"), "todos iguales");
    probarTodos({"a", "b", "c", "d", "e", "f", "g", "h"}, "ya ordenado");
    probarTodos({"h", "g", "f", "e", "d", "c", "b", "a"}, "orden inverso");
    probarTodos({"car", "ca", "c", "cart", "carts", "cartel", "", "a"}, "prefijos y cadena vacia");
    probarTodos({"Zeta", "alfa", "Beta", "zeta", "beta"}, "mayusculas (orden ASCII)");
    probarTodos({"palabraconmuchasletras_mas_de_quince", "corta",
                 "otrapalabramuylarga_tambien_de_mas", "a"}, "strings largos (fuera de SSO)");

    std::cout << "Pruebas aleatorias (3000 arreglos)...\n";
    std::mt19937 gen(12345);
    const string letras = "abcde";
    for (int t = 0; t < 3000; ++t) {
        const int n = static_cast<int>(gen() % 60);
        vector<string> v;
        for (int i = 0; i < n; ++i) {
            const int len = 1 + static_cast<int>(gen() % (t % 3 == 0 ? 2 : 20));
            string s;
            for (int k = 0; k < len; ++k) s += letras[gen() % letras.size()];
            v.push_back(s);
        }
        probarTodos(v, "aleatorio #" + std::to_string(t));
        if (fallos > 10) break;
    }

    std::cout << "Estructura del heap (insertar / extraer)...\n";
    {
        EstadisticasHeap e;
        MonticuloBinarioMin h(e);
        vector<string> palabras = {"m", "c", "x", "a", "p", "c", "z", "b"};
        for (const auto& p : palabras) { h.insertar(p); comprobar(h.esValido(), "heap valido tras insertar"); }
        comprobar(h.minimo() == "a", "minimo correcto");
        string anterior;
        while (!h.vacio()) {
            string x = h.extraerMin();
            comprobar(anterior <= x, "extraerMin en orden");
            comprobar(h.esValido(), "heap valido tras extraer");
            anterior = x;
        }
        bool lanzo = false;
        try { h.extraerMin(); } catch (const std::out_of_range&) { lanzo = true; }
        comprobar(lanzo, "extraer de heap vacio lanza excepcion");
    }

    // Todo lo reservado por las pruebas debe haberse liberado (sin fugas).
    // (Se evalúa antes de llamar a comprobar(): el texto del mensaje es un
    //  std::string largo que tambien reserva momoria.)
    const bool sinFugas = memoria::actual() == memInicial;
    comprobar(sinFugas, "sin fugas de memoria (new/delete balanceados)");

    std::cout << "\n" << (pruebas - fallos) << " / " << pruebas << " pruebas correctas\n";
    std::cout << (fallos == 0 ? "TODAS LAS PRUEBAS PASARON\n" : "HAY PRUEBAS FALLIDAS\n");
    return fallos == 0 ? 0 : 1;
}
