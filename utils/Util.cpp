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

    if (segment == "/." || segment == ".") {
      // pass
    } else if (segment == "/.." || segment == "..") {
      std::size_t pos = outputBuffer.find_last_of('/');
      if (pos != std::string::npos) {
        outputBuffer.erase(pos);
      }
    } else if (!segment.empty() && segment != "/") {
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
