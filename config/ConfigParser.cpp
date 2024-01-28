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

std::vector<Server> ConfigParser::parse(void) {
  readConfigFile();
  std::cout << _storageBuffer << std::endl;
  return _servers;
}

// Config 파일 읽기
// - 읽기에 실패한 경우 예외 발생
void ConfigParser::readConfigFile(void) {
  std::stringstream ss;
  std::ifstream file(_path);
  char buffer[BUFFER_SIZE];

  if (!isRegularFile(_path)) {
    throw std::runtime_error(
        "[] ConfigParser: readConfigFile - not a regular file " + _path);
  }

  if (!file or !file.is_open() or file.fail()) {
    throw std::runtime_error(
        "[] ConfigParser: readConfigFile - unable to open config file: " +
        _path);
  }

  while (file.good()) {
    std::memset(buffer, 0, BUFFER_SIZE);
    file.read(buffer, BUFFER_SIZE);

    std::streamsize size = file.gcount();
    _storageBuffer.append(buffer, size);

    if (!file.eof() and file.fail())
      throw std::runtime_error(
          "[] ConfigParser: readConfigFile - problem reading a file: " + _path);
  }
}

bool ConfigParser::isRegularFile(std::string const& path) {
  if (access(path.c_str(), F_OK) == -1)
    throw std::runtime_error("[] ConfigParser: isFile - file not exist: " +
                             _path);

  struct stat statbuf;

  if (stat(path.c_str(), &statbuf) == -1) {
    throw std::runtime_error("[] ConfigParser: isFile - stat failed");
  }

  return S_ISREG(statbuf.st_mode);
}