#include "AResponseBuilder.hpp"

/**
 * Constructor & Destructor
 */

AResponseBuilder::AResponseBuilder(EBuilderType const& type) : _type(type) {}

AResponseBuilder::AResponseBuilder(AResponseBuilder const& builder)
    : _type(builder._type) {}

AResponseBuilder::~AResponseBuilder(void) {}

/**
 * Operator Overloading
 */

AResponseBuilder& AResponseBuilder::operator=(AResponseBuilder const& builder) {
  _type = builder._type;
  _response = builder._response;
}

/**
 * Public method
 */

EBuilderType const& AResponseBuilder::getType(void) const { return _type; }

Response const& AResponseBuilder::getResponse(void) const { return _response; }
