#ifndef __CONFIG_PARSER_HPP__
#define __CONFIG_PARSER_HPP__

#include <sys/stat.h>
#include <unistd.h>

#include <algorithm>
#include <cctype>
#include <cstring>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "../utils/Enum.hpp"
#include "../utils/Util.hpp"
#include "Location.hpp"
#include "Server.hpp"

// ConfigParser 클래스
// - HTTP Config를 파싱한 결과 (std::vector<Server> 객체 반환)
class ConfigParser {
 private:
  static int const BUFFER_SIZE = 4096;

  std::string const _path;
  std::vector<Server> _servers;
  std::string _projectRootPath;

  std::stringstream _bufferStream;
  std::string _line;

 public:
  ConfigParser(char const* configPath);
  ~ConfigParser(void);
  std::vector<Server> parse(void);

 private:
  ConfigParser(void);
  ConfigParser(ConfigParser const& ConfigParser);
  ConfigParser& operator=(ConfigParser const& ConfigParser);

  void storeProjectRoot(void);
  void storeServerListenInfo(Server& server);
  void storeServerName(Server& server);

  void storeLocationUri(Location& location);
  void storeLocationRoot(Location& location);
  void storeLocationIndex(Location& location);
  void storeLocationMethod(Location& location);
  void storeLocationBodySize(Location& location);
  void storeLocationAutoindex(Location& location);
  void storeLocationErrorPage(Location& location);
  void storeLocationCgi(Location& location);
  void storeLocationRedirect(Location& location);

  void parseServer(Server& server);
  Location const parseLocation(void);

  void readConfigFile(void);
  bool isRegularFile(std::string const& path);
  bool isStartsWith(std::string const& str, std::string const& prefix);

  void split(std::string const& line, std::vector<std::string>& result);
  void checkSize(std::vector<std::string> const& result, size_t size);
  void removeSemicolon(void);

  void throwFormatError(std::string const& func);
};

#endif
