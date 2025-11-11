#include "sdl_wgpu.h"
#include "sdl_exception.h"
#include "state.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_video.h>
#include <stdexcept>
#include <webgpu/wgpu.h>
namespace jmauto {
WGPUSurface GetSurface() {
  SDL_PropertiesID props = SDL_GetWindowProperties(state->window);
  if (!props) {
    throw SDLException("get window properties");
  }
  const char *driver = SDL_GetCurrentVideoDriver();
  if (!driver) {
    throw std::runtime_error("get video driver");
  }
  if (SDL_strcmp(driver, "x11") == 0) {
    void *x11_display = SDL_GetPointerProperty(
        props, SDL_PROP_WINDOW_X11_DISPLAY_POINTER, nullptr);
    if (x11_display == nullptr) {
      throw std::runtime_error("x11 display pointer");
    }
    uint64_t x11_window =
        SDL_GetNumberProperty(props, SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0);
    if (x11_window == 0) {
      throw std::runtime_error("x11 window number");
    }

    WGPUSurfaceSourceXlibWindow fromXlibWindow = {};
    fromXlibWindow.chain.sType = WGPUSType_SurfaceSourceXlibWindow;
    fromXlibWindow.chain.next = nullptr;
    fromXlibWindow.display = x11_display;
    fromXlibWindow.window = x11_window;

    WGPUSurfaceDescriptor surfaceDescriptor = {};
    surfaceDescriptor.nextInChain = &fromXlibWindow.chain;
    surfaceDescriptor.label = (WGPUStringView){nullptr, WGPU_STRLEN};

    return wgpuInstanceCreateSurface(state->instance, &surfaceDescriptor);
  } else if (SDL_strcmp(driver, "wayland") == 0) {
    void *wayland_display = SDL_GetPointerProperty(
        props, SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER, nullptr);
    if (wayland_display == nullptr) {
      throw std::runtime_error("wayland display pointer");
    }
    void *wayland_surface = SDL_GetPointerProperty(
        props, SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER, nullptr);
    if (wayland_surface == nullptr) {
      throw std::runtime_error("wayland surface pointer");
    }

    WGPUSurfaceSourceWaylandSurface fromWaylandSurface = {};
    fromWaylandSurface.chain.sType = WGPUSType_SurfaceSourceWaylandSurface;
    fromWaylandSurface.chain.next = nullptr;
    fromWaylandSurface.display = wayland_display;
    fromWaylandSurface.surface = wayland_surface;

    WGPUSurfaceDescriptor surfaceDescriptor;
    surfaceDescriptor.nextInChain = &fromWaylandSurface.chain;
    surfaceDescriptor.label = (WGPUStringView){nullptr, WGPU_STRLEN};

    return wgpuInstanceCreateSurface(state->instance, &surfaceDescriptor);
  }
  throw std::runtime_error("wrong video driver");
}
} // namespace jmauto
