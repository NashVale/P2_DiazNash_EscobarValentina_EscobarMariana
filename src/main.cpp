// main.cpp
// ---------------------------------------------------------------------------
// Practica: ordenamiento de un dataset grande de palabras (C++)
// ST0245 - Estructuras de Datos y Algoritmos - Universidad EAFIT
//
// Estrategias:
//   A) QuickSort sobre std::vector<std::string>           (quicksort.hpp)
//   B) HeapSort con un monticulo binario                    (heapsort.hpp)
//   C) Arbol AVL + recorrido inorden                        (avl.hpp)
//   +  Extra: HeapSort in situ (para comparar memoria)
//
// Para cada estrategia se mide tiempo (std::chrono, varias repeticiones),
// memoria (estimada con sizeof y medida con el contador de new/delete) y se
// verifica que el resultado este ordenado y contenga las mismas palabras.
//
// Uso:
//   ordenador                      -> menu interactivo (para la sustentacion)
//   ordenador --todo               -> corre todo sin preguntar
//   ordenador --dataset ruta --reps 5 --salida results
//
// NOTA: los mensajes no llevan tildes a proposito, para que se vean bien en
// la consola de Windows (que no siempre usa UTF-8).
// ---------------------------------------------------------------------------
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>
#include <utility>
#include <vector>

#include "avl.hpp"
#include "heapsort.hpp"
#include "memoria.hpp"
#include "quicksort.hpp"
#include "utilidades.hpp"

namespace fs = std::filesystem;
using std::cout;
using std::size_t;
using std::string;
using std::vector;

// ===========================================================================
// Configuracion y resultados
// ===========================================================================
struct Config {
    string dataset = "data/dataset.txt";
    string carpetaSalida = "results";
    int repeticiones = 5;
};

struct Resultado {
    string nombre;
    string archivo;                   // nombre corto para archivos de salida
    string estructura;
    string bigOTiempo, bigOEspacio;
    vector<double> tiempos;           // ms por repeticion
    vector<std::pair<string, double>> fases;   // tiempo medio por fase (ms)
    size_t memTotalEstimada = 0;      // estructura completa (sizeof)
    size_t memExtraEstimada = 0;      // lo que el algoritmo agrega a la entrada
    size_t memExtraMedida = 0;        // pico real medido con new/delete
    vector<std::pair<string, string>> detalles;
    bool ordenado = false;
    bool permutacion = false;
    vector<string> salida;
};

static string linea(char c = '-', int n = 78) { return string(n, c); }

// ===========================================================================
// A) QuickSort
// ===========================================================================
static Resultado correrQuick(const vector<string>& datos, int reps) {
    Resultado r;
    r.nombre = "QuickSort";
    r.archivo = "quicksort";
    r.estructura = "Arreglo dinamico std::vector<std::string>";
    r.bigOTiempo = "O(n log n) prom. / O(n^2) peor";
    r.bigOEspacio = "O(log n) extra (in situ)";

    EstadisticasQuick ult;
    for (int k = 0; k < reps; ++k) {
        vector<string> v = datos;              // copia de trabajo (no se mide)
        EstadisticasQuick e;
        const size_t base = memoria::actual();
        memoria::reiniciarPico();

        const auto t0 = util::Reloj::now();
        quick::ordenar(v, e);
        const auto t1 = util::Reloj::now();

        // Se lee el pico ANTES de guardar el tiempo: push_back puede reservar
        // memoria y esa reserva es del programa de medicion, no del algoritmo.
        r.memExtraMedida = std::max(r.memExtraMedida, memoria::pico() - base);
        r.tiempos.push_back(util::msDesde(t0, t1));
        if (k == reps - 1) { ult = e; r.salida = std::move(v); }
    }

    const size_t n = datos.size(), S = sizeof(string);
    const size_t ext = util::bytesTextoExterno(datos);
    r.memTotalEstimada = n * S + ext;
    r.memExtraEstimada = 0;   // in situ; solo pila de recursion y la copia del pivote
    r.detalles = {
        {"Comparaciones", util::conMiles(ult.comparaciones)},
        {"Intercambios", util::conMiles(ult.intercambios)},
        {"Rondas de particion", util::conMiles(ult.particiones)},
        {"Profundidad max. de recursion",
         std::to_string(ult.profundidadMaxima) + "  (log2 n = " +
             util::fijo(std::log2(static_cast<double>(n)), 1) + ")"},
        {"Comparaciones / (n log2 n)",
         util::fijo(ult.comparaciones / (n * std::log2(static_cast<double>(n))), 3)},
    };
    return r;
}

// ===========================================================================
// B) HeapSort con monticulo binario (version del enunciado: heap -> lista)
// ===========================================================================
static Resultado correrHeap(const vector<string>& datos, int reps) {
    Resultado r;
    r.nombre = "HeapSort";
    r.archivo = "heapsort";
    r.estructura = "Monticulo binario (min-heap) en arreglo + lista de salida";
    r.bigOTiempo = "O(n log n) en todos los casos";
    r.bigOEspacio = "O(n) extra (lista de salida)";

    double tConstruir = 0, tExtraer = 0;
    EstadisticasHeap eConstruir, eTotal;
    int alturaHeap = 0;
    bool heapValido = false;

    for (int k = 0; k < reps; ++k) {
        vector<string> copia = datos;
        EstadisticasHeap e;
        vector<string> salida;
        const size_t base = memoria::actual();
        memoria::reiniciarPico();

        const auto t0 = util::Reloj::now();
        MonticuloBinarioMin h(std::move(copia), e);       // Floyd, O(n)
        const auto t1 = util::Reloj::now();
        if (k == reps - 1) {                               // fuera de la medicion
            eConstruir = e;
            alturaHeap = h.altura();
            heapValido = h.esValido();
        }
        const auto t1b = util::Reloj::now();
        salida.reserve(h.tamano());
        while (!h.vacio()) salida.push_back(h.extraerMin());  // n * O(log n)
        const auto t2 = util::Reloj::now();

        const double fase1 = util::msDesde(t0, t1), fase2 = util::msDesde(t1b, t2);
        tConstruir += fase1;
        tExtraer += fase2;
        r.memExtraMedida = std::max(r.memExtraMedida, memoria::pico() - base);
        r.tiempos.push_back(fase1 + fase2);
        if (k == reps - 1) { eTotal = e; r.salida = std::move(salida); }
    }
    r.fases = {{"Construir heap (Floyd)", tConstruir / reps},
               {"Extraer n minimos", tExtraer / reps}};

    const size_t n = datos.size(), S = sizeof(string);
    const size_t ext = util::bytesTextoExterno(datos);
    r.memTotalEstimada = 2 * n * S + ext;  // arreglo del heap + lista de salida
    r.memExtraEstimada = n * S;            // la lista de salida (el heap reusa la entrada)
    r.detalles = {
        {"Altura del heap", std::to_string(alturaHeap) + "  (floor(log2 n))"},
        {"Propiedad de heap verificada", heapValido ? "SI" : "NO"},
        {"Comparaciones al construir", util::conMiles(eConstruir.comparaciones) +
                                           "  (<= 2n = " + util::conMiles(2 * n) + ")"},
        {"Comparaciones totales", util::conMiles(eTotal.comparaciones)},
        {"Intercambios totales", util::conMiles(eTotal.intercambios)},
        {"Comparaciones / (n log2 n)",
         util::fijo(eTotal.comparaciones / (n * std::log2(static_cast<double>(n))), 3)},
    };
    return r;
}

// ===========================================================================
// Extra: HeapSort in situ (max-heap dentro del mismo arreglo)
// ===========================================================================
static Resultado correrHeapInSitu(const vector<string>& datos, int reps) {
    Resultado r;
    r.nombre = "HeapSort in situ (extra)";
    r.archivo = "heapsort_insitu";
    r.estructura = "Max-heap dentro del mismo std::vector";
    r.bigOTiempo = "O(n log n) en todos los casos";
    r.bigOEspacio = "O(1) extra";

    EstadisticasHeap ult;
    for (int k = 0; k < reps; ++k) {
        vector<string> v = datos;
        EstadisticasHeap e;
        const size_t base = memoria::actual();
        memoria::reiniciarPico();
        const auto t0 = util::Reloj::now();
        heap::ordenarInSitu(v, e);
        const auto t1 = util::Reloj::now();
        r.memExtraMedida = std::max(r.memExtraMedida, memoria::pico() - base);
        r.tiempos.push_back(util::msDesde(t0, t1));
        if (k == reps - 1) { ult = e; r.salida = std::move(v); }
    }
    const size_t n = datos.size(), S = sizeof(string);
    r.memTotalEstimada = n * S + util::bytesTextoExterno(datos);
    r.memExtraEstimada = 0;
    r.detalles = {
        {"Comparaciones", util::conMiles(ult.comparaciones)},
        {"Intercambios", util::conMiles(ult.intercambios)},
    };
    return r;
}

// ===========================================================================
// C) Arbol AVL + recorrido inorden
// ===========================================================================
static Resultado correrAVL(const vector<string>& datos, int reps) {
    Resultado r;
    r.nombre = "Arbol AVL";
    r.archivo = "avl";
    r.estructura = "Arbol AVL (nodos enlazados) + lista por recorrido inorden";
    r.bigOTiempo = "O(n log n) en todos los casos";
    r.bigOEspacio = "O(n) extra (nodos + lista)";

    double tInsertar = 0, tInorden = 0;
    EstadisticasAVL ult;
    int altura = 0;
    size_t nodos = 0;
    bool valido = false;

    for (int k = 0; k < reps; ++k) {
        vector<string> copia = datos;
        EstadisticasAVL e;
        vector<string> salida;
        const size_t base = memoria::actual();
        memoria::reiniciarPico();
        {
            const auto t0 = util::Reloj::now();
            ArbolAVL arbol(e);
            for (auto& s : copia) arbol.insertar(std::move(s));  // 1. insertar
            const auto t1 = util::Reloj::now();
            salida.reserve(arbol.elementos());
            arbol.inorden(salida);                               // 2-3. inorden -> lista
            const auto t2 = util::Reloj::now();

            const double f1 = util::msDesde(t0, t1), f2 = util::msDesde(t1, t2);
            tInsertar += f1;
            tInorden += f2;
            r.memExtraMedida = std::max(r.memExtraMedida, memoria::pico() - base);
            r.tiempos.push_back(f1 + f2);
            if (k == reps - 1) {
                ult = e;
                altura = arbol.altura();
                nodos = arbol.nodos();
                valido = arbol.esValido();   // fuera de la medicion de tiempo
            }
        }   // aqui se destruye el arbol (no entra en el tiempo)
        if (k == reps - 1) r.salida = std::move(salida);
    }
    r.fases = {{"Insertar n palabras", tInsertar / reps},
               {"Recorrido inorden -> lista", tInorden / reps}};

    const size_t n = datos.size(), S = sizeof(string), N = ArbolAVL::bytesPorNodo();
    const size_t ext = util::bytesTextoExterno(datos);
    const double lg = std::log2(static_cast<double>(n));
    r.memTotalEstimada = n * N + n * S + 2 * ext;  // nodos + lista (texto largo 2 veces)
    r.memExtraEstimada = n * N + n * S + ext;      // las claves se movieron a los nodos
    r.detalles = {
        {"Nodos creados", util::conMiles(nodos)},
        {"sizeof(Nodo)", std::to_string(N) + " bytes  (string " + std::to_string(S) +
                             " + 2 punteros + 2 int)"},
        {"Altura del arbol", std::to_string(altura) + "  (minima posible " +
                                 std::to_string(static_cast<int>(std::ceil(std::log2(n + 1.0)))) +
                                 ", cota AVL 1.44*log2 n = " + util::fijo(1.44 * lg, 1) + ")"},
        {"Propiedades AVL verificadas", valido ? "SI (orden BST + balance + alturas)" : "NO"},
        {"Comparaciones", util::conMiles(ult.comparaciones)},
        {"Rotaciones simples", util::conMiles(ult.rotacionesSimples)},
        {"Rotaciones dobles", util::conMiles(ult.rotacionesDobles)},
        {"Comparaciones / (n log2 n)", util::fijo(ult.comparaciones / (n * lg), 3)},
    };
    return r;
}

// ===========================================================================
// Presentacion de resultados
// ===========================================================================
static void verificar(Resultado& r, const vector<string>& datos) {
    r.ordenado = util::estaOrdenado(r.salida);
    r.permutacion = util::esPermutacion(datos, r.salida);
}

static void imprimirResultado(const Resultado& r) {
    const util::Resumen t = util::resumir(r.tiempos);
    cout << "\n" << linea('=') << "\n  " << r.nombre << "\n" << linea('=') << "\n";
    cout << "  Estructura : " << r.estructura << "\n";
    cout << "  Big O      : tiempo " << r.bigOTiempo << " | espacio " << r.bigOEspacio << "\n";
    cout << linea() << "\n";
    cout << "  Tiempo (" << r.tiempos.size() << " repeticiones): media " << util::fijo(t.media)
         << " ms | desv. " << util::fijo(t.desviacion) << " ms | min " << util::fijo(t.minimo)
         << " ms | max " << util::fijo(t.maximo) << " ms\n";
    for (const auto& f : r.fases)
        cout << "      - " << f.first << ": " << util::fijo(f.second) << " ms\n";
    cout << "  Memoria total estimada (sizeof) : " << util::mb(r.memTotalEstimada) << "\n";
    cout << "  Memoria EXTRA estimada          : " << util::mb(r.memExtraEstimada) << "\n";
    cout << "  Memoria EXTRA medida (new/del)  : " << util::mb(r.memExtraMedida) << "  ("
         << util::conMiles(r.memExtraMedida) << " bytes)\n";
    cout << linea() << "\n";
    for (const auto& d : r.detalles) {
        cout << "  " << d.first << string(d.first.size() < 32 ? 32 - d.first.size() : 1, ' ')
             << ": " << d.second << "\n";
    }
    cout << linea() << "\n";
    cout << "  Verificacion: ordenado = " << (r.ordenado ? "SI" : "NO")
         << " | mismas palabras que la entrada = " << (r.permutacion ? "SI" : "NO") << "\n";
    if (!r.salida.empty()) {
        cout << "  Primeras: ";
        for (size_t i = 0; i < 5 && i < r.salida.size(); ++i) cout << r.salida[i] << " ";
        cout << "\n  Ultimas : ";
        for (size_t i = r.salida.size() >= 5 ? r.salida.size() - 5 : 0; i < r.salida.size(); ++i)
            cout << r.salida[i] << " ";
        cout << "\n";
    }
}

static void imprimirComparacion(const vector<Resultado>& rs) {
    if (rs.empty()) return;
    double mejor = std::numeric_limits<double>::max();
    for (const auto& r : rs) mejor = std::min(mejor, util::resumir(r.tiempos).media);

    cout << "\n" << linea('=') << "\n  ANALISIS COMPARATIVO\n" << linea('=') << "\n";
    cout << "  Estrategia                  Media(ms)  Desv(ms)  vs mejor  Extra est.  Extra medida\n";
    cout << linea() << "\n";
    for (const auto& r : rs) {
        const util::Resumen t = util::resumir(r.tiempos);
        string nom = r.nombre;
        nom.resize(26, ' ');
        string m = util::fijo(t.media), d = util::fijo(t.desviacion);
        string x = util::fijo(t.media / mejor) + "x";
        string ee = util::mb(r.memExtraEstimada), em = util::mb(r.memExtraMedida);
        cout << "  " << nom << "  " << string(9 - std::min<size_t>(9, m.size()), ' ') << m
             << "  " << string(8 - std::min<size_t>(8, d.size()), ' ') << d
             << "  " << string(8 - std::min<size_t>(8, x.size()), ' ') << x
             << "  " << string(10 - std::min<size_t>(10, ee.size()), ' ') << ee
             << "  " << string(12 - std::min<size_t>(12, em.size()), ' ') << em << "\n";
    }
    cout << linea() << "\n  Complejidad teorica:\n";
    for (const auto& r : rs)
        cout << "   - " << r.nombre << ": tiempo " << r.bigOTiempo << "; espacio " << r.bigOEspacio
             << "\n";

    // Coherencia entre las salidas de los tres algoritmos
    bool iguales = true;
    for (size_t i = 1; i < rs.size(); ++i)
        if (rs[i].salida != rs[0].salida) iguales = false;
    bool todosBien = true;
    for (const auto& r : rs) todosBien = todosBien && r.ordenado && r.permutacion;
    cout << linea() << "\n";
    cout << "  Todas las salidas ordenadas y completas : " << (todosBien ? "SI" : "NO") << "\n";
    cout << "  Las salidas son identicas entre si      : " << (iguales ? "SI" : "NO") << "\n";
}

static void guardarResultados(const vector<Resultado>& rs, const Config& cfg, size_t n) {
    fs::create_directories(cfg.carpetaSalida);
    std::ofstream csv(cfg.carpetaSalida + "/resultados.csv");
    csv << "estrategia,n,repeticiones,media_ms,desviacion_ms,min_ms,max_ms,"
           "mem_total_estimada_bytes,mem_extra_estimada_bytes,mem_extra_medida_bytes,"
           "ordenado,permutacion\n";
    for (const auto& r : rs) {
        const util::Resumen t = util::resumir(r.tiempos);
        csv << r.nombre << "," << n << "," << r.tiempos.size() << "," << util::fijo(t.media, 3)
            << "," << util::fijo(t.desviacion, 3) << "," << util::fijo(t.minimo, 3) << ","
            << util::fijo(t.maximo, 3) << "," << r.memTotalEstimada << "," << r.memExtraEstimada
            << "," << r.memExtraMedida << "," << (r.ordenado ? "si" : "no") << ","
            << (r.permutacion ? "si" : "no") << "\n";
        util::escribirPalabras(cfg.carpetaSalida + "/ordenado_" + r.archivo + ".txt", r.salida);
    }
    cout << "\n  Archivos guardados en '" << cfg.carpetaSalida
         << "/': resultados.csv y ordenado_*.txt\n";
}

// ===========================================================================
// Experimento extra 1: escalabilidad (n = 10.000 ... 100.000)
// ===========================================================================
static void experimentoEscalabilidad(const vector<string>& datos, const Config& cfg) {
    cout << "\n" << linea('=') << "\n  EXPERIMENTO: ESCALABILIDAD (minimo de " << cfg.repeticiones << " repeticiones por punto)\n"
         << linea('=') << "\n";
    cout << "  Si el tiempo crece como n log n, la columna t/(n log2 n) debe ser casi constante.\n"
         << "  Se usa el MINIMO de las repeticiones: el ruido externo (otros procesos) solo\n"
         << "  puede sumar tiempo, asi que el minimo es la medida mas fiel del algoritmo.\n\n";
    cout << "        n   Quick(ms)  Heap(ms)   AVL(ms)  | ns por (n log2 n): Quick   Heap    AVL\n";
    cout << linea() << "\n";

    fs::create_directories(cfg.carpetaSalida);
    std::ofstream csv(cfg.carpetaSalida + "/escalabilidad.csv");
    csv << "n,quicksort_ms,heapsort_ms,avl_ms,quick_ns_nlogn,heap_ns_nlogn,avl_ns_nlogn\n";

    const int reps = cfg.repeticiones;
    for (size_t n = 10000; n <= datos.size(); n += 10000) {
        vector<string> sub(datos.begin(), datos.begin() + static_cast<std::ptrdiff_t>(n));
        double tq = 1e300, th = 1e300, ta = 1e300;   // minimo de las repeticiones
        for (int k = 0; k < reps; ++k) {
            { vector<string> v = sub; EstadisticasQuick e;
              auto t0 = util::Reloj::now(); quick::ordenar(v, e); auto t1 = util::Reloj::now();
              tq = std::min(tq, util::msDesde(t0, t1)); }
            { vector<string> v = sub; EstadisticasHeap e;
              auto t0 = util::Reloj::now(); auto s = heap::ordenar(std::move(v), e);
              auto t1 = util::Reloj::now(); th = std::min(th, util::msDesde(t0, t1)); }
            { vector<string> v = sub; EstadisticasAVL e; vector<string> s;
              auto t0 = util::Reloj::now();
              { ArbolAVL a(e); for (auto& w : v) a.insertar(std::move(w));
                s.reserve(a.elementos()); a.inorden(s);
                auto t1 = util::Reloj::now(); ta = std::min(ta, util::msDesde(t0, t1)); } }
        }
        const double nlogn = n * std::log2(static_cast<double>(n));
        const double cq = tq * 1e6 / nlogn, ch = th * 1e6 / nlogn, ca = ta * 1e6 / nlogn;
        std::printf("  %7zu  %9.2f %9.2f %9.2f  |                    %5.2f  %5.2f  %5.2f\n",
                    n, tq, th, ta, cq, ch, ca);
        csv << n << "," << util::fijo(tq, 3) << "," << util::fijo(th, 3) << "," << util::fijo(ta, 3)
            << "," << util::fijo(cq, 4) << "," << util::fijo(ch, 4) << "," << util::fijo(ca, 4)
            << "\n";
    }
    cout << "\n  Guardado en '" << cfg.carpetaSalida << "/escalabilidad.csv'\n";
}

// ===========================================================================
// Experimento extra 2: casos especiales de entrada
// ===========================================================================
static void experimentoCasos(const vector<string>& datos, const Config& cfg) {
    cout << "\n" << linea('=') << "\n  EXPERIMENTO: TIPO DE ENTRADA (n = " << util::conMiles(datos.size())
         << ", " << cfg.repeticiones << " repeticiones)\n" << linea('=') << "\n";

    // Entradas de prueba. La version ordenada se obtiene con NUESTRO QuickSort.
    vector<string> ordenada = datos;
    { EstadisticasQuick e; quick::ordenar(ordenada, e); }
    vector<string> inversa(ordenada.rbegin(), ordenada.rend());
    vector<string> repetidos;                 // solo 100 palabras distintas
    repetidos.reserve(datos.size());
    for (size_t i = 0; i < datos.size(); ++i) repetidos.push_back(datos[i % 100]);

    struct Entrada { string nombre; const vector<string>* v; };
    const vector<Entrada> entradas = {{"Aleatoria", &datos},
                                      {"Ya ordenada", &ordenada},
                                      {"Orden inverso", &inversa},
                                      {"Muchos repetidos (100 distintas)", &repetidos}};

    fs::create_directories(cfg.carpetaSalida);
    std::ofstream csv(cfg.carpetaSalida + "/casos_entrada.csv");
    csv << "entrada,quicksort_ms,heapsort_ms,avl_ms,avl_rotaciones\n";

    cout << "  Entrada                            Quick(ms)  Heap(ms)   AVL(ms)   Rotaciones AVL\n"
         << linea() << "\n";
    const int reps = cfg.repeticiones;
    for (const auto& en : entradas) {
        double tq = 0, th = 0, ta = 0;
        std::uint64_t rot = 0;
        for (int k = 0; k < reps; ++k) {
            { vector<string> v = *en.v; EstadisticasQuick e;
              auto t0 = util::Reloj::now(); quick::ordenar(v, e); auto t1 = util::Reloj::now();
              tq += util::msDesde(t0, t1); }
            { vector<string> v = *en.v; EstadisticasHeap e;
              auto t0 = util::Reloj::now(); auto s = heap::ordenar(std::move(v), e);
              auto t1 = util::Reloj::now(); th += util::msDesde(t0, t1); }
            { vector<string> v = *en.v; EstadisticasAVL e; vector<string> s;
              auto t0 = util::Reloj::now();
              { ArbolAVL a(e); for (auto& w : v) a.insertar(std::move(w));
                s.reserve(a.elementos()); a.inorden(s);
                auto t1 = util::Reloj::now(); ta += util::msDesde(t0, t1); }
              rot = e.rotacionesSimples + e.rotacionesDobles; }
        }
        tq /= reps; th /= reps; ta /= reps;
        string nom = en.nombre; nom.resize(33, ' ');
        std::printf("  %s %9.2f %9.2f %9.2f   %12s\n", nom.c_str(), tq, th, ta,
                    util::conMiles(rot).c_str());
        csv << en.nombre << "," << util::fijo(tq, 3) << "," << util::fijo(th, 3) << ","
            << util::fijo(ta, 3) << "," << rot << "\n";
    }

    // Por que importa el pivote: QuickSort con pivote = primer elemento.
    const size_t m = std::min<size_t>(20000, datos.size());
    vector<string> aleatM(datos.begin(), datos.begin() + static_cast<std::ptrdiff_t>(m));
    vector<string> ordM = aleatM;
    { EstadisticasQuick e; quick::ordenar(ordM, e); }

    cout << "\n  Importancia del pivote (n = " << util::conMiles(m) << "):\n";
    cout << "  Pivote / entrada                     Tiempo(ms)   Comparaciones   Prof. recursion\n"
         << linea() << "\n";
    struct Prueba { string nombre; const vector<string>* v; quick::Pivote p; };
    const vector<Prueba> pruebas = {
        {"Mediana de 3 / aleatoria", &aleatM, quick::Pivote::MedianaDeTres},
        {"Mediana de 3 / ya ordenada", &ordM, quick::Pivote::MedianaDeTres},
        {"Primer elemento / aleatoria", &aleatM, quick::Pivote::Primero},
        {"Primer elemento / ya ordenada", &ordM, quick::Pivote::Primero},
    };
    csv << "\npivote_entrada,n,ms,comparaciones,profundidad\n";
    for (const auto& p : pruebas) {
        vector<string> v = *p.v;
        EstadisticasQuick e;
        auto t0 = util::Reloj::now();
        quick::ordenar(v, e, p.p);
        auto t1 = util::Reloj::now();
        string nom = p.nombre; nom.resize(35, ' ');
        std::printf("  %s %10.2f   %13s   %15d\n", nom.c_str(), util::msDesde(t0, t1),
                    util::conMiles(e.comparaciones).c_str(), e.profundidadMaxima);
        csv << p.nombre << "," << m << "," << util::fijo(util::msDesde(t0, t1), 3) << ","
            << e.comparaciones << "," << e.profundidadMaxima << "\n";
    }
    cout << "\n  Con pivote = primero y entrada ordenada, cada particion deja 0 y n-1\n"
            "  elementos: ~n^2/2 comparaciones (peor caso O(n^2)). La mediana de tres\n"
            "  lo evita. Gracias a la recursion sobre la parte pequena, la pila no se\n"
            "  desborda ni en ese peor caso.\n";
    cout << "\n  Guardado en '" << cfg.carpetaSalida << "/casos_entrada.csv'\n";
}

// ===========================================================================
// Programa principal
// ===========================================================================
static void imprimirEncabezado(const vector<string>& datos, const Config& cfg) {
    cout << linea('=') << "\n  ORDENAMIENTO DE UN DATASET GRANDE - QuickSort / HeapSort / AVL\n"
         << linea('=') << "\n";
    cout << "  Dataset           : " << cfg.dataset << "\n";
    cout << "  Palabras leidas   : " << util::conMiles(datos.size()) << "\n";
    cout << "  sizeof(string)    : " << sizeof(string) << " bytes   (capacidad SSO: "
         << util::capacidadSSO() << " caracteres)\n";
    cout << "  Palabras > SSO    : " << util::conMiles(util::cuantasLargas(datos))
         << " (su texto va en un bloque aparte del heap: "
         << util::mb(util::bytesTextoExterno(datos)) << ")\n";
    cout << "  Repeticiones      : " << cfg.repeticiones << "\n";
#if defined(__clang__)
    cout << "  Compilador        : clang " << __clang_major__ << "." << __clang_minor__ << "\n";
#elif defined(__GNUC__)
    cout << "  Compilador        : g++ " << __VERSION__ << "\n";
#elif defined(_MSC_VER)
    cout << "  Compilador        : MSVC " << _MSC_VER << "\n";
#endif
#if (defined(__GNUC__) && !defined(__OPTIMIZE__)) || (defined(_MSC_VER) && defined(_DEBUG))
    cout << "  ADVERTENCIA: compilado SIN optimizacion. Use -O2 para medir tiempos reales.\n";
#endif
}

static vector<Resultado> correrLosTres(const vector<string>& datos, const Config& cfg,
                                       bool incluirExtra) {
    vector<Resultado> rs;
    cout << "\n  Ejecutando QuickSort...\n";
    rs.push_back(correrQuick(datos, cfg.repeticiones));
    cout << "  Ejecutando HeapSort...\n";
    rs.push_back(correrHeap(datos, cfg.repeticiones));
    cout << "  Ejecutando Arbol AVL...\n";
    rs.push_back(correrAVL(datos, cfg.repeticiones));
    if (incluirExtra) {
        cout << "  Ejecutando HeapSort in situ (extra)...\n";
        rs.push_back(correrHeapInSitu(datos, cfg.repeticiones));
    }
    for (auto& r : rs) {
        verificar(r, datos);
        imprimirResultado(r);
    }
    imprimirComparacion(rs);
    guardarResultados(rs, cfg, datos.size());
    return rs;
}

static int leerOpcion() {
    cout << "\n" << linea() << "\n  MENU\n"
         << "   1. QuickSort\n"
         << "   2. HeapSort (monticulo binario)\n"
         << "   3. Arbol AVL + inorden\n"
         << "   4. Los tres + analisis comparativo (+ HeapSort in situ)\n"
         << "   5. Experimento de escalabilidad (n = 10k .. 100k)\n"
         << "   6. Experimento de tipos de entrada y pivote\n"
         << "   0. Salir\n"
         << "  Opcion: " << std::flush;
    int op;
    if (!(std::cin >> op)) return 0;
    return op;
}

int main(int argc, char* argv[]) {
    Config cfg;
    bool todo = false;
    for (int i = 1; i < argc; ++i) {
        const string a = argv[i];
        if (a == "--todo") todo = true;
        else if (a == "--dataset" && i + 1 < argc) cfg.dataset = argv[++i];
        else if (a == "--salida" && i + 1 < argc) cfg.carpetaSalida = argv[++i];
        else if (a == "--reps" && i + 1 < argc) cfg.repeticiones = std::max(1, std::stoi(argv[++i]));
        else if (a == "--ayuda" || a == "-h") {
            cout << "Uso: " << argv[0] << " [--todo] [--dataset ruta] [--reps N] [--salida carpeta]\n";
            return 0;
        }
    }
    // Si no se encuentra la ruta por defecto, se intenta en la carpeta actual.
    if (!fs::exists(cfg.dataset) && fs::exists("dataset.txt")) cfg.dataset = "dataset.txt";

    vector<string> datos;
    try {
        datos = util::leerPalabras(cfg.dataset);
    } catch (const std::exception& ex) {
        std::cerr << "ERROR: " << ex.what() << "\n"
                  << "Ejecute el programa desde la carpeta del proyecto o use --dataset ruta\n";
        return 1;
    }
    if (datos.empty()) {
        std::cerr << "ERROR: el dataset esta vacio\n";
        return 1;
    }
    imprimirEncabezado(datos, cfg);

    if (todo) {
        correrLosTres(datos, cfg, true);
        experimentoEscalabilidad(datos, cfg);
        experimentoCasos(datos, cfg);
        return 0;
    }

    while (true) {
        const int op = leerOpcion();
        if (op == 0) break;
        vector<Resultado> rs;
        switch (op) {
            case 1: rs.push_back(correrQuick(datos, cfg.repeticiones)); break;
            case 2: rs.push_back(correrHeap(datos, cfg.repeticiones)); break;
            case 3: rs.push_back(correrAVL(datos, cfg.repeticiones)); break;
            case 4: correrLosTres(datos, cfg, true); continue;
            case 5: experimentoEscalabilidad(datos, cfg); continue;
            case 6: experimentoCasos(datos, cfg); continue;
            default: cout << "  Opcion no valida\n"; continue;
        }
        for (auto& r : rs) {
            verificar(r, datos);
            imprimirResultado(r);
        }
    }
    cout << "  Fin.\n";
    return 0;
}
