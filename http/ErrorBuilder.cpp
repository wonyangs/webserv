#include "ErrorBuilder.hpp"

/**
 * Constructor & Destructor
 */

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
    setType(builder.getType());
    _response = builder._response;
    _statusCode = builder._statusCode;
  }
  return *this;
}

/**
 * Public method
 */

void ErrorBuilder::build(void) {
  Location const& location = _request.getLocation();

  if (location.hasErrorPage(_statusCode)) {
    readStatusCodeFile();
  } else {
    generateDefaultPage();
  }
}

void ErrorBuilder::close(void) {}

/**
 * Public method
 */

void ErrorBuilder::readStatusCodeFile(void) {
  // open file
}

void ErrorBuilder::generateDefaultPage(void) {}
