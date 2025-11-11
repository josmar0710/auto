#pragma once
#include <stdexcept>
#include <string>
namespace jmauto {
class SDLException : public std::runtime_error {
public:
  SDLException(const std::string &msg);
};
} // namespace jmauto
