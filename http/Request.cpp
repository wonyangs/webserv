#include "Request.hpp"

// Constructor & Destructor

Request::Request(void) : _method(HTTP_NONE), _locationFlag(false) {}

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

  // std::cout << "[Header]" << std::endl;
  // for (std::map<std::string, std::string>::const_iterator it =
  // _header.begin();
  //      it != _header.end(); ++it) {
  //   std::cout << "[" << it->first << "]: " << it->second << std::endl;
  // }

  // std::cout << "Body: " << _body << std::endl;
  std::cout << "Location uri: " << _location.getUri() << std::endl;
  std::cout << "fullPath: " << _fullPath << std::endl;
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

// host, connection의 field-value는 소문자로 저장되어 있음
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

// 요청에 알맞는 host 반환
// - 멤버 변수가 아님
// - host 문자열에 처음 나오는 : 을 기준으로 앞만 host로 판단
// - HTTP/1.1 요청일 때 host가 존재하지 않는 경우 예외 발생
std::string const Request::getHost(void) const {
  if (isHeaderFieldNameExists("host") == false) {
    if (_httpVersion == "HTTP/1.1") {
      throw StatusException(
          HTTP_BAD_REQUEST,
          "[2401] Request: getHost - Host header is required on HTTP/1.1");
    }
    return "";
  }

  std::string const& fieldValue = getHeaderFieldValues("host");
  size_t pos = fieldValue.find(":");

  if (pos != std::string::npos) {
    return fieldValue.substr(0, pos);
  }

  return fieldValue;
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

  setMethod(result[methodIndex]);
  storeRequestTarget(result[requestTargetIndex]);
  setHttpVersion(result[httpVersionIndex]);
}

void Request::storeRequestTarget(std::string const& requestTarget) {
  if (Config::MAX_URI_SIZE < requestTarget.size()) {
    throw StatusException(
        HTTP_REQUEST_URI_TOO_LARGE,
        "[2103] Request: storeRequestTarget - path is too long");
  }

  if (isValidRequestTarget(requestTarget) == false) {
    throw StatusException(
        HTTP_BAD_REQUEST,
        "[2104] Request: storeRequestTarget - request Target is invalid");
  }

  std::string path, query;
  splitRequestTarget(path, query, requestTarget);

  setPath(pctDecode(path));
  setQuery(pctDecode(query));
}

// Header field 저장
// - result 배열에는 fieldName과 fieldValue가 순서대로 저장되어 있다고 가정
// - Request 객체에 이미 존재하는 field-name일 경우 예외 발생
void Request::storeHeaderField(std::vector<std::string> const& result) {
  int const fieldNameIndex = 0, fieldValueIndex = 1;

  std::string const& fieldName = result[fieldNameIndex];
  std::string const& fieldValue = result[fieldValueIndex];

  if (isHeaderFieldNameExists(fieldName)) {
    throw StatusException(
        HTTP_BAD_REQUEST,
        "[2202] Request: storeHeaderField - duplicate field-name");
  }

  _header.insert(std::pair<std::string, std::string>(fieldName, fieldValue));
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
// - field-name은 소문자로 저장되어 있음
bool Request::isHeaderFieldNameExists(std::string const& fieldName) const {
  return (_header.find(fieldName) != _header.end());
}

bool Request::isConnectionClose(void) const {
  if (_httpVersion == "HTTP/1.0") return true;

  if (isHeaderFieldNameExists("connection") and
      getHeaderFieldValues("connection") == "close")
    return true;

  return false;
}

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
}

// Private Method - setter

void Request::setMethod(std::string const& method) {
  _method = matchEHttpMethod(method);
}

void Request::setPath(std::string const& path) { _path = path; }

void Request::setQuery(std::string const& query) { _query = query; }

void Request::setHttpVersion(std::string const& httpVersion) {
  if (isValidHTTPVersionFormat(httpVersion) == false) {
    throw StatusException(
        HTTP_BAD_REQUEST,
        "[2006] Request: setHttpVersion - invalid format: " + httpVersion);
  }

  if (httpVersion != "HTTP/1.0" and httpVersion != "HTTP/1.1") {
    throw StatusException(
        HTTP_VERSION_NOT_SUPPORTED,
        "[2007] Request: setHttpVersion - version not supported: " +
            httpVersion);
  }

  _httpVersion = httpVersion;
}

void Request::setFullPath(std::string const& fullPath) { _fullPath = fullPath; }

// Private Method

// method가 매칭되는 enum EHttpMethod를 반환
// - 매칭되는 EHttpMethod가 없는 경우 예외 발생
EHttpMethod Request::matchEHttpMethod(std::string method) {
  if (method == "GET") return HTTP_GET;
  if (method == "POST") return HTTP_POST;
  if (method == "DELETE") return HTTP_DELETE;
  throw StatusException(HTTP_NOT_IMPLEMENTED,
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

bool Request::isHex(char ch) {
  return ('0' <= ch and ch <= '9') or ('a' <= ch and ch <= 'f') or
         ('A' <= ch and ch <= 'F');
}

bool Request::isValidRequestTarget(std::string const& requestTarget) {
  size_t size = requestTarget.size();
  if (requestTarget.size() < 1 or requestTarget.front() != '/') return false;

  std::string const& others = "-._~!$&'()*+,;=:@/?";
  for (size_t i = 0; i < size; i++) {
    char ch = requestTarget[i];

    if (std::isalpha(ch) or std::isdigit(ch) or
        others.find(ch) != std::string::npos)
      continue;

    if (ch == '%' and i + 2 < size) {
      if (isHex(requestTarget[i + 1]) and isHex(requestTarget[i + 2])) continue;
    }

    return false;
  }
  return true;
}

bool Request::isValidHTTPVersionFormat(std::string const& httpVersion) {
  if (httpVersion.size() != 8) return false;
  if (httpVersion.substr(0, 5) != "HTTP/") return false;
  if (isdigit(httpVersion[5]) == false or httpVersion[6] != '.' or
      isdigit(httpVersion[7]) == false)
    return false;

  return true;
}

// 인자로 받은 16진수를 문자(char)로 변경하여 반환
char Request::hexToChar(std::string const& hexStr) {
  int n;

  std::stringstream ss;
  ss << std::hex << hexStr;
  ss >> n;
  return static_cast<char>(n);
}

// percent-encoding을 디코딩하여 반환
std::string Request::pctDecode(std::string const& str) {
  std::stringstream ss;

  for (size_t i = 0; i < str.size(); i++) {
    if (str[i] == '%' and i + 2 < str.size()) {
      ss << hexToChar(str.substr(i + 1, 2));
      i += 2;
    } else {
      ss << str[i];
    }
  }
  return ss.str();
}