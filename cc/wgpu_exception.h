#pragma once
#include <stdexcept>
#include <string>
namespace jmauto {
class WGPUException : public std::runtime_error {
public:
  WGPUException(const std::string &msg);
};
} // namespace jmauto
