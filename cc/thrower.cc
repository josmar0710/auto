#include "thrower.h"
#include <iostream>
#include <stdexcept>
namespace jmauto {
Thrower::Thrower(
    const std::function<void(
        const std::function<void(const std::runtime_error &error)> &pusher)>
        &func) {
  std::runtime_error native{""};
  bool native_throw{false};
  try {
    func([&](const std::runtime_error &error) { _errors.push(error); });
  } catch (const std::runtime_error &error) {
    native = error;
    native_throw = true;
  }
  bool throwed = !_errors.empty();
  while (!_errors.empty()) {
    std::cerr << "Thrower error: " << _errors.front().what() << "\n";
    _errors.pop();
  }
  if (native_throw) {
    throw native;
  }
  if (throwed) {
    throw std::runtime_error("Thrower throwed");
  }
}
} // namespace jmauto
