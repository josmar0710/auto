#pragma once
#include <functional>
namespace jmauto {
class Defer {
public:
  Defer(const std::function<void()> &func);
  ~Defer();

private:
  std::function<void()> _func;
};
} // namespace jmauto
