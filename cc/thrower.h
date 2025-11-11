#pragma once
#include <functional>
#include <queue>
#include <stdexcept>
namespace jmauto {
class Thrower {
public:
  Thrower(
      const std::function<void(
          const std::function<void(const std::runtime_error &error)> &pusher)>
          &func);

private:
  std::queue<std::runtime_error> _errors;
};
} // namespace jmauto
