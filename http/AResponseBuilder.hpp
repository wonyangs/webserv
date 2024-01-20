#ifndef __A_RESPONSE_BUILDER_HPP__
#define __A_RESPONSE_BUILDER_HPP__

#include "Request.hpp"
#include "Response.hpp"

class AResponseBuilder {
 protected:
  enum EBuilderType { ERROR };
  Request const& _request;
  Response _response;

 private:
  EBuilderType _type;

 public:
  EBuilderType const& getType(void) const;
  Response const& getResponse(void) const;

  virtual void build(void) = 0;
  virtual void close(void) = 0;

 protected:
  AResponseBuilder(EBuilderType const& type, Request const& request);
  AResponseBuilder(AResponseBuilder const& builder);
  ~AResponseBuilder(void);

  AResponseBuilder& operator=(AResponseBuilder const& builder);

  void setType(EBuilderType const& type);

 private:
  AResponseBuilder(void);
};

#endif