#ifndef __REQUEST_PARSER_HPP__
#define __REQUEST_PARSER_HPP__

#include <algorithm>
#include <cctype>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "Request.hpp"
#include "../utils/StatusException.hpp"

#define HTAB 9
#define SP 32
#define COLON 58

// TODO: chunk 처리
enum EParsingStatus {
  READY,
  REQUEST_LINE,
  HEADER_FIELD,
  BODY,
  DONE,
};

// RequestParser 클래스
// - HTTP Request를 파싱해서 Request 객체에 저장
class RequestParser {
 private:
  enum EBodyType {
    BODY_NONE,
    BODY_CHUNKED,
    BODY_CONTENT_LENGTH,
  };

  enum EParsingStatus _status;
  std::vector<u_int8_t> _requestLine;
  std::vector<u_int8_t> _header;
  std::vector<u_int8_t> _body;

  std::vector<u_int8_t> _buffer;

  Request& _request;

  enum EBodyType _bodyType;
  size_t _bodyLength;

 public:
  RequestParser(Request& request);
  RequestParser(RequestParser const& request);
  ~RequestParser(void);

  RequestParser& operator=(RequestParser const& request);

  enum EParsingStatus getParsingStatus();

  void parse(u_int8_t* buffer, ssize_t bytesRead);
  void clear();

 private:
  RequestParser(void);

  void setBodyLength(std::string const& bodyLengthString);

  void parseRequestLine(u_int8_t const& ch);
  void parseHeaderField(u_int8_t const& ch);
  void parseBody(u_int8_t const& ch);

  std::vector<std::string> processRequestLine(void);
  std::vector<std::string> processHeaderField(void);
  std::string processBody(void);

  void splitRequestLine(std::vector<std::string>& result);
  void splitHeaderField(std::vector<std::string>& result);

  enum EBodyType checkBodyType(void);
  bool isInvalidFormatSize(std::vector<std::string> const& result, size_t size);
  bool isEndWithCRLF(std::vector<u_int8_t> const& vec);
  void removeCRLF(std::vector<u_int8_t>& vec);
  void toLowerCase(std::string& str);
  std::string trim(std::string const& str);
  bool isWhitespace(int c);
};

#endif
