#ifndef __CONFIG_PARSER_HPP__
#define __CONFIG_PARSER_HPP__

#include <sys/stat.h>
#include <unistd.h>

#include <algorithm>
#include <cstring>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "Location.hpp"
#include "Server.hpp"

// ConfigParser 클래스
// - HTTP Config를 파싱한 결과 (std::vector<Server> 객체 반환)
class ConfigParser {
 private:
  std::string const _path;
  std::vector<Server> _servers;
  std::string _storageBuffer;

 public:
  ConfigParser(char const* configPath);
  ~ConfigParser(void);
  std::vector<Server> parse(void);

 private:
  static int const BUFFER_SIZE = 4096;

  ConfigParser(void);
  ConfigParser(ConfigParser const& ConfigParser);
  ConfigParser& operator=(ConfigParser const& ConfigParser);

  void readConfigFile(void);
  bool isRegularFile(std::string const& path);
};

#endif
