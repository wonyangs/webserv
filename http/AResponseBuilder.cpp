#include "AResponseBuilder.hpp"

/**
 * Constructor & Destructor
 */

AResponseBuilder::AResponseBuilder(EBuilderType const& type,
                                   Request const& request)
    : _isDone(false), _request(request), _type(type) {}

AResponseBuilder::AResponseBuilder(AResponseBuilder const& builder)
    : _response(builder._response),
      _isDone(builder._isDone),
      _request(builder._request),
      _type(builder._type) {}

AResponseBuilder::~AResponseBuilder(void) {}

/**
 * Operator Overloading
 */

AResponseBuilder& AResponseBuilder::operator=(AResponseBuilder const& builder) {
  if (this != &builder) {
    _response = builder._response;
    _isDone = builder._isDone;
    _request = builder._request;
    _type = builder._type;
  }
  return *this;
}

/**
 * Public method
 */

AResponseBuilder::EBuilderType const& AResponseBuilder::getType(void) const {
  return _type;
}

Response const& AResponseBuilder::getResponse(void) const { return _response; }

bool AResponseBuilder::isDone(void) const { return _isDone; }

/**
 * Protected method - setter
 */

Request const& AResponseBuilder::getRequest(void) const { return _request; }

void AResponseBuilder::setRequest(Request const& request) {
  _request = request;
}

void AResponseBuilder::setType(EBuilderType const& type) { _type = type; }
