#ifndef __A_RESPONSE_BUILDER_HPP__
#define __A_RESPONSE_BUILDER_HPP__

#include "Response.hpp"

class AResponseBuilder {
 private:
  EBuilderType _type;
  Response _response;

 public:
  EBuilderType const& getType(void) const;
  Response const& getResponse(void) const;

  virtual void build(void) = 0;
  virtual void close(void) = 0;

 protected:
  enum EBuilderType = {ERROR};

  AResponseBuilder(EBuilderType const& type);
  AResponseBuilder(AResponseBuilder const& builder);
  ~AResponseBuilder(void);

  AResponseBuilder& operator=(AResponseBuilder const& builder);

 private:
  AResponseBuilder(void);
};

#endif