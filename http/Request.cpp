#include "Request.hpp"

#include "Config.hpp"

// Constructor & Destructor

Request::Request(void) : _method(HTTP_NONE) {}

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
  }
  return *this;
}

// Public Method - getter

enum EHttpMethod const& Request::getMethod(void) const { return _method; }

std::string const& Request::getPath(void) const { return _path; }

std::string const& Request::getQuery(void) const { return _query; }

std::string const& Request::getHttpVersion(void) const { return _httpVersion; }

std::map<std::string, std::vector<std::string> > const& Request::getHeader(
    void) const {
  return _header;
}

std::string const& Request::getBody(void) const { return _body; }

std::vector<std::string> const& Request::getHeaderFieldValues(
    std::string const& fieldName) const {
  if (isHeaderFieldNameExists(fieldName) == false) {
    throw std::runtime_error(
        "[2203] Request: getHeaderFieldValues -  field-name does not exist");
  }

  std::map<std::string, std::vector<std::string> >::const_iterator it =
      _header.find(fieldName);
  return it->second;
}

// debug

#include <iostream>

// 디버깅용 Request 정보 출력 함수
void Request::print() const {
  std::cout << "Method: " << _method << std::endl;
  std::cout << "Path: " << _path << std::endl;
  std::cout << "Query: " << _query << std::endl;
  std::cout << "HTTP Version: " << _httpVersion << std::endl;

  std::cout << "Headers:" << std::endl;
  for (std::map<std::string, std::vector<std::string> >::const_iterator it =
           _header.begin();
       it != _header.end(); ++it) {
    std::cout << "header-field: [" << it->first << "] ";
    for (std::vector<std::string>::const_iterator vit = it->second.begin();
         vit != it->second.end(); ++vit) {
      std::cout << "[" << *vit << "]" << std::endl;
    }
  }

  std::cout << "Body: " << _body << std::endl;
}

// Public Method

// Request Line에 해당하는 method, path, http version 저장
// - result 배열에는 method, path, httpVersion이 순서대로 저장되어 있다고 가정
void Request::storeRequestLine(std::vector<std::string> const& result) {
  int const methodIndex = 0, pathIndex = 1, httpVersionIndex = 2;

  _method = matchEHttpMethod(result[methodIndex]);
  if (Config::MAX_URI_SIZE < result[pathIndex].size()) {
    throw StatusException(
        HTTP_REQUEST_URI_TOO_LARGE,
        "[2103] Request: storeRequestLine - path is too long");
  }
  _path = result[pathIndex];
  _httpVersion = result[httpVersionIndex];
}

// Header field 저장
// - result 배열에는 fieldName과 fieldValue가 순서대로 저장되어 있다고 가정
void Request::storeHeaderField(std::vector<std::string> const& result) {
  int const fieldNameIndex = 0, fieldValueIndex = 1;

  std::string fieldName = result[fieldNameIndex];
  std::vector<std::string> fieldValue;
  fieldValue.push_back(result[fieldValueIndex]);
  // - TODO:: , 로 구분될 수 있는 값 처리 필요
  _header.insert(
      std::pair<std::string, std::vector<std::string> >(fieldName, fieldValue));
}

// body 저장
void Request::storeBody(std::string const& result) { _body = result; }

// 해당 헤더 field-name의 존재를 확인하는 함수
bool Request::isHeaderFieldNameExists(std::string const& fieldName) const {
  return (_header.find(fieldName) != _header.end());
}

// 해당 헤더 field-value의 존재를 확인하는 함수
// - 해당 헤더 field-name이 없다면 false 반환
bool Request::isHeaderFieldValueExists(std::string const& fieldName,
                                       std::string const& fieldValue) {
  if (isHeaderFieldNameExists(fieldName) == false) return false;

  std::vector<std::string> const& fieldValues = _header[fieldName];
  std::vector<std::string>::const_iterator it =
      std::find(fieldValues.begin(), fieldValues.end(), fieldValue);
  return (it != fieldValues.end());
}

// 멤버 변수를 비어있는 상태로 초기화
void Request::clear() {
  _method = HTTP_NONE;
  _path.clear();
  _query.clear();
  _httpVersion.clear();
  _header.clear();
  _body.clear();
}

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
