// utilidades.hpp
// ---------------------------------------------------------------------------
// Funciones de apoyo: lectura del dataset, verificacion de resultados,
// estadisticas de tiempo y estimacion de memoria con sizeof.
// ---------------------------------------------------------------------------
#pragma once
#include <chrono>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace util {

// --- Lectura ----------------------------------------------------------------
// Lee una palabra por linea. Quita el '\r' final: el dataset del profesor
// viene con finales de linea de Windows (\r\n) y, sin esto, en Linux/macOS
// cada palabra quedaria con un caracter invisible al final.
inline std::vector<std::string> leerPalabras(const std::string& ruta) {
    std::ifstream in(ruta);
    if (!in) throw std::runtime_error("No se pudo abrir el archivo: " + ruta);
    std::vector<std::string> palabras;
    palabras.reserve(100000);
    std::string linea;
    while (std::getline(in, linea)) {
        while (!linea.empty() &&
               (linea.back() == '\r' || linea.back() == ' ' || linea.back() == '\t'))
            linea.pop_back();
        std::size_t ini = 0;
        while (ini < linea.size() && (linea[ini] == ' ' || linea[ini] == '\t')) ++ini;
        if (ini > 0) linea.erase(0, ini);
        if (!linea.empty()) palabras.push_back(linea);
    }
    return palabras;
}

inline void escribirPalabras(const std::string& ruta, const std::vector<std::string>& v) {
    std::ofstream out(ruta);
    if (!out) throw std::runtime_error("No se pudo escribir: " + ruta);
    for (const auto& s : v) out << s << '\n';
}

// --- Verificacion -----------------------------------------------------------
inline bool estaOrdenado(const std::vector<std::string>& v) {
    for (std::size_t i = 1; i < v.size(); ++i)
        if (v[i] < v[i - 1]) return false;
    return true;
}

// Verifica que 'salida' tenga exactamente las mismas palabras (con las mismas
// repeticiones) que 'entrada'. Usa una tabla hash: O(n) y NO depende de
// ningun ordenamiento, asi la verificacion es independiente de los algoritmos.
inline bool esPermutacion(const std::vector<std::string>& entrada,
                          const std::vector<std::string>& salida) {
    if (entrada.size() != salida.size()) return false;
    std::unordered_map<std::string, long long> conteo;
    conteo.reserve(entrada.size() * 2);
    for (const auto& s : entrada) ++conteo[s];
    for (const auto& s : salida) {
        auto it = conteo.find(s);
        if (it == conteo.end() || --it->second < 0) return false;
    }
    return true;
}

// --- Tiempo -----------------------------------------------------------------
using Reloj = std::chrono::steady_clock;  // monotono: no lo afectan cambios de hora

inline double msDesde(Reloj::time_point t0, Reloj::time_point t1) {
    return std::chrono::duration<double, std::milli>(t1 - t0).count();
}

struct Resumen { double media = 0, desviacion = 0, minimo = 0, maximo = 0; };

inline Resumen resumir(const std::vector<double>& xs) {
    Resumen r;
    if (xs.empty()) return r;
    r.minimo = r.maximo = xs[0];
    for (double x : xs) {
        r.media += x;
        if (x < r.minimo) r.minimo = x;
        if (x > r.maximo) r.maximo = x;
    }
    r.media /= static_cast<double>(xs.size());
    if (xs.size() > 1) {
        double s = 0;
        for (double x : xs) s += (x - r.media) * (x - r.media);
        r.desviacion = std::sqrt(s / static_cast<double>(xs.size() - 1));
    }
    return r;
}

// --- Memoria (estimacion con sizeof) ---------------------------------------
// Un std::string guarda los textos cortos DENTRO del propio objeto (Small
// String Optimization, SSO). Solo los textos mas largos que esa capacidad
// usan un bloque extra en el heap. Capacidad SSO tipica: 15 caracteres en
// GCC y MSVC, 22 en Clang/libc++.
inline std::size_t capacidadSSO() { return std::string().capacity(); }

inline std::size_t bytesTextoExterno(const std::vector<std::string>& v) {
    const std::size_t sso = capacidadSSO();
    std::size_t total = 0;
    for (const auto& s : v)
        if (s.size() > sso) total += s.size() + 1;  // +1 por el '\0'
    return total;
}

inline std::size_t cuantasLargas(const std::vector<std::string>& v) {
    const std::size_t sso = capacidadSSO();
    std::size_t c = 0;
    for (const auto& s : v) if (s.size() > sso) ++c;
    return c;
}

// --- Formato ---------------------------------------------------------------
inline std::string mb(std::size_t bytes) {
    std::ostringstream o;
    o << std::fixed << std::setprecision(2) << bytes / (1024.0 * 1024.0) << " MB";
    return o.str();
}

inline std::string conMiles(unsigned long long x) {
    std::string s = std::to_string(x), r;
    int k = 0;
    for (auto it = s.rbegin(); it != s.rend(); ++it) {
        if (k > 0 && k % 3 == 0) r.insert(r.begin(), '.');
        r.insert(r.begin(), *it);
        ++k;
    }
    return r;
}

inline std::string fijo(double x, int dec = 2) {
    std::ostringstream o;
    o << std::fixed << std::setprecision(dec) << x;
    return o.str();
}

}  // namespace util
