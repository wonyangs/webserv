#include "RequestParser.hpp"

// Constructor & Destructor

RequestParser::RequestParser(void)
    : _status(READY), _chunkSize(0), _bodyLength(0) {}

RequestParser::RequestParser(RequestParser const& requestParser)
    : _request(requestParser._request) {
  *this = requestParser;
}

RequestParser::~RequestParser(void) {}

// Operator Overloading

RequestParser& RequestParser::operator=(RequestParser const& requestParser) {
  if (this != &requestParser) {
    _request = requestParser._request;
    _status = requestParser._status;
    _requestLine = requestParser._requestLine;
    _header = requestParser._header;
    _body = requestParser._body;
    _storageBuffer = requestParser._storageBuffer;
    _chunkSizeBuffer = requestParser._chunkSizeBuffer;
    _chunkSize = requestParser._chunkSize;
    _bodyLength = requestParser._bodyLength;
  }
  return *this;
}

// Public Method - getter

enum EParsingStatus RequestParser::getParsingStatus() const { return _status; }

Request const& RequestParser::getRequest() const { return _request; }

// Public Method

void RequestParser::initRequestLocationAndFullPath(Location const& location) {
  _request.setLocation(location);
  _request.storeFullPath();
}

#include <iostream>

// buffer 값 파싱
// - 만약 _storageBuffer가 비어있지 않으면 해당 값 먼저 파싱 진행
// - 이후 buffer 값 파싱
// - 파싱 도중 HEADER_FIELD_END 또는 DONE이 되었을 경우,
//   남은 octets은 _storageBuffer에 저장
void RequestParser::parse(octet_t const* buffer, ssize_t bytesRead) {
  if (_status == HEADER_FIELD_END) {
    setupBodyParse();

    if (_status == DONE) return;
    if (_request.getMethod() != HTTP_POST) {
      throw StatusException(
          HTTP_BAD_REQUEST,
          "[2008] RequestParser: parse - only post methods can handle payload");
    }
  }

  for (size_t i = 0; i < _storageBuffer.size(); i++) {
    parseOctet(_storageBuffer[i]);

    if ((_status == HEADER_FIELD_END or _status == DONE) and
        i + 1 != _storageBuffer.size()) {
      setStorageBuffer(i + 1, buffer, bytesRead);
      return;
    }
  }

  _storageBuffer.clear();
  for (ssize_t i = 0; i < bytesRead; i++) {
    parseOctet(buffer[i]);
  }
}

// 멤버 변수를 비어있는 상태로 초기화
// - _storageBuffer 변수는 제외
void RequestParser::clear() {
  _status = READY;
  _requestLine.clear();
  _header.clear();
  _body.clear();
  _request.clear();
  _chunkSizeBuffer.clear();
  _chunkSize = 0;
  _bodyLength = 0;
}

// _storageBuffer 변수가 비어있지 않은지 여부 확인
bool RequestParser::isStorageBufferNotEmpty() {
  return (_storageBuffer.size() != 0);
}

// Private Method - setter

void RequestParser::setBodyLength(size_t bodyLength) {
  size_t maxBodySize = _request.getLocation().getMaxBodySize();
  if (maxBodySize < bodyLength) {
    throw StatusException(
        HTTP_REQUEST_ENTITY_TOO_LARGE,
        "[2009] RequestParser: setBodyLength - body content too large");
  }

  _bodyLength = bodyLength;
}

// bodyLength 저장
// - 만약 bodyLengthString 을 size_t로 변환 실패할 경우 예외 발생
// - TODO: config에 있는 client_max_body_size 예외 처리 필요
void RequestParser::setBodyLength(std::string const& bodyLengthString) {
  size_t bodyLength;

  if (bodyLengthString.size() < 1 or bodyLengthString[0] == '-') {
    throw StatusException(
        HTTP_BAD_REQUEST,
        "[2001] RequestParser: setBodyLength - invalid type: " +
            bodyLengthString);
  }

  std::stringstream ss;
  ss << bodyLengthString;
  ss >> bodyLength;

  if (ss.fail() or !ss.eof()) {
    throw StatusException(
        HTTP_BAD_REQUEST,
        "[2002] RequestParser: setBodyLength - invalid type: " +
            bodyLengthString);
  }

  setBodyLength(bodyLength);
}

void RequestParser::setChunkSize(std::string const& chunkSizeString) {
  if (chunkSizeString.size() < 1 or chunkSizeString[0] == '-') {
    throw StatusException(
        HTTP_BAD_REQUEST,
        "[2003] RequestParser: setChunkSize - invalid type: " +
            chunkSizeString);
  }

  std::stringstream ss;
  ss << chunkSizeString;
  ss >> std::hex >> _chunkSize;

  if (ss.fail() or !ss.eof()) {
    throw StatusException(
        HTTP_BAD_REQUEST,
        "[2004] RequestParser: setChunkSize - invalid type: " +
            chunkSizeString);
  }
}

// storageBuffer 저장
// - 남아있는 값에서 startIdx 이전 값들 삭제 후 buffer 값 추가
void RequestParser::setStorageBuffer(size_t startIdx, octet_t const* buffer,
                                     ssize_t bytesRead) {
  std::vector<octet_t> tmp;

  for (size_t i = startIdx; i < _storageBuffer.size(); i++) {
    tmp.push_back(_storageBuffer[i]);
  }
  for (ssize_t i = 0; i < bytesRead; i++) {
    tmp.push_back(buffer[i]);
  }

  _storageBuffer.clear();
  _storageBuffer = tmp;
}

// Private Method

// octet 파싱
void RequestParser::parseOctet(octet_t const& octet) {
  switch (_status % 10) {
    case READY:
      _status = REQUEST_LINE;
      // fallthrough
    case REQUEST_LINE:
      parseRequestLine(octet);
      break;
    case HEADER_FIELD:
      parseHeaderField(octet);
      break;
    case BODY_CONTENT_LENGTH:
      parseBodyContentLength(octet);
      break;
    case BODY_CHUNKED:
      parseBodyChunked(octet);
      break;
    case HEADER_FIELD_END:
    case DONE:
      _storageBuffer.push_back(octet);
      break;
    default:
      throw std::runtime_error(
          "[2005] RequestParser: parseOctet - "
          "no switch case exists for the _state");
  }
}

// RequestLine 파싱
// - CRLF가 입력되었을 경우 입력이 끝났다고 정의
void RequestParser::parseRequestLine(octet_t const& octet) {
  _requestLine.push_back(octet);

  if (octet == '\n' and isEndWithCRLF(_requestLine)) {
    if (_requestLine.size() == 2) {
      RequestParser::clear();
      return;
    }
    std::vector<std::string> processResult;
    processResult = processRequestLine();
    _request.storeRequestLine(processResult);
    _status = HEADER_FIELD;
  }
}

// HeaderField 파싱
// - CRLF가 입력되었을 경우 header field 한 줄의 입력이 끝났다고 정의
// - 단, CRLF만 입력되었을 경우는 header field의 입력 종료로 처리
void RequestParser::parseHeaderField(octet_t const& octet) {
  _header.push_back(octet);

  if (octet == '\n' and isEndWithCRLF(_header)) {
    if (_header.size() == 2) {
      _status = HEADER_FIELD_END;
      _header.clear();
      return;
    }

    std::vector<std::string> processResult;
    processResult = processHeaderField();
    _request.storeHeaderField(processResult);
    _header.clear();
  }
}

// body 파싱
// - bodyLength 만큼 저장되었을 경우 status를 DONE으로 변경
void RequestParser::parseBodyContentLength(octet_t const& octet) {
  _body.push_back(octet);

  if (_body.size() == _bodyLength) {
    std::string processResult = processBody();
    _request.storeBody(processResult);
    _status = DONE;
  }
}

// chunk body 파싱
void RequestParser::parseBodyChunked(octet_t const& octet) {
  switch (_status) {
    case BODY_CHUNKED:
      _status = BODY_CHUNK_SIZE;
      // fallthrough
    case BODY_CHUNK_SIZE:
      parseBodyChunkSize(octet);
      break;
    case BODY_CHUNK_DATA:
      parseBodyChunkData(octet);
      break;
    case BODY_CHUNK_TRAILER:
      parseBodyChunkTrailer(octet);
      break;
    default:
      throw std::runtime_error(
          "[2302] RequestParser: parseBodyChunked - invalid status");
  }
}

// chunk body에 대한 chunk-size 파싱
// - CRLF가 입력되었을 경우 입력 종료 후 chunk-size 저장
// - chunk-size가 0인 경우 last-chunk로 처리 후
//   입력받은 body를 request 객체에 저장
// - chunk-size가 1 이상인 경우 chunk-data를 읽도록 상태 변경
void RequestParser::parseBodyChunkSize(octet_t const& octet) {
  _chunkSizeBuffer.push_back(octet);

  if (octet == '\n' and isEndWithCRLF(_chunkSizeBuffer)) {
    processBodyChunkSize();
    _status = _chunkSize == 0 ? BODY_CHUNK_TRAILER : BODY_CHUNK_DATA;
    _chunkSizeBuffer.clear();

    if (_status == BODY_CHUNK_TRAILER) {
      std::string processResult = processBody();
      _request.storeBody(processResult);
    }
  }
}

// chunk body에 대한 chunk-data 파싱
// - chunk-size 만큼 chunk-data를 읽은 후 CRLF가 입력되지 않았을 경우 예외 발생
void RequestParser::parseBodyChunkData(octet_t const& octet) {
  size_t const crlfLength = 2;

  size_t const bodySize = _body.size();
  if ((bodySize == _bodyLength and octet != '\r') or
      (bodySize == _bodyLength + 1 and octet != '\n')) {
    throw StatusException(
        HTTP_BAD_REQUEST,
        "[2301] RequestParser: parseBodyChunkData - invalid format");
  }

  _body.push_back(octet);

  if (_body.size() == _bodyLength + crlfLength) {
    removeCRLF(_body);
    _status = BODY_CHUNK_SIZE;
  }
}

// chunk body에 대한 Trailer 파싱
// - Trailer는 무시
// - 형식 검사를 하지 않고 CRLF만 들어올 경우 종료
void RequestParser::parseBodyChunkTrailer(octet_t const& octet) {
  _header.push_back(octet);

  if (octet == '\n' and isEndWithCRLF(_header)) {
    if (_header.size() == 2) {
      _status = DONE;
      _header.clear();
      return;
    }

    _header.clear();
  }
}

// RequestLine 후처리
// - size가 3이 아닌 경우 예외 발생
std::vector<std::string> RequestParser::processRequestLine() {
  removeCRLF(_requestLine);

  std::vector<std::string> result;
  splitRequestLine(result);

  if (isInvalidFormatSize(result, 3)) {
    throw StatusException(
        HTTP_BAD_REQUEST,
        "[2100] RequestParser: processRequestLine - invalid format size");
  }
  return result;
}

// Body 파싱 전 BodyParsingStatus 확인
// - BODY_CONTENT_LENGTH, BODY_CHUNKED, DONE 중 하나로 설정
void RequestParser::setupBodyParse(void) {
  _status = checkBodyParsingStatus();

  if (_status == BODY_CONTENT_LENGTH) {
    std::string const& contentLengthValue =
        _request.getHeaderFieldValues("content-length");
    setBodyLength(contentLengthValue);

    if (_bodyLength == 0) _status = DONE;
  }
}

// HeaderField 후처리
// - 한 줄의 입력을 처리
// - size가 2가 아닌 경우 예외 발생
// - field-name의 길이가 1 미만이거나 Whitespace로 끝나는 경우 예외 발생
// - host 또는 connection인 경우 fieldValue를 소문자로 저장
std::vector<std::string> RequestParser::processHeaderField() {
  removeCRLF(_header);

  std::vector<std::string> result;
  splitHeaderField(result);

  if (isInvalidFormatSize(result, 2)) {
    throw StatusException(
        HTTP_BAD_REQUEST,
        "[2200] RequestParser: processHeaderField - invalid format size");
  }

  int const fieldNameIndex = 0, fieldValueIndex = 1;
  if (result[fieldNameIndex].size() < 1 or
      isWhitespace(result[fieldNameIndex][result[fieldNameIndex].size() - 1])) {
    throw StatusException(
        HTTP_BAD_REQUEST,
        "[2201] RequestParser: processHeaderField - No whitespace is "
        "allowed between the header field-name and colon");
  }

  result[fieldValueIndex] = trim(result[fieldValueIndex]);
  Util::toLowerCase(result[fieldNameIndex]);
  if (result[fieldNameIndex] == "host" or
      result[fieldNameIndex] == "connection")
    Util::toLowerCase(result[fieldValueIndex]);

  return result;
}

// chunkSize 후처리
// - _chunkSizeBuffer에 저장된 값을 _chunkSize에 저장 후 _bodyLength에 추가
void RequestParser::processBodyChunkSize() {
  removeCRLF(_chunkSizeBuffer);

  std::vector<std::string> result;
  splitBodyChunkSize(result);

  std::string const& chunkSizeString = result[0];
  setChunkSize(chunkSizeString);
  setBodyLength(_bodyLength + _chunkSize);
}

// body 후처리
// - vector 형식을 std::string으로 변환
std::string RequestParser::processBody() {
  std::string result(_body.begin(), _body.end());
  return result;
}

// requestLine를 SP( )을 기준으로 split
// - split한 결과는 매개변수 result에 저장
void RequestParser::splitRequestLine(std::vector<std::string>& result) {
  std::string requestLine(_requestLine.begin(), _requestLine.end());
  std::stringstream ss(requestLine);
  std::string token;

  while (std::getline(ss, token, ' ')) {
    result.push_back(token);
  }

  if (result.size() == 0) {
    throw StatusException(
        HTTP_BAD_REQUEST,
        "[2102] RequestParser: splitRequestLine - requestLine is empty");
  }
}

// header를 가장 처음 나온 COLON(:)을 기준으로 split
// - split한 결과는 매개변수 result에 저장
void RequestParser::splitHeaderField(std::vector<std::string>& result) {
  std::string headerField(_header.begin(), _header.end());
  size_t pos = headerField.find(COLON);

  if (pos != std::string::npos) {
    result.push_back(headerField.substr(0, pos));
    result.push_back(headerField.substr(pos + 1));
  }

  if (result.size() == 0) {
    throw StatusException(
        HTTP_BAD_REQUEST,
        "[2204] RequestParser: splitHeaderField - headerField is empty");
  }
}

// chunkSizeBuffer SEMICOLON(;)을 기준으로 split
// - split한 결과는 매개변수 result에 저장
void RequestParser::splitBodyChunkSize(std::vector<std::string>& result) {
  std::string chunkSizeBuffer(_chunkSizeBuffer.begin(), _chunkSizeBuffer.end());
  std::stringstream ss(chunkSizeBuffer);
  std::string token;

  while (std::getline(ss, token, ';')) {
    result.push_back(token);
  }

  if (result.size() == 0) {
    throw StatusException(
        HTTP_BAD_REQUEST,
        "[2300] RequestParser: splitBodyChunkSize - chunkSizeBuffer is empty");
  }
}

bool RequestParser::isBodyChunk(void) {
  if (_request.isHeaderFieldNameExists("transfer-encoding") == false)
    return false;

  std::stringstream ss(_request.getHeaderFieldValues("transfer-encoding"));
  std::string item;
  while (getline(ss, item, ',')) {
    if (trim(item) == "chunked") return true;
  }
  return false;
}

// _request 객체의 현재 Body Parsing Status 검사 후 enum EParsingStatus 반환
// - transfer-encoding 헤더 필드에 chunked라는 값이 존재하는 경우 BODY_CHUNKED
// - content-length 헤더 필드가 존재하는 경우 BODY_CONTENT_LENGTH
// - 이 외의 경우 DONE
EParsingStatus RequestParser::checkBodyParsingStatus() {
  if (isBodyChunk()) {
    if (_request.isHeaderFieldNameExists("content-length")) {
      throw StatusException(
          HTTP_BAD_REQUEST,
          "[2205] RequestParser: checkBodyParsingStatus - Content-Length field "
          "cannot exist when Transfer-Encoding field is included.");
    }
    return BODY_CHUNKED;
  }

  if (_request.isHeaderFieldNameExists("content-length"))
    return BODY_CONTENT_LENGTH;

  return DONE;
}

// result의 size가 인자로 받은 size와 일치하지 않는지 여부 반환
bool RequestParser::isInvalidFormatSize(std::vector<std::string> const& result,
                                        size_t size) {
  return (result.size() != size);
}

// 인자로 받은 vec이 CRLF로 끝나는지 여부 반환
// - vec의 size가 2 미만인 경우 false로 처리
bool RequestParser::isEndWithCRLF(std::vector<octet_t> const& vec) {
  if (vec.size() < 2) {
    return false;
  }

  if (vec[vec.size() - 2] == '\r' and vec.back() == '\n') {
    return true;
  }

  return false;
}

// vec에서 CRLF 제거
// - 인자로 받은 vec이 CRLF로 끝난다고 가정
// - vec의 size가 2 미만인 경우 2000 예외 발생
void RequestParser::removeCRLF(std::vector<octet_t>& vec) {
  if (vec.size() < 2) {
    throw std::runtime_error(
        "[2000] RequestParser: removeCRLF - CRLF does not exist for removal.");
  }
  vec.pop_back();
  vec.pop_back();
}

// string의 선행, 후행 공백 제거
std::string RequestParser::trim(std::string const& str) {
  std::string::const_iterator it = str.begin();
  while (it != str.end() and isWhitespace(*it)) {
    ++it;
  }

  std::string::const_reverse_iterator rit = str.rbegin();
  while (rit.base() != it and isWhitespace(*rit)) {
    ++rit;
  }

  return std::string(it, rit.base());
}

// WhiteSpace(SP / HTAB) 인지 확인
bool RequestParser::isWhitespace(int c) { return (c == SP or c == HTAB); }
