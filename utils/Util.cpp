#include "Util.hpp"

std::string const Util::itos(int n) {
  std::stringstream ss;
  ss << n;
  return ss.str();
}
