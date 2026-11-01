// memoria.hpp

#pragma once
#include <cstddef>

namespace memoria {

// Bytes reservados en este momento.
std::size_t actual();

// Máximo de bytes reservados al mismo tiempo desde el último reinicio.
std::size_t pico();

// Hace pico() = actual(), para empezar a medir una nueva fase.
void reiniciarPico();

// Número de llamadas a new desde que arranco el programa.
std::size_t asignaciones();

}  
