#ifndef __A_RESPONSE_BUILDER_HPP__
#define __A_RESPONSE_BUILDER_HPP__

#include <string>

#include "Request.hpp"
#include "Response.hpp"

class AResponseBuilder {
 public:
  enum EBuilderType { AUTOINDEX, REDIRECT, STATIC, ERROR };

 protected:
  Response _response;
  bool _isDone;

 private:
  Request _request;
  EBuilderType _type;

 public:
  AResponseBuilder(EBuilderType const& type, Request const& request);
  AResponseBuilder(AResponseBuilder const& builder);
  virtual ~AResponseBuilder(void);

  AResponseBuilder& operator=(AResponseBuilder const& builder);

  EBuilderType const& getType(void) const;
  Response& getResponse(void);
  bool isDone(void) const;

  virtual int build(void) = 0;
  virtual void close(void) = 0;

 protected:
  Request const& getRequest(void) const;

  void setRequest(Request const& request);
  void setType(EBuilderType const& type);

  virtual void buildResponseContent(std::string const& body) = 0;

 private:
  AResponseBuilder(void);
};

#endif