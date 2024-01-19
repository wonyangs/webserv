#include "AResponseBuilder.hpp"

/**
 * Constructor & Destructor
 */

AResponseBuilder::AResponseBuilder(EBuilderType const& type,
                                   Request const& request)
    : _request(request), _type(type) {}

AResponseBuilder::AResponseBuilder(AResponseBuilder const& builder)
    : _request(builder._request), _type(builder._type) {}

AResponseBuilder::~AResponseBuilder(void) {}

/**
 * Operator Overloading
 */

AResponseBuilder& AResponseBuilder::operator=(AResponseBuilder const& builder) {
  if (this != &builder) {
    _type = builder._type;
    _response = builder._response;
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
