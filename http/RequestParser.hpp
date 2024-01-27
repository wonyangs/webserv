#ifndef __REQUEST_PARSER_HPP__
#define __REQUEST_PARSER_HPP__

#include <algorithm>
#include <cctype>
#include <cstring>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "../utils/Config.hpp"
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
  std::vector<octet_t> _requestLine;
  std::vector<octet_t> _header;
  std::vector<octet_t> _body;

  std::vector<octet_t> _storageBuffer;
  std::vector<octet_t> _chunkSizeBuffer;

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

  void parse(octet_t const* buffer, ssize_t bytesRead);
  void clear();

  bool isStorageBufferNotEmpty(void);

 private:
  void setBodyLength(size_t bodyLength);
  void setBodyLength(std::string const& bodyLengthString);
  void setChunkSize(std::string const& chunkSizeString);
  void setStorageBuffer(size_t startIdx, octet_t const* buffer,
                        ssize_t bytesRead);

  void parseOctet(octet_t const& octet);
  void parseRequestLine(octet_t const& octet);
  void parseHeaderField(octet_t const& octet);
  void parseBodyContentLength(octet_t const& octet);
  void parseBodyChunked(octet_t const& octet);
  void parseBodyChunkSize(octet_t const& octet);
  void parseBodyChunkData(octet_t const& octet);
  void parseBodyChunkTrailer(octet_t const& octet);

  void setupBodyParse(void);

  std::vector<std::string> processRequestLine(void);
  std::vector<std::string> processHeaderField(void);
  std::string processBody(void);
  void processBodyChunkSize(void);

  void splitRequestLine(std::vector<std::string>& result);
  void splitHeaderField(std::vector<std::string>& result);
  void splitBodyChunkSize(std::vector<std::string>& result);

  bool isBodyChunk(void);
  EParsingStatus checkBodyParsingStatus(void);
  bool isInvalidFormatSize(std::vector<std::string> const& result, size_t size);
  bool isEndWithCRLF(std::vector<octet_t> const& vec);
  void removeCRLF(std::vector<octet_t>& vec);
  void toLowerCase(std::string& str);
  std::string trim(std::string const& str);
  bool isWhitespace(int c);
};

#endif
