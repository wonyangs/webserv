#ifndef __UTIL_HPP__
#define __UTIL_HPP__

#include <sstream>
#include <string>

class Util {
 public:
  static std::string const itos(int n);

 private:
  Util(void);
  Util(Util const& util);
  ~Util(void);
  Util& operator=(Util const& util);
};

#endif