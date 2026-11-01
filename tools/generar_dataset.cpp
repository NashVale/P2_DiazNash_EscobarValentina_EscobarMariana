
// generar_dataset.cpp
// Crea dataset.txt con N palabras elegidas al azar de words_alpha.txt.
// Uso: generar_dataset words_alpha.txt data/dataset.txt [N=100000] [semilla=2026]

#include <cstdint>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <utility>
#include <vector>

// Genera un entero aleatorio entre 0 y limite-1 sin sesgo.
// Se utiliza el método de rechazo para evitar que algunos números
// tengan mayor probabilidad de aparecer que otros.
static std::uint64_t enteroUniforme(std::mt19937_64& gen, std::uint64_t limite) {
    const std::uint64_t maximo = UINT64_MAX - (UINT64_MAX % limite);
    std::uint64_t x;

    // Si el número generado supera el rango permitido, se descarta
    // y se genera otro para mantener una distribución uniforme.
    do { x = gen(); } while (x >= maximo);

    return x % limite;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Uso: " << argv[0] << " words_alpha.txt dataset.txt [N] [semilla]\n";
        return 1;
    }

    const std::string entrada = argv[1], salida = argv[2];
    const std::size_t n = argc > 3 ? std::stoull(argv[3]) : 100000;
    const std::uint64_t semilla = argc > 4 ? std::stoull(argv[4]) : 2026;

    std::ifstream in(entrada);
    if (!in) {
        std::cerr << "No se pudo abrir " << entrada << "\n";
        return 1;
    }

    std::vector<std::string> palabras;
    std::string linea;

    // Lee el archivo línea por línea y almacena las palabras
    // no vacías en un vector para poder seleccionarlas después.
    while (std::getline(in, linea)) {
        while (!linea.empty() && (linea.back() == '\r' || linea.back() == ' '))
            linea.pop_back();

        if (!linea.empty())
            palabras.push_back(linea);
    }

    if (palabras.size() < n) {
        std::cerr << "El archivo solo tiene " << palabras.size() << " palabras\n";
        return 1;
    }

    std::mt19937_64 gen(semilla);

    // Selección aleatoria sin repetición mediante Fisher-Yates parcial:
    // en cada iteración se elige una posición aleatoria de la parte
    // que todavía no ha sido seleccionada y se intercambian sus elementos.
    for (std::size_t i = 0; i < n; ++i) {
        const std::size_t j = i + enteroUniforme(gen, palabras.size() - i);
        std::swap(palabras[i], palabras[j]);
    }

    std::ofstream out(salida);
    if (!out) {
        std::cerr << "No se pudo escribir " << salida << "\n";
        return 1;
    }

    // Escribe las primeras N palabras seleccionadas en el archivo de salida.
    for (std::size_t i = 0; i < n; ++i)
        out << palabras[i] << '\n';

    std::cout << "Leidas " << palabras.size() << " palabras de " << entrada << "\n"
              << "Escritas " << n << " palabras aleatorias en " << salida
              << " (semilla " << semilla << ")\n";

    return 0;
}
