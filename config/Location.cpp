#include "Location.hpp"

#include <stdexcept>

// Constructor & Destructor

Location::Location(void){};

Location::Location(std::string const& uri, std::string const& rootPath,
                   std::string const& indexFile)
    : _uri(uri),
      _rootPath(rootPath),
      _indexFile(indexFile),
      _maxBodySize(DEFAULT_MAX_BODY_SIZE),
      _autoIndex(false),
      _hasAllowMethodField(false),
      _hasRedirectField(false) {}

Location::Location(Location const& location) { *this = location; }

Location::~Location(void) {}

// Operator Overloading

Location& Location::operator=(Location const& location) {
  if (this != &location) {
    this->_uri = location._uri;
    this->_rootPath = location._rootPath;
    this->_indexFile = location._indexFile;

    this->_maxBodySize = location._maxBodySize;
    this->_errorPages = location._errorPages;
    this->_allowMethods = location._allowMethods;
    this->_autoIndex = location._autoIndex;
    this->_redirectUri = location._redirectUri;
  }
  return *this;
}

// Public Method - setter

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
  _errorPages[statusCode] = path;
}

// - Location 블럭에 허용할 메서드 추가
// - 호출하지 않을 시 모든 메서드 허용
// - 이미 있는 메서드를 다시 추가할 경우 예외 발생
void Location::addAllowMethod(EHttpMethod method) {
  // 한번이라도 이 메서드가 호출된 경우 차단된 메서드가 있다고 판단
  _hasAllowMethodField = true;
  if (_allowMethods.find(method) != _allowMethods.end()) {
    throw std::runtime_error(
        "[1102] Location: addAllowMethod - duplicate method");
  }
  _allowMethods.insert(method);
}

// - Location 블럭의 autoindex 설정
// - 호출하지 않을 시 기본값(false) 적용
void Location::setAutoIndex(bool setting) { _autoIndex = setting; }

// - Location 블럭에 redirect할 uri 설정
// - 호출하지 않을 시 redirect 블럭이 아님
// - redirect uri 설정 시 해당 location 블럭은 무조건 redirect 됨
void Location::setRedirectUri(std::string const& path) {
  // 한번이라도 이 메서드가 호출된 경우 redirect 블럭으로 판단
  _hasRedirectField = true;
  _redirectUri = path;
}

// Public Method - getter

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
