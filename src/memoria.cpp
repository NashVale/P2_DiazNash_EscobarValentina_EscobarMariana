// memoria.cpp
// Reemplaza new/delete y guarda el tamaño en una cabecera para contar memoria. 

#include "memoria.hpp"

#include <cstdlib>
#include <new>

namespace {

std::size_t g_actual = 0;
std::size_t g_pico = 0;
std::size_t g_asignaciones = 0;

constexpr std::size_t CABECERA = alignof(std::max_align_t);

void* reservar(std::size_t n) {
    void* bloque = std::malloc(n + CABECERA);
    if (bloque == nullptr) throw std::bad_alloc();
    *static_cast<std::size_t*>(bloque) = n;
    g_actual += n;
    ++g_asignaciones;
    if (g_actual > g_pico) g_pico = g_actual;
    return static_cast<char*>(bloque) + CABECERA;
}

void liberar(void* p) noexcept {
    if (p == nullptr) return;
    char* bloque = static_cast<char*>(p) - CABECERA;
    g_actual -= *reinterpret_cast<std::size_t*>(bloque);
    std::free(bloque);
}

}  

namespace memoria {
std::size_t actual() { return g_actual; }
std::size_t pico() { return g_pico; }
void reiniciarPico() { g_pico = g_actual; }
std::size_t asignaciones() { return g_asignaciones; }
}  

// Operadores reemplazados
void* operator new(std::size_t n) { return reservar(n); }
void* operator new[](std::size_t n) { return reservar(n); }
void* operator new(std::size_t n, const std::nothrow_t&) noexcept {
    try { return reservar(n); } catch (...) { return nullptr; }
}
void* operator new[](std::size_t n, const std::nothrow_t&) noexcept {
    try { return reservar(n); } catch (...) { return nullptr; }
}
void operator delete(void* p) noexcept { liberar(p); }
void operator delete[](void* p) noexcept { liberar(p); }
void operator delete(void* p, std::size_t) noexcept { liberar(p); }
void operator delete[](void* p, std::size_t) noexcept { liberar(p); }
void operator delete(void* p, const std::nothrow_t&) noexcept { liberar(p); }
void operator delete[](void* p, const std::nothrow_t&) noexcept { liberar(p); }
