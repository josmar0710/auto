#pragma once
#include <functional>
#include <stack>
namespace jmauto {
class Releaser {
public:
  ~Releaser();
  void Push(const std::function<void()> &func);

private:
  std::stack<std::function<void()>> funcs;
};
} // namespace jmauto
