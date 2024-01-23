#ifndef __UTIL_HPP__
#define __UTIL_HPP__

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

#include "../utils/StatusException.hpp"

typedef std::vector<u_int8_t> octetVec;
typedef std::vector<std::string> stringVec;

class Util {
 public:
  static std::string const itos(int n);

  static stringVec split(octetVec const& vec, char del);
  static stringVec splitOnce(octetVec const& vec, char del);

 private:
  Util(void);
  Util(Util const& util);
  ~Util(void);
  Util& operator=(Util const& util);
};

#endif