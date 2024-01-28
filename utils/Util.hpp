#ifndef __UTIL_HPP__
#define __UTIL_HPP__

#include <sstream>
#include <stdexcept>
#include <string>

class Util {
 public:
  static std::string const itos(int n);
  static int stoi(std::string const& str);
  static std::string const removeDotSegments(std::string const& path);

 private:
  Util(void);
  Util(Util const& util);
  ~Util(void);
  Util& operator=(Util const& util);
};

#endif