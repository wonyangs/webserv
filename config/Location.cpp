#include "Location.hpp"

#include <stdexcept>

// Constructor & Destructor

Location::Location(void) {}

Location::Location(std::string const& projectRootPath)
    : _projectRootPath(projectRootPath),
      _maxBodySize(DEFAULT_MAX_BODY_SIZE),
      _autoIndex(false),
      _cgiFlag(false),
      _hasAllowMethodField(false),
      _hasRedirectField(false) {}

Location::Location(std::string const& uri, std::string const& rootPath,
                   std::string const& indexFile)
    : _projectRootPath(""),
      _uri(uri),
      _rootPath(rootPath),
      _indexFile(indexFile),
      _maxBodySize(DEFAULT_MAX_BODY_SIZE),
      _autoIndex(false),
      _cgiFlag(false),
      _hasAllowMethodField(false),
      _hasRedirectField(false) {}

Location::Location(Location const& location) { *this = location; }

Location::~Location(void) {}

// Operator Overloading

Location& Location::operator=(Location const& location) {
  if (this != &location) {
    this->_projectRootPath = location._projectRootPath;
    this->_uri = location._uri;
    this->_rootPath = location._rootPath;
    this->_indexFile = location._indexFile;

    this->_maxBodySize = location._maxBodySize;
    this->_errorPages = location._errorPages;
    this->_allowMethods = location._allowMethods;
    this->_autoIndex = location._autoIndex;
    this->_redirectUri = location._redirectUri;

    this->_cgiFlag = location._cgiFlag;
    this->_cgiExtention = location._cgiExtention;
    this->_cgiPath = location._cgiPath;
    this->_uploadPath = location._uploadPath;

    this->_hasAllowMethodField = location._hasAllowMethodField;
    this->_hasRedirectField = location._hasRedirectField;
  }
  return *this;
}

// debug

#include <iostream>
void Location::printConfiguration() {
  std::cout << "==========================================" << std::endl;
  std::cout << "project root path: " << _projectRootPath << std::endl;
  std::cout << "URI: " << _uri << std::endl;
  std::cout << "root path: " << _rootPath << std::endl;
  std::cout << "index: " << _indexFile << std::endl;
  std::cout << "max body size: " << _maxBodySize << std::endl;

  std::cout << "error page:" << std::endl;
  std::map<int, std::string>::const_iterator errorPageIter;
  for (errorPageIter = _errorPages.begin(); errorPageIter != _errorPages.end();
       ++errorPageIter) {
    std::cout << errorPageIter->first << ": " << errorPageIter->second
              << std::endl;
  }

  std::cout << "allow method:";
  std::set<EHttpMethod>::const_iterator allowMethodIter;
  for (allowMethodIter = _allowMethods.begin();
       allowMethodIter != _allowMethods.end(); ++allowMethodIter) {
    std::cout << " " << *allowMethodIter;
  }
  std::cout << std::endl;

  std::cout << "autoindex: " << (_autoIndex ? "on" : "off") << std::endl;
  std::cout << "redirect URI: " << _redirectUri << std::endl;

  std::cout << "CGI flag: " << (_cgiFlag ? "on" : "off") << std::endl;
  if (_cgiFlag) {
    std::cout << "CGI extension: " << _cgiExtention << std::endl;
    std::cout << "CGI path: " << _cgiPath << std::endl;
    std::cout << "upload path: " << _uploadPath << std::endl;
  }
}

// Public Method - setter

void Location::setUri(std::string const& uri) { _uri = uri; }

void Location::setRootPath(std::string const& rootPath) {
  if (Util::isValidPath(rootPath) == false) {
    throw std::runtime_error("[] Location: setRootPath - invalid path");
  }

  _rootPath = convertPath(rootPath);
}
void Location::setIndexFile(std::string const& indexFile) {
  if (Util::isValidPath(indexFile) == false) {
    throw std::runtime_error("[] Location: indexFile - invalid path");
  }

  _indexFile = convertPath(indexFile);
}

// - Location 블럭의 max body size 설정
// - 호출하지 않을 시 기본값(DEFAULT_MAX_BODY_SIZE) 적용
// - size 범위 초과시 예외 발생
void Location::setMaxBodySize(int size) {
  if (size < Config::MIN_LIMIT_BODY_SIZE or
      size > Config::MAX_LIMIT_BODY_SIZE) {
    throw std::runtime_error("[1100] Location: setMaxBodySize - range error");
  }
  _maxBodySize = size;
}

// - Location 블럭에 특정 상태코드에 대한 error page 경로 추가
// - 이미 있는 상태코드를 다시 추가할 경우 예외 발생
void Location::addErrorPage(int statusCode, std::string const& path) {
  if (_errorPages.find(statusCode) != _errorPages.end()) {
    throw std::runtime_error(
        "[1101] Location: addErrorPage - duplicate status code");
  }

  if (Util::isValidPath(path) == false) {
    throw std::runtime_error("[] Location: addErrorPage - invalid path");
  }

  std::string const convertedPath = convertPath(path);
  _errorPages[statusCode] = getFullPath(convertedPath);
}

// Location 블럭에 허용할 메서드 추가
// - 호출하지 않을 시 모든 메서드 허용
// - 이미 있는 메서드를 다시 추가할 경우 예외 발생
// - 한번이라도 이 메서드가 호출된 경우 차단된 메서드가 있다고 판단
void Location::addAllowMethod(std::string methodString) {
  Util::toUpperCase(methodString);
  EHttpMethod method = Util::matchEHttpMethod(methodString);

  _hasAllowMethodField = true;
  if (_allowMethods.find(method) != _allowMethods.end()) {
    throw std::runtime_error(
        "[1102] Location: addAllowMethod - duplicate method");
  }
  _allowMethods.insert(method);
}

// - Location 블럭의 autoindex 설정
// - 호출하지 않을 시 기본값(false) 적용
void Location::setAutoIndex(std::string const& setting) {
  if (setting != "on" and setting != "off") {
    throw std::runtime_error("[] Location: setAutoIndex - invalid setting");
  }

  _autoIndex = setting == "on" ? true : false;
}

// Location 블럭에 redirect할 uri 설정
// - 호출하지 않을 시 redirect 블럭이 아님
// - redirect uri 설정 시 해당 location 블럭은 무조건 redirect 됨
// - 한번이라도 이 메서드가 호출된 경우 redirect 블럭으로 판단
void Location::setRedirectUri(std::string const& path) {
  if (Util::isValidPath(path) == false) {
    throw std::runtime_error("[] Location: setRedirectUri - invalid path");
  }

  _hasRedirectField = true;
  _redirectUri = convertPath(path);
}

// CGI extention 설정
// - 한 번이라도 메서드가 호출된 경우 cgi를 가지고 있다고 판단
void Location::setCgiExtention(std::string const& extention) {
  _cgiExtention = extention;
  _cgiFlag = true;
}

// CGI path 설정
// - 한 번이라도 메서드가 호출된 경우 cgi를 가지고 있다고 판단
void Location::setCgiPath(std::string const& cgiPath) {
  if (Util::isValidPath(cgiPath) == false) {
    throw std::runtime_error("[] Location: setCgiPath - invalid path");
  }

  std::string const convertedPath = convertPath(cgiPath);
  _cgiPath = getFullPath(convertedPath);
  _cgiFlag = true;
}

// 파일 upload 디렉토리 설정
// - 한 번이라도 메서드가 호출된 경우 cgi를 가지고 있다고 판단
void Location::setUploadDir(std::string const& dirPath) {
  if (Util::isValidPath(dirPath) == false) {
    throw std::runtime_error("[] Location: setUploadDir - invalid path");
  }

  std::string const convertedPath = convertPath(dirPath);
  _uploadPath = getFullPath(convertedPath);
  _cgiFlag = true;
}

// Public Method - getter

std::string const& Location::getProjectRootPath(void) const {
  return _projectRootPath;
}

std::string const& Location::getUri(void) const { return _uri; }

std::string const& Location::getRootPath(void) const { return _rootPath; }

std::string const& Location::getIndexFile(void) const { return _indexFile; }

int Location::getMaxBodySize(void) const { return _maxBodySize; }

// - 해당 상태코드에 대한 에러 페이지 경로가 존재하는지 여부 반환
bool Location::hasErrorPage(int statusCode) const {
  if (_errorPages.find(statusCode) == _errorPages.end()) {
    return false;
  }
  return true;
}

// - 해당 상태코드에 대한 에러 페이지 경로 반환
// - 에러 페이지 경로가 존재하지 않을 경우 예외 발생
std::string const& Location::getErrorPagePath(int statusCode) const {
  if (_errorPages.find(statusCode) == _errorPages.end()) {
    throw std::runtime_error(
        "[1103] Location: getErrorPagePath - no path to status code");
  }
  std::map<int, std::string>::const_iterator it = _errorPages.find(statusCode);
  return it->second;
}

// - 해당 메서드가 허용되는지 여부 반환
bool Location::isAllowMethod(EHttpMethod method) const {
  if (_hasAllowMethodField == false) {
    return true;
  }
  if (_allowMethods.find(method) == _allowMethods.end()) {
    return false;
  }
  return true;
}

bool Location::isAutoIndex(void) const { return _autoIndex; }

// - 해당 블럭이 redirect 필드를 가지고 있는지 여부 반환
bool Location::isRedirectBlock(void) const { return _hasRedirectField; }

// - 해당 블럭의 redirect uri 반환
// - 해당 블릭이 redirect 블럭이 아닌 경우 예외 발생
std::string const& Location::getRedirectUri(void) const {
  if (_hasRedirectField == false) {
    throw std::runtime_error(
        "[1104] Location: getRedirectUri - doesn't have a uri");
  }
  return _redirectUri;
}

// CGI 정보를 가지고 있는지 여부 반환
bool Location::hasCgiInfo(void) const { return _cgiFlag; }

// CGI extention 반환
// - CGI 정보가 설정되어 있지 않은데 호출된 경우 예외 발생
std::string const& Location::getCgiExtention(void) const {
  if (_cgiFlag == false) {
    throw std::runtime_error("[1105] Location: getCgiExtention - no cgi info");
  }
  return _cgiExtention;
}

// CGI 경로 반환
// - CGI 정보가 설정되어 있지 않은데 호출된 경우 예외 발생
std::string const& Location::getCgiPath(void) const {
  if (_cgiFlag == false) {
    throw std::runtime_error("[1106] Location: getCgiPath - no cgi info");
  }
  return _cgiPath;
}

// 파일 업로드 디렉토리 경로 반환
// - CGI 정보가 설정되어 있지 않은데 호출된 경우 예외 발생
std::string const& Location::getUploadDirPath(void) const {
  if (_cgiFlag == false) {
    throw std::runtime_error("[1107] Location: getUploadDirPath - no cgi info");
  }

  return _uploadPath;
}

bool Location::isRequiredValuesSet(void) const {
  return (_rootPath.size() != 0 and _indexFile.size() != 0);
}

std::string Location::getFullPath(std::string const& path) {
  std::string projectRootPath = _projectRootPath;
  if (path.front() != '/' and projectRootPath.back() != '/')
    projectRootPath.push_back('/');
  else if (path.front() == '/' and projectRootPath.back() == '/') {
    projectRootPath.pop_back();
  }

  return projectRootPath + path;
}

std::string Location::convertPath(std::string const& path) {
  return Util::removeDotSegments(Util::pctDecode(path));
}