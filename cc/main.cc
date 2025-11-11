#include "init.h"
#include "loop.h"
#include "sdl_exception.h"
#include "state.h"
#include <exception>
#include <iostream>
namespace jmauto {
void Main() {
  State stateobj{};
  state = &stateobj;
  Init();
  if (!SDL_ShowWindow(state->window)) {
    throw SDLException("show window");
  }
  state->must_run = true;
  while (state->must_run) {
    Loop();
  }
}
} // namespace jmauto

int main() {
  try {
    jmauto::Main();
    return 0;
  } catch (const std::exception &error) {
    std::cerr << "Main caught error: " << error.what() << "\n";
  }
  return 1;
}
