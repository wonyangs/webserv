#include "Util.hpp"

std::string const Util::itos(int n) {
  std::stringstream ss;
  ss << n;
  return ss.str();
}

stringVec Util::split(octetVec const& vec, char del) {
  stringVec result;

  std::string data(vec.begin(), vec.end());
  std::stringstream ss(data);
  std::string token;

  while (std::getline(ss, token, del)) {
    result.push_back(token);
  }

  if (result.size() == 0) {
    throw StatusException(HTTP_BAD_REQUEST,
                          "[6100] Util: split - vector is empty");
  }
  return result;
}

stringVec Util::splitOnce(octetVec const& vec, char del) {
  stringVec result;

  std::string data(vec.begin(), vec.end());
  size_t pos = data.find(del);

  if (pos != std::string::npos) {
    result.push_back(data.substr(0, pos));
    result.push_back(data.substr(pos + 1));
  }

  if (result.size() == 0) {
    throw StatusException(HTTP_BAD_REQUEST,
                          "[6101] Util: splitOnce - vector is empty");
  }
  return result;
}
