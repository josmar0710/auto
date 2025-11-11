#include "sdl_exception.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_error.h>
#include <string>
namespace jmauto {
SDLException::SDLException(const std::string &msg)
    : std::runtime_error(msg + " -> " + std::string{SDL_GetError()}) {}
} // namespace jmauto
