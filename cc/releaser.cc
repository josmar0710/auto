#include "releaser.h"
namespace jmauto {
Releaser::~Releaser() {
  while (!this->funcs.empty()) {
    this->funcs.top()();
    this->funcs.pop();
  }
}
void Releaser::Push(const std::function<void()> &func) {
  this->funcs.push(func);
}
} // namespace jmauto
