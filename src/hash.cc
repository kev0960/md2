#include "hash.h"

#include <openssl/evp.h>
#include <openssl/sha.h>

#include <iomanip>
#include <optional>
#include <sstream>
#include <string>

namespace md2 {
namespace {

constexpr unsigned int kSha256HashLength = 32;

std::string CharToHex(int c) {
  std::stringstream stream;
  stream << std::setfill('0') << std::setw(2) << std::hex << c;
  return stream.str();
}

}  // namespace

std::optional<std::string> GenerateSha256Hash(const std::string& s) {
  std::optional<std::string> maybe_hash = std::nullopt;

  EVP_MD_CTX* context = EVP_MD_CTX_new();

  if (context != NULL) {
    if (EVP_DigestInit_ex(context, EVP_sha256(), NULL)) {
      if (EVP_DigestUpdate(context, s.c_str(), s.length())) {
        unsigned char hash_arr[kSha256HashLength];
        unsigned int hash_length = kSha256HashLength;

        if (EVP_DigestFinal_ex(context, hash_arr, &hash_length)) {
          std::string hash;
          hash.reserve(kSha256HashLength);

          for (int c : hash_arr) {
            hash.append(CharToHex(c));
          }

          maybe_hash = hash;
        }
      }
    }

    EVP_MD_CTX_free(context);
  }

  return maybe_hash;
}

}  // namespace md2
