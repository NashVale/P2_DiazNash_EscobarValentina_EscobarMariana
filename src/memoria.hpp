// memoria.hpp
// ---------------------------------------------------------------------------
// Medidor de memoria dinamica (heap) del programa.
//
// En memoria.cpp se reemplazan los operadores globales new/delete de C++.
// Cada vez que el programa pide memoria (un nodo del AVL, el buffer de un
// std::vector, el texto largo de un std::string, ...) se suma el tamano
// pedido a un contador; cada vez que se libera, se resta. Asi podemos saber
// cuanta memoria EXTRA reservo cada algoritmo, de forma portable (funciona
// igual en Windows, Linux y macOS) y sin herramientas externas.
//
// Limitacion (importante para la sustentacion): se cuentan los bytes que el
// programa PIDE. El sistema operativo / malloc agrega un pequeno sobrecosto
// por bloque (~8-16 bytes) que aqui no aparece.
// ---------------------------------------------------------------------------
#pragma once
#include <cstddef>

namespace memoria {

// Bytes reservados en este momento.
std::size_t actual();

// Maximo de bytes reservados al mismo tiempo desde el ultimo reinicio.
std::size_t pico();

// Hace pico() = actual(), para empezar a medir una nueva fase.
void reiniciarPico();

// Numero de llamadas a new desde que arranco el programa.
std::size_t asignaciones();

}  // namespace memoria
