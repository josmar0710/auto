#include "defer.h"
#include <functional>
namespace jmauto {
Defer::Defer(const std::function<void()> &func) : _func(func) {}
Defer::~Defer() { _func(); }
} // namespace jmauto
