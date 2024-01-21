#include "ErrorBuilder.hpp"

/**
 * Constructor & Destructor
 */

ErrorBuilder::ErrorBuilder(void) : AResponseBuilder(), _statusCode(500) {}

ErrorBuilder::ErrorBuilder(Request const& request, int statusCode)
    : AResponseBuilder(ERROR, request), _statusCode(statusCode) {}

ErrorBuilder::ErrorBuilder(ErrorBuilder const& builder)
    : AResponseBuilder(builder) {
  _statusCode = builder._statusCode;
}

ErrorBuilder::~ErrorBuilder(void) {}

/**
 * Operator Overloading
 */

ErrorBuilder& ErrorBuilder::operator=(ErrorBuilder const& builder) {
  if (this != &builder) {
    _response = builder._response;
    _isDone = builder._isDone;
    setRequest(builder.getRequest());
    setType(builder.getType());
    _statusCode = builder._statusCode;
  }
  return *this;
}

/**
 * Public method
 */

#include <iostream>

void ErrorBuilder::build(void) {
  std::cout << "error builder!" << std::endl;
  generateDefaultPage();

  // Location const& location = _request.getLocation();
  // if (location.hasErrorPage(_statusCode)) {
  //   readStatusCodeFile();
  // } else {
  //   generateDefaultPage();
  // }
}

void ErrorBuilder::close(void) {}

/**
 * Public method
 */

void ErrorBuilder::readStatusCodeFile(void) {
  // open file
}

// Connection은 request Header 정보 보고 변경되어야 함
void ErrorBuilder::generateDefaultPage(void) {
  // HTTP 응답 생성

  std::string const& body = Config::defaultErrorPageBody(_statusCode);

  _response.setHttpVersion("HTTP/1.1");
  _response.setStatusCode(_statusCode);

  _response.addHeader("Content-Type", "text/html");
  _response.addHeader("Content-Length", Util::itos(body.size()));
  _response.addHeader("Connection", "keep-alive");

  _response.appendBody(body);

  _response.makeResponseContent();

  _isDone = true;
}