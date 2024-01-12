#ifndef __ENUM_HPP__
#define __ENUM_HPP__

enum EHttpMethod { HTTP_GET, HTTP_POST, HTTP_DELETE };

enum EStatusCode {
  HTTP_OK = 200,
  HTTP_CREATED = 201,
  HTTP_ACCEPTED = 202,
  HTTP_NO_CONTENT = 204,
  HTTP_PARTIAL_CONTENT = 206,

  HTTP_SPECIAL_RESPONSE = 300,
  HTTP_MOVED_PERMANENTLY = 301,
  HTTP_MOVED_TEMPORARILY = 302,
  HTTP_SEE_OTHER = 303,
  HTTP_NOT_MODIFIED = 304,
  HTTP_TEMPORARY_REDIRECT = 307,
  HTTP_PERMANENT_REDIRECT = 308,

  HTTP_BAD_REQUEST = 400,
  HTTP_UNAUTHORIZED = 401,
  HTTP_FORBIDDEN = 403,
  HTTP_NOT_FOUND = 404,
  HTTP_NOT_ALLOWED = 405,
  HTTP_REQUEST_TIME_OUT = 408,
  HTTP_CONFLICT = 409,
  HTTP_LENGTH_REQUIRED = 411,
  HTTP_PRECONDITION_FAILED = 412,
  HTTP_REQUEST_ENTITY_TOO_LARGE = 413,
  HTTP_REQUEST_URI_TOO_LARGE = 414,
  HTTP_UNSUPPORTED_MEDIA_TYPE = 415,
  HTTP_RANGE_NOT_SATISFIABLE = 416,
  HTTP_MISDIRECTED_REQUEST = 421,
  HTTP_TOO_MANY_REQUESTS = 429,

  HTTP_INTERNAL_SERVER_ERROR = 500,
  HTTP_NOT_IMPLEMENTED = 501,
  HTTP_BAD_GATEWAY = 502,
  HTTP_SERVICE_UNAVAILABLE = 503,
  HTTP_GATEWAY_TIME_OUT = 504,
  HTTP_VERSION_NOT_SUPPORTED = 505,
  HTTP_INSUFFICIENT_STORAGE = 507
};

#endif