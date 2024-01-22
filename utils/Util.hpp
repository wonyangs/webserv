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
  static stringVec splitVector(octetVec const& vec, char del);
  static stringVec splitVectorOnce(octetVec const& vec, char del);

 private:
  Util(void);
  Util(Util const& util);
  ~Util(void);
  Util& operator=(Util const& util);
};

#endif