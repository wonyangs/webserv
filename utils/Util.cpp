#include "Util.hpp"

std::string const Util::itos(int n) {
  std::stringstream ss;
  ss << n;
  return ss.str();
}

int Util::stoi(std::string const& str) {
  std::stringstream ss;
  int n;
  ss << str;
  ss >> n;

  if (ss.fail() or !ss.eof()) {
    throw std::runtime_error("[6102] Util: stoi - invalid str: " + str);
  }
  return n;
}

// string을 모두 대문자로 변경
void Util::toUpperCase(std::string& str) {
  for (std::string::iterator it = str.begin(); it != str.end(); ++it)
    *it = std::toupper(static_cast<unsigned char>(*it));
}

// string을 모두 소문자로 변경
void Util::toLowerCase(std::string& str) {
  for (std::string::iterator it = str.begin(); it != str.end(); ++it)
    *it = std::tolower(static_cast<unsigned char>(*it));
}

// path에서 .과 ..을 계산하여 반환
// - ..로 인해 /보다 뒤로 갈 경우 /로 고정
std::string const Util::removeDotSegments(std::string const& path) {
  std::string inputBuffer = path;
  std::string outputBuffer;

  while (!inputBuffer.empty()) {
    std::size_t segmentEnd = inputBuffer.find('/', 1);
    std::string segment = segmentEnd != std::string::npos
                              ? inputBuffer.substr(0, segmentEnd)
                              : inputBuffer;

    if (segment == "/." or segment == ".") {
      // pass
    } else if (segment == "/.." or segment == "..") {
      std::size_t pos = outputBuffer.find_last_of('/');
      if (pos != std::string::npos) {
        outputBuffer.erase(pos);
      }
    } else if (!segment.empty() and segment != "/") {
      outputBuffer += segment;
    }

    if (segmentEnd != std::string::npos) {
      inputBuffer.erase(0, segmentEnd);
    } else {
      inputBuffer.clear();
    }
  }

  return outputBuffer.empty() ? "/" : outputBuffer;
}

// 인자로 받은 16진수를 문자(char)로 변경하여 반환
char Util::hexToChar(std::string const& hexStr) {
  int n;

  std::stringstream ss;
  ss << std::hex << hexStr;
  ss >> n;
  return static_cast<char>(n);
}

// percent-encoding을 디코딩하여 반환
std::string Util::pctDecode(std::string const& str) {
  std::stringstream ss;

  for (size_t i = 0; i < str.size(); i++) {
    if (str[i] == '%' and i + 2 < str.size()) {
      ss << Util::hexToChar(str.substr(i + 1, 2));
      i += 2;
    } else {
      ss << str[i];
    }
  }
  return ss.str();
}

bool Util::isHex(char ch) {
  return ('0' <= ch and ch <= '9') or ('a' <= ch and ch <= 'f') or
         ('A' <= ch and ch <= 'F');
}

bool Util::isPctEncoded(std::string const& str) {
  if (str.front() != '%' or str.size() != 3) return false;
  if (Util::isHex(str[1]) and Util::isHex(str[2])) return true;
  return false;
}

bool Util::isUnreserved(char ch) {
  if (std::isalpha(ch) or std::isdigit(ch)) return true;

  std::string const& others = "-._~";
  return (others.find(ch) != std::string::npos);
}

bool Util::isSubDelims(char ch) {
  std::string const& subDelims = "!$&'()*+,;=";
  return (subDelims.find(ch) != std::string::npos);
}

bool Util::isValidPath(std::string const& path) {
  if (path.size() < 1 or path.front() != '/') return false;

  std::string const& others = "/:@";
  for (size_t i = 0; i < path.size(); i++) {
    char ch = path[i];
    if (others.find(ch) != std::string::npos) continue;
    if (Util::isUnreserved(ch) or Util::isSubDelims(ch)) continue;
    if (Util::isPctEncoded(path.substr(i, 3))) continue;

    return false;
  }
  return true;
}

bool Util::isValidQuery(std::string const& query) {
  std::string const& others = ":@/?";

  for (size_t i = 0; i < query.size(); i++) {
    char ch = query[i];
    if (others.find(ch) != std::string::npos) continue;
    if (Util::isUnreserved(ch) or Util::isSubDelims(ch)) continue;
    if (Util::isPctEncoded(query.substr(i, 3))) continue;

    return false;
  }
  return true;
}

// method가 매칭되는 enum EHttpMethod를 반환
// - 매칭되는 EHttpMethod가 없는 경우 예외 발생
EHttpMethod Util::matchEHttpMethod(std::string method) {
  if (method == "GET") return HTTP_GET;
  if (method == "POST") return HTTP_POST;
  if (method == "DELETE") return HTTP_DELETE;
  throw std::runtime_error("[] Util: matchEHttpMethod - match failed");
}

std::string Util::convertPath(std::string const& path) {
  return Util::removeDotSegments(Util::pctDecode(path));
}
