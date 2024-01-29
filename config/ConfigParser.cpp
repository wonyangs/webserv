#include "ConfigParser.hpp"

/**
 * Constructor & Destructor
 */

ConfigParser::ConfigParser(char const* configPath) : _path(configPath) {}

ConfigParser::~ConfigParser(void) {}

/**
 * Public method
 */

#include <iostream>

void ConfigParser::storeProjectRoot(void) {
  std::getline(_bufferStream, _line);

  if (isStartsWith(_line, "project_root ") == false) {
    throw std::runtime_error(
        "[1201] ConfigParser: parse - The first line of config must contain "
        "project_root");
  }

  std::vector<std::string> result;
  split(_line, result);
  checkSize(result, 2);
  if (Util::isValidPath(result[1]) == false)
    throw std::runtime_error(
        "[1210] ConfigParser: storeProjectRoot - invalid path");
  _projectRootPath = result[1];
}

std::vector<Server> ConfigParser::parse(void) {
  readConfigFile();

  storeProjectRoot();
  while (std::getline(_bufferStream, _line)) {
    if (_line == "") continue;

    if (_line == "server {") {
      _servers.push_back(Server());
      parseServer(_servers.back());
      continue;
    }

    throwFormatError("ConfigParser: parse");
  }

  return _servers;
}

void ConfigParser::parseServer(Server& server) {
  while (std::getline(_bufferStream, _line)) {
    if (_line == "}") break;
    if (_line == "") continue;

    if (isStartsWith(_line, "\tlocation ")) {
      server.addLocationBlock(parseLocation());
      continue;
    }

    removeSemicolon();
    if (isStartsWith(_line, "\tlisten ")) {
      storeServerListenInfo(server);
    } else if (isStartsWith(_line, "\tserver_name ")) {
      storeServerName(server);
    } else {
      throwFormatError("ConfigParser: parseServer");
    }
  }

  if (server.isRequiredValuesSet() == false) {
    throw std::runtime_error(
        "[1202] ConfigParser: parseServer - host ip and port directives and "
        "root location block are required");
  }
}

Location const ConfigParser::parseLocation(void) {
  std::string const directives[8] = {
      "\t\troot ",         "\t\tindex",
      "\t\tallow_method ", "\t\tclient_max_body_size ",
      "\t\tautoindex ",    "\t\terror_page ",
      "\t\tcgi ",          "\t\tredirect ",
  };

  void (ConfigParser::*func[8])(Location& location) = {
      &ConfigParser::storeLocationRoot,
      &ConfigParser::storeLocationIndex,
      &ConfigParser::storeLocationMethod,
      &ConfigParser::storeLocationBodySize,
      &ConfigParser::storeLocationAutoindex,
      &ConfigParser::storeLocationErrorPage,
      &ConfigParser::storeLocationCgi,
      &ConfigParser::storeLocationRedirect};

  int cnt[8] = {0, 0, 0, 0, 0, 0, 0, 0};

  Location location(_projectRootPath);
  storeLocationUri(location);

  while (std::getline(_bufferStream, _line)) {
    if (_line == "\t}") break;
    if (_line == "") continue;

    removeSemicolon();
    for (int i = 0; i < 8; i++) {
      if (isStartsWith(_line, directives[i])) {
        cnt[i]++;
        if (i != 5 and 1 < cnt[i])
          throw std::runtime_error(
              "[1203] ConfigParser: parseLocation - duplicate directive");
        (this->*func[i])(location);
        break;
      }

      if (i == 7) {
        throwFormatError("ConfigParser: parseLocation");
      }
    }
  }

  location.printConfiguration();  // debug

  if (location.isRequiredValuesSet() == false) {
    throw std::runtime_error(
        "[1204] ConfigParser: parseLocation - root and index directives are "
        "required");
  }
  return location;
}

void ConfigParser::storeLocationRoot(Location& location) {
  std::vector<std::string> result;
  split(_line, result);
  checkSize(result, 2);

  location.setRootPath(result[1]);
}

void ConfigParser::storeLocationIndex(Location& location) {
  std::vector<std::string> result;
  split(_line, result);
  checkSize(result, 2);

  location.setIndexFile(result[1]);
}

void ConfigParser::storeLocationMethod(Location& location) {
  std::vector<std::string> result;
  split(_line, result);

  if (result.size() == 1) {
    throwFormatError("ConfigParser: storeLocationMethod");
  }

  for (size_t i = 1; i < result.size(); i++) {
    location.addAllowMethod(result[i]);
  }
}

void ConfigParser::storeLocationBodySize(Location& location) {
  std::vector<std::string> result;
  split(_line, result);
  checkSize(result, 2);

  location.setMaxBodySize(Util::stoi(result[1]));
}

void ConfigParser::storeLocationAutoindex(Location& location) {
  std::vector<std::string> result;
  split(_line, result);
  checkSize(result, 2);

  location.setAutoIndex(result[1]);
}

void ConfigParser::storeLocationErrorPage(Location& location) {
  std::vector<std::string> result;
  split(_line, result);

  if (result.size() == 1) {
    throwFormatError("ConfigParser: storeLocationErrorPage");
  }

  for (size_t i = 1; i < result.size() - 1; i++) {
    location.addErrorPage(Util::stoi(result[i]), result.back());
  }
}

void ConfigParser::storeLocationCgi(Location& location) {
  std::vector<std::string> result;
  split(_line, result);
  checkSize(result, 4);

  location.setCgiExtension(result[1]);
  location.setCgiPath(result[2]);
  location.setUploadDir(result[3]);
}

void ConfigParser::storeLocationRedirect(Location& location) {
  std::vector<std::string> result;
  split(_line, result);
  checkSize(result, 2);

  location.setRedirectUri(result[1]);
}

void ConfigParser::storeServerListenInfo(Server& server) {
  std::vector<std::string> result;
  split(_line, result);
  checkSize(result, 3);

  server.setHostIp(result[1]);
  server.setPort(result[2]);
}

void ConfigParser::storeServerName(Server& server) {
  std::vector<std::string> result;
  split(_line, result);

  // 빈 값이면 예외
  for (size_t i = 1; i < result.size(); i++) {
    Util::toLowerCase(result[i]);
    server.addServerName(result[i]);
  }
}

void ConfigParser::storeLocationUri(Location& location) {
  std::vector<std::string> result;
  split(_line, result);
  checkSize(result, 3);

  if (result[2] != "{") throwFormatError("ConfigParser: storeLocationUri");

  location.setUri(result[1]);
}

// Config 파일 읽기
// - 읽기에 실패한 경우 예외 발생
void ConfigParser::readConfigFile(void) {
  std::stringstream ss;
  std::ifstream file(_path.c_str());
  char buffer[BUFFER_SIZE];

  if (!isRegularFile(_path)) {
    throw std::runtime_error(
        "[1205] ConfigParser: readConfigFile - not a regular file: " + _path);
  }

  if (!file or !file.is_open() or file.fail()) {
    throw std::runtime_error(
        "[1206] ConfigParser: readConfigFile - unable to open config file: " +
        _path);
  }

  while (file.good()) {
    std::memset(buffer, 0, BUFFER_SIZE);
    file.read(buffer, BUFFER_SIZE);

    std::streamsize size = file.gcount();
    _bufferStream.write(buffer, size);

    if (!file.eof() and file.fail())
      throw std::runtime_error(
          "[1207] ConfigParser: readConfigFile - problem reading a file: " +
          _path);
  }
}

// 일반 파일인지 확인
bool ConfigParser::isRegularFile(std::string const& path) {
  if (access(path.c_str(), F_OK) == -1)
    throw std::runtime_error(
        "[1208] ConfigParser: isRegularFile - file not exist: " + path);

  struct stat statbuf;
  if (stat(path.c_str(), &statbuf) == -1) {
    throw std::runtime_error(
        "[1209] ConfigParser: isRegularFile - stat failed");
  }
  return S_ISREG(statbuf.st_mode);
}

bool ConfigParser::isStartsWith(std::string const& str,
                                std::string const& prefix) {
  return str.find(prefix) == 0;
}

void ConfigParser::split(std::string const& line,
                         std::vector<std::string>& result) {
  std::stringstream ss(line);
  std::string token;

  while (std::getline(ss, token, ' ')) {
    if (token == "") throwFormatError("ConfigParser: split");
    result.push_back(token);
  }
}

void ConfigParser::checkSize(std::vector<std::string> const& result,
                             size_t size) {
  if (result.size() != size) {
    throwFormatError("ConfigParser: checkSize");
  }
}

void ConfigParser::removeSemicolon(void) {
  if (_line.back() != ';') {
    throwFormatError("ConfigParser: removeSemicolon");
  }

  _line.pop_back();
}

void ConfigParser::throwFormatError(std::string const& func) {
  throw std::runtime_error("[1200] " + func +
                           " - config file format is incorrect: " + _line);
}
