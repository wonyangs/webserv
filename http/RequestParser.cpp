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
    _parsedData = requestParser._parsedData;
    _tempStorage = requestParser._tempStorage;
    _chunkSize = requestParser._chunkSize;
    _bodyLength = requestParser._bodyLength;
  }
  return *this;
}

// Public Method - getter

enum EParsingStatus RequestParser::getParsingStatus() const { return _status; }

Request const& RequestParser::getRequest() const { return _request; }

// Public Method - setter

void RequestParser::setRequestLocation(Location const& location) {
  _request.setLocation(location);
}

// Public Method

#include <iostream>

// buffer 값 파싱
// - 만약 _tempStorage가 비어있지 않으면 해당 값 먼저 파싱 진행
// - 이후 buffer 값 파싱
// - 파싱 도중 HEADER_FIELD_END 또는 DONE이 되었을 경우,
//   남은 octets은 _tempStorage에 저장
void RequestParser::parse(u_int8_t const* buffer, ssize_t bytesRead) {
  if (_status == HEADER_FIELD_END) {
    setupBodyParse();

    if (_status == DONE) return;
  }

  for (size_t i = 0; i < _tempStorage.size(); i++) {
    parseOctet(_tempStorage[i]);

    if ((_status == HEADER_FIELD_END or _status == DONE) and
        i + 1 != _tempStorage.size()) {
      setTempStorage(i + 1, buffer, bytesRead);
      return;
    }
  }

  _tempStorage.clear();
  for (ssize_t i = 0; i < bytesRead; i++) {
    parseOctet(buffer[i]);
  }
}

// 멤버 변수를 비어있는 상태로 초기화
// - _tempStorage 변수는 제외
void RequestParser::clear() {
  _status = READY;
  _parsedData.clear();
  _chunkSize = 0;
  _bodyLength = 0;
}

// _tempStorage 변수가 비어있지 않은지 여부 확인
bool RequestParser::isTempStorageNotEmpty() {
  return (_tempStorage.size() != 0);
}

// Private Method - setter

// bodyLength 저장
// - 만약 bodyLengthString 을 size_t로 변환 실패할 경우 예외 발생
// - TODO: config에 있는 client_max_body_size 예외 처리 필요
void RequestParser::setBodyLength(std::string const& bodyLengthString) {
  if (bodyLengthString.size() < 1 or bodyLengthString[0] == '-') {
    throw std::invalid_argument(
        "[2001] RequestParser: setBodyLength - invalid type: " +
        bodyLengthString);
  }

  std::stringstream ss;
  ss << bodyLengthString;
  ss >> _bodyLength;

  if (ss.fail() or !ss.eof()) {
    throw std::invalid_argument(
        "[2002] RequestParser: setBodyLength - invalid type: " +
        bodyLengthString);
  }
}

void RequestParser::setChunkSize(std::string const& chunkSizeString) {
  if (chunkSizeString.size() < 1 or chunkSizeString[0] == '-') {
    throw std::invalid_argument(
        "[2003] RequestParser: setChunkSize - invalid type: " +
        chunkSizeString);
  }

  std::stringstream ss;
  ss << chunkSizeString;
  ss >> std::hex >> _chunkSize;

  if (ss.fail() or !ss.eof()) {
    throw std::invalid_argument(
        "[2004] RequestParser: setChunkSize - invalid type: " +
        chunkSizeString);
  }
}

// TempStorage 저장
// - 남아있는 값에서 startIdx 이전 값들 삭제 후 buffer 값 추가
void RequestParser::setTempStorage(size_t startIdx, u_int8_t const* buffer,
                                     ssize_t bytesRead) {
  std::vector<u_int8_t> tmp;

  for (size_t i = startIdx; i < _tempStorage.size(); i++) {
    tmp.push_back(_tempStorage[i]);
  }
  for (ssize_t i = 0; i < bytesRead; i++) {
    tmp.push_back(buffer[i]);
  }

  _tempStorage.clear();
  _tempStorage = tmp;
}

// Private Method

// octet 파싱
void RequestParser::parseOctet(u_int8_t const& octet) {
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
      _tempStorage.push_back(octet);
      break;
    default:
      throw std::runtime_error(
          "[2005] RequestParser: parseOctet - "
          "no switch case exists for the _state");
  }
}

// RequestLine 파싱
// - CRLF가 입력되었을 경우 입력이 끝났다고 정의
void RequestParser::parseRequestLine(u_int8_t const& octet) {
  _parsedData.push_back(octet);

  if (octet == '\n' and isEndWithCRLF(_parsedData)) {
    if (_parsedData.size() == 2) {
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
void RequestParser::parseHeaderField(u_int8_t const& octet) {
  _parsedData.push_back(octet);

  if (octet == '\n' and isEndWithCRLF(_parsedData)) {
    if (_parsedData.size() == 2) {
      _status = HEADER_FIELD_END;
      _parsedData.clear();
      return;
    }

    std::vector<std::string> processResult;
    processResult = processHeaderField();
    _request.storeHeaderField(processResult);
  }
}

// body 파싱
// - bodyLength 만큼 저장되었을 경우 status를 DONE으로 변경
void RequestParser::parseBodyContentLength(u_int8_t const& octet) {
  _parsedData.push_back(octet);

  if (_parsedData.size() == _bodyLength) {
    std::string processResult = processBody();
    _request.storeBody(processResult);
    _status = DONE;
  }
}

// chunk body 파싱
void RequestParser::parseBodyChunked(u_int8_t const& octet) {
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
void RequestParser::parseBodyChunkSize(u_int8_t const& octet) {
  _parsedData.push_back(octet);

  if (octet == '\n' and isEndWithCRLF(_parsedData)) {
    processBodyChunkSize();
    _status = _chunkSize == 0 ? BODY_CHUNK_TRAILER : BODY_CHUNK_DATA;
    _parsedData.clear();

    if (_status == BODY_CHUNK_TRAILER) {
      std::string processResult = processBody();
      _request.storeBody(processResult);
    }
  }
}

// chunk body에 대한 chunk-data 파싱
// - chunk-size 만큼 chunk-data를 읽은 후 CRLF가 입력되지 않았을 경우 예외 발생
void RequestParser::parseBodyChunkData(u_int8_t const& octet) {
  size_t const crlfLength = 2;

  size_t const bodySize = _parsedData.size();
  if ((bodySize == _bodyLength and octet != '\r') or
      (bodySize == _bodyLength + 1 and octet != '\n')) {
    throw StatusException(
        HTTP_BAD_REQUEST,
        "[2301] RequestParser: parseBodyChunkData - invalid format");
  }

  _parsedData.push_back(octet);

  if (_parsedData.size() == _bodyLength + crlfLength) {
    removeCRLF(_parsedData);
    _status = BODY_CHUNK_SIZE;
  }
}

// chunk body에 대한 Trailer 파싱
// - Trailer는 무시
// - 형식 검사를 하지 않고 CRLF만 들어올 경우 종료
void RequestParser::parseBodyChunkTrailer(u_int8_t const& octet) {
  _parsedData.push_back(octet);

  if (octet == '\n' and isEndWithCRLF(_parsedData)) {
    if (_parsedData.size() == 2) {
      _status = DONE;
      _parsedData.clear();
      return;
    }

    _parsedData.clear();
  }
}

// RequestLine 후처리
// - size가 3이 아닌 경우 예외 발생
std::vector<std::string> RequestParser::processRequestLine() {
  removeCRLF(_parsedData);

  std::vector<std::string> result;
  splitRequestLine(result);

  _parsedData.clear();

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
    std::vector<std::string> const& contentLengthValues =
        _request.getHeaderFieldValues("content-length");
    setBodyLength(contentLengthValues[0]);
  }
}

// HeaderField 후처리
// - 한 줄의 입력을 처리
// - size가 2가 아닌 경우 예외 발생
// - field-name의 길이가 1 미만이거나 Whitespace로 끝나는 경우 예외 발생
// - Request 객체에 이미 존재하는 field-name일 경우 예외 발생
std::vector<std::string> RequestParser::processHeaderField() {
  removeCRLF(_parsedData);

  std::vector<std::string> result;
  splitHeaderField(result);
  _parsedData.clear();

  if (isInvalidFormatSize(result, 2)) {
    throw StatusException(
        HTTP_BAD_REQUEST,
        "[2200] RequestParser: processHeaderField - invalid format size");
  }

  int const fieldNameIndex = 0, fieldValueIndex = 1;
  if (result[fieldNameIndex].size() < 1 or
      isWhitespace(result[fieldNameIndex].back())) {
    throw StatusException(
        HTTP_BAD_REQUEST,
        "[2201] RequestParser: processHeaderField - No whitespace is "
        "allowed between the header field-name and colon");
  }

  if (_request.isHeaderFieldNameExists(result[fieldNameIndex])) {
    throw StatusException(
        HTTP_BAD_REQUEST,
        "[2202] RequestParser: processHeaderField - duplicate field-name");
  }

  toLowerCase(result[fieldNameIndex]);
  result[fieldValueIndex] = trim(result[fieldValueIndex]);
  return result;
}

// chunkSize 후처리
// - _chunkSizeBuffer에 저장된 값을 _chunkSize에 저장 후 _bodyLength에 추가
void RequestParser::processBodyChunkSize() {
  removeCRLF(_parsedData);

  std::vector<std::string> result;
  splitBodyChunkSize(result);

  std::string const& chunkSizeString = result[0];
  setChunkSize(chunkSizeString);

  _bodyLength += _chunkSize;
}

// body 후처리
// - vector 형식을 std::string으로 변환
std::string RequestParser::processBody() {
  std::string result(_parsedData.begin(), _parsedData.end());
  _parsedData.clear();
  return result;
}

// requestLine를 SP( )을 기준으로 split
// - split한 결과는 매개변수 result에 저장
void RequestParser::splitRequestLine(std::vector<std::string>& result) {
  std::string requestLine(_parsedData.begin(), _parsedData.end());
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
  std::string headerField(_parsedData.begin(), _parsedData.end());
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
  std::string chunkSizeBuffer(_parsedData.begin(), _parsedData.end());
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

// _request 객체의 현재 Body Parsing Status 검사 후 enum EParsingStatus 반환
// - transfer-encoding 헤더 필드에 chunked라는 값이 존재하는 경우 BODY_CHUNKED
// - content-length 헤더 필드가 존재하는 경우 BODY_CONTENT_LENGTH
// - 이 외의 경우 DONE
EParsingStatus RequestParser::checkBodyParsingStatus() {
  if (_request.isHeaderFieldValueExists("transfer-encoding", "chunked"))
    return BODY_CHUNKED;

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
bool RequestParser::isEndWithCRLF(std::vector<u_int8_t> const& vec) {
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
void RequestParser::removeCRLF(std::vector<u_int8_t>& vec) {
  if (vec.size() < 2) {
    throw std::runtime_error(
        "[2000] RequestParser: removeCRLF - CRLF does not exist for removal.");
  }
  vec.pop_back();
  vec.pop_back();
}

// string을 모두 소문자로 변경
void RequestParser::toLowerCase(std::string& str) {
  for (std::string::iterator it = str.begin(); it != str.end(); ++it)
    *it = std::tolower(static_cast<unsigned char>(*it));
}

// string의 선행, 후행 공백 제거
std::string RequestParser::trim(std::string const& str) {
  std::string::const_iterator it = str.begin();
  while (it != str.end() && isWhitespace(*it)) {
    ++it;
  }

  std::string::const_reverse_iterator rit = str.rbegin();
  while (rit.base() != it && isWhitespace(*rit)) {
    ++rit;
  }

  return std::string(it, rit.base());
}

// WhiteSpace(SP / HTAB) 인지 확인
bool RequestParser::isWhitespace(int c) { return (c == SP or c == HTAB); }
