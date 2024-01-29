#ifndef __UTIL_HPP__
#define __UTIL_HPP__

#include <cctype>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <string>

#include "Enum.hpp"

class Util {
 public:
  static std::string const itos(int n);
  static int stoi(std::string const& str);
  static void toUpperCase(std::string& str);
  static void toLowerCase(std::string& str);
  static std::string const removeDotSegments(std::string const& path);
  static bool isHex(char ch);
  static char hexToChar(std::string const& hexStr);
  static bool isPctEncoded(std::string const& str);
  static bool isUnreserved(char ch);
  static bool isSubDelims(char ch);
  static std::string pctDecode(std::string const& str);
  static bool isValidPath(std::string const& path);
  static bool isValidQuery(std::string const& query);
  static EHttpMethod matchEHttpMethod(std::string method);

 private:
  Util(void);
  Util(Util const& util);
  ~Util(void);
  Util& operator=(Util const& util);
};

#endif