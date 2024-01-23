#ifndef __REQUEST_PARSER_HPP__
#define __REQUEST_PARSER_HPP__

#include <algorithm>
#include <cctype>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "../utils/StatusException.hpp"
#include "Request.hpp"

#define HTAB 9
#define SP 32
#define COLON 58

enum EParsingStatus {
  READY = 0,
  REQUEST_LINE = 1,
  HEADER_FIELD = 2,
  HEADER_FIELD_END = 3,
  BODY_CONTENT_LENGTH = 4,
  BODY_CHUNKED = 5,
  BODY_CHUNK_SIZE = 15,
  BODY_CHUNK_DATA = 25,
  BODY_CHUNK_TRAILER = 35,
  DONE = 6,
};

// RequestParser 클래스
// - HTTP Request를 파싱해서 Request 객체에 저장
class RequestParser {
 private:
  Request _request;

  enum EParsingStatus _status;
  std::vector<u_int8_t> _requestLine;
  std::vector<u_int8_t> _header;
  std::vector<u_int8_t> _body;

  std::vector<u_int8_t> _storageBuffer;
  std::vector<u_int8_t> _chunkSizeBuffer;

  size_t _chunkSize;
  size_t _bodyLength;

 public:
  RequestParser(void);
  RequestParser(RequestParser const& requestParser);
  ~RequestParser(void);

  RequestParser& operator=(RequestParser const& requestParser);

  enum EParsingStatus getParsingStatus(void) const;
  Request const& getRequest(void) const;

  void initRequestLocationAndFullPath(Location const& location);

  void parse(u_int8_t const* buffer, ssize_t bytesRead);
  void clear();

  bool isStorageBufferNotEmpty(void);

 private:
  void setBodyLength(std::string const& bodyLengthString);
  void setChunkSize(std::string const& chunkSizeString);
  void setStorageBuffer(size_t startIdx, u_int8_t const* buffer,
                        ssize_t bytesRead);

  void parseOctet(u_int8_t const& octet);
  void parseRequestLine(u_int8_t const& octet);
  void parseHeaderField(u_int8_t const& octet);
  void parseBodyContentLength(u_int8_t const& octet);
  void parseBodyChunked(u_int8_t const& octet);
  void parseBodyChunkSize(u_int8_t const& octet);
  void parseBodyChunkData(u_int8_t const& octet);
  void parseBodyChunkTrailer(u_int8_t const& octet);

  void setupBodyParse(void);

  std::vector<std::string> processRequestLine(void);
  std::vector<std::string> processHeaderField(void);
  std::string processBody(void);
  void processBodyChunkSize(void);

  void splitRequestLine(std::vector<std::string>& result);
  void splitHeaderField(std::vector<std::string>& result);
  void splitBodyChunkSize(std::vector<std::string>& result);

  EParsingStatus checkBodyParsingStatus(void);
  bool isInvalidFormatSize(std::vector<std::string> const& result, size_t size);
  bool isEndWithCRLF(std::vector<u_int8_t> const& vec);
  void removeCRLF(std::vector<u_int8_t>& vec);
  void toLowerCase(std::string& str);
  std::string trim(std::string const& str);
  bool isWhitespace(int c);
};

#endif
