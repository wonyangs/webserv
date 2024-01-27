#include "Util.hpp"

std::string const Util::itos(int n) {
  std::stringstream ss;
  ss << n;
  return ss.str();
}

int Util::stoi(std::string const& str) {
  std::stringstream ss;
  int n;
  ss << str;
  ss >> n;

  if (ss.fail() or !ss.eof()) {
    throw std::runtime_error("[6102] Util: stoi - invalid str: " + str);
  }
  return n;
}
