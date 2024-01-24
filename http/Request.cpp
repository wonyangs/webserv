#include "Request.hpp"

// Constructor & Destructor

Request::Request(void)
    : _method(HTTP_NONE), _locationFlag(false), _isConnectionClose(false) {}

Request::Request(Request const& request) { *this = request; }

Request::~Request(void) {}

// Operator Overloading

Request& Request::operator=(Request const& request) {
  if (this != &request) {
    _method = request._method;
    _path = request._path;
    _query = request._query;
    _httpVersion = request._httpVersion;
    _header = request._header;
    _body = request._body;

    _location = request._location;
    _locationFlag = request._locationFlag;
    _fullPath = request._fullPath;
    _isConnectionClose = request._isConnectionClose;
  }
  return *this;
}

// debug

#include <iostream>

// 디버깅용 Request 정보 출력 함수
void Request::print() const {
  std::cout << "Method: " << _method << std::endl;
  std::cout << "Path: " << _path << std::endl;
  std::cout << "Query: " << _query << std::endl;
  std::cout << "HTTP Version: " << _httpVersion << std::endl;

  std::cout << "[Header]" << std::endl;
  for (std::map<std::string, std::string>::const_iterator it = _header.begin();
       it != _header.end(); ++it) {
    std::cout << "[" << it->first << "]: " << it->second << std::endl;
  }

  std::cout << "Body: " << _body << std::endl;
  std::cout << "Location uri: " << _location.getUri() << std::endl;
  std::cout << "fullPath: " << _fullPath << std::endl;
  std::cout << "isConnectionClose: " << _isConnectionClose << std::endl;
}

// Public Method - getter

enum EHttpMethod const& Request::getMethod(void) const { return _method; }

std::string const& Request::getPath(void) const { return _path; }

std::string const& Request::getQuery(void) const { return _query; }

std::string const& Request::getHttpVersion(void) const { return _httpVersion; }

std::map<std::string, std::string> const& Request::getHeader(void) const {
  return _header;
}

std::string const& Request::getBody(void) const { return _body; }

Location const& Request::getLocation(void) const {
  if (_locationFlag == false) {
    throw std::runtime_error(
        "[2400] Request: getLocation - location flag is false");
  }
  return _location;
}

bool Request::getLocationFlag(void) const { return _locationFlag; }

std::string const& Request::getFullPath(void) const { return _fullPath; }

std::string const& Request::getHeaderFieldValues(
    std::string const& fieldName) const {
  if (isHeaderFieldNameExists(fieldName) == false) {
    throw std::runtime_error(
        "[2203] Request: getHeaderFieldValues - field-name does not exist");
  }

  std::map<std::string, std::string>::const_iterator it =
      _header.find(fieldName);
  return it->second;
}

// Public Method - setter

void Request::setLocation(Location const& location) {
  _location = location;
  _locationFlag = true;
}

// Public Method

std::string const Request::generateIndexPath(void) const {
  Location const& location = getLocation();
  std::string const& indexFile = location.getIndexFile();
  std::string fullPath = getFullPath();

  if (indexFile.front() == '/') fullPath.pop_back();
  fullPath += indexFile;

  return fullPath;
}

// Request Line에 해당하는 method, request-target, http version 저장
// - result 배열에는 method, request-target, httpVersion이 순서대로 저장되어
// 있다고 가정
// request-target의 size가 MAX_URI_SIZE를 초과할 경우 예외 발생
void Request::storeRequestLine(std::vector<std::string> const& result) {
  int const methodIndex = 0, requestTargetIndex = 1, httpVersionIndex = 2;

  if (Config::MAX_URI_SIZE < result[requestTargetIndex].size()) {
    throw StatusException(
        HTTP_REQUEST_URI_TOO_LARGE,
        "[2103] Request: storeRequestLine - path is too long");
  }

  std::string path, query;
  splitRequestTarget(path, query, result[requestTargetIndex]);

  setMethod(result[methodIndex]);
  setPath(path);
  setQuery(query);
  setHttpVersion(result[httpVersionIndex]);
}

// Header field 저장
// - result 배열에는 fieldName과 fieldValue가 순서대로 저장되어 있다고 가정
void Request::storeHeaderField(std::vector<std::string> const& result) {
  int const fieldNameIndex = 0, fieldValueIndex = 1;

  std::string const& fieldName = result[fieldNameIndex];
  std::string const& fieldValue = result[fieldValueIndex];
  _header.insert(std::pair<std::string, std::string>(fieldName, fieldValue));

  if (fieldName == "connection" and fieldValue == "close")
    _isConnectionClose = true;
}

// body 저장
void Request::storeBody(std::string const& result) { _body = result; }

// fullPath 저장
void Request::storeFullPath(void) {
  Location const& location = getLocation();
  std::string root = location.getRootPath();
  std::string locationUri = location.getUri();

  // path의 맨 처음이 무조건 /로 시작해 /를 제거
  if (root.back() == '/') root.pop_back();
  if (locationUri.back() == '/') locationUri.pop_back();

  size_t pos = _path.find(locationUri);

  if (pos != std::string::npos and pos == 0) {
    setFullPath(root + _path.substr(locationUri.length()));
  } else {
    setFullPath(root + _path);
  }
}

// 해당 헤더 field-name의 존재를 확인하는 함수
bool Request::isHeaderFieldNameExists(std::string const& fieldName) const {
  return (_header.find(fieldName) != _header.end());
}

bool Request::isConnectionClose(void) const { return _isConnectionClose; }

// 멤버 변수를 비어있는 상태로 초기화
void Request::clear() {
  _method = HTTP_NONE;
  _path.clear();
  _query.clear();
  _httpVersion.clear();
  _header.clear();
  _body.clear();
  _locationFlag = false;
  _fullPath.clear();
  _isConnectionClose = false;
}

// Private Method - setter

void Request::setMethod(std::string const& method) {
  _method = matchEHttpMethod(method);
}

void Request::setPath(std::string const& path) { _path = path; }

void Request::setQuery(std::string const& query) { _query = query; }

void Request::setHttpVersion(std::string const& httpVersion) {
  _httpVersion = httpVersion;
  if (_httpVersion == "HTTP/1.0") _isConnectionClose = true;
}

void Request::setFullPath(std::string const& fullPath) { _fullPath = fullPath; }

// Private Method

// method가 매칭되는 enum EHttpMethod를 반환
// - 매칭되는 EHttpMethod가 없는 경우 예외 발생
EHttpMethod Request::matchEHttpMethod(std::string method) {
  if (method == "GET") return HTTP_GET;
  if (method == "POST") return HTTP_POST;
  if (method == "DELETE") return HTTP_DELETE;
  throw StatusException(HTTP_NOT_ALLOWED,
                        "[2101] Request: matchEHttpMethod - invalid method");
}

// request-target을 ?을 기준으로 split
// - 가장 처음 나오는 ? 을 기준으로 앞을 path, 뒤를 query에 저장
void Request::splitRequestTarget(std::string& path, std::string& query,
                                 const std::string& requestTarget) {
  size_t found = requestTarget.find('?');

  if (found != std::string::npos) {
    path = requestTarget.substr(0, found);
    query = requestTarget.substr(found + 1);
    return;
  }

  path = requestTarget;
  query = "";
}