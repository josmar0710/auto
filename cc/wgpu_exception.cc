#include "wgpu_exception.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_error.h>
#include <string>
namespace jmauto {
WGPUException::WGPUException(const std::string &msg)
    : std::runtime_error("wgpu error -> " + msg) {}
} // namespace jmauto
