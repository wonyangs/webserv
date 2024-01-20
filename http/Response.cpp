#include "Response.hpp"

// Constructor & Destructor

Response::Response(void)
    : _responseContent(""),
      _startIndex(-1),
      _httpVersion(""),
      _statusCode(-1),
      _body("") {}

Response::Response(Response const& response) { *this = response; }

Response::~Response(void) {}

// Operator Overloading

Response& Response::operator=(Response const& response) {
  if (this != &response) {
    _responseContent = response._responseContent;
    _startIndex = response._startIndex;
    _httpVersion = response._httpVersion;
    _statusCode = response._statusCode;
    _header = response._header;
    _body = response._body;
  }
  return *this;
}

// debug

#include <iostream>

// 디버깅용 Response 정보 출력 함수
void Response::print() const {
  std::cout << "Response Content: " << _responseContent << std::endl;
  std::cout << "Start Index: " << _startIndex << std::endl;
  std::cout << "HTTP Version: " << _httpVersion << std::endl;
  std::cout << "StatusCode: " << _statusCode << std::endl;

  std::cout << "Headers:" << std::endl;
  for (std::map<std::string, std::string>::const_iterator it = _header.begin();
       it != _header.end(); ++it) {
    std::cout << "header-field: [" << it->first << "] ";
    std::cout << "[" << it->second << "]" << std::endl;
  }

  std::cout << "Body: " << _body << std::endl;
}

// Public Method - getter

std::string const& Response::getHttpVersion(void) const { return _httpVersion; }

int const& Response::getStatusCode(void) const { return _statusCode; }

std::map<std::string, std::string> const& Response::getHeader(void) const {
  return _header;
}

std::string const& Response::getBody(void) const { return _body; }

// Public Method - setter

void Response::setResponseContent(void) {
  std::string const crlf = "\r\n";

  std::stringstream ss;

  ss << _httpVersion << " " << _statusCode << " "
     << findStatusMessage(_statusCode) << crlf;

  for (std::map<std::string, std::string>::const_iterator it = _header.begin();
       it != _header.end(); ++it) {
    ss << it->first << ": " << it->second << crlf;
  }
  ss << crlf;
  ss << _body;

  _responseContent = ss.str();
  _startIndex = 0;
}

void Response::setHttpVersion(std::string const& httpVersion) {
  _httpVersion = httpVersion;
}

void Response::setStatusCode(int const& statusCode) {
  _statusCode = statusCode;
}

// Public Method

// _header 멤버 변수에 fieldName을 key로, fieldValue를 value로 저장
// - 만약 중복된 헤더인 경우 예외 발생
void Response::addHeader(std::string const& fieldName,
                         std::string const& fieldValue) {
  if (isHeaderFieldNameExists(fieldName)) {
    throw std::runtime_error(
        "[5000] Response: addHeader - duplicate header field-name");
  }
  _header.insert(std::pair<std::string, std::string>(fieldName, fieldValue));
}

// _body 멤버 변수에 인자를 붙여서 저장
void Response::appendBody(std::string const& body) { _body.append(body); }

// 멤버 변수를 비어있는 상태로 초기화
void Response::clear(void) {
  _responseContent.clear();
  _startIndex = -1;
  _httpVersion.clear();
  _statusCode = -1;
  _header.clear();
  _body.clear();
}

// 상태 코드에 해당하는 메시지 찾기
// - 정의되지 않은 코드일 경우 예외 발생
std::string const& Response::findStatusMessage(int code) {
  std::map<int, std::string>::const_iterator it =
      Config::statusMessages.find(code);

  if (it == Config::statusMessages.end()) {
    throw std::runtime_error(
        "[5001] Response: findStatusMessage - status message does not exist: " +
        std::to_string(code));
  }

  return it->second;
}

// Private Method

// 해당 헤더 field-name의 존재를 확인하는 함수
bool Response::isHeaderFieldNameExists(std::string const& fieldName) const {
  return (_header.find(fieldName) != _header.end());
}
