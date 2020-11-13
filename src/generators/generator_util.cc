#include "generator_util.h"

namespace md2 {
namespace {

constexpr std::string_view kItguru = "http://itguru.tistory.com";

}

std::string_view StripItguruLink(std::string_view url) {
  if (url.substr(0, kItguru.size()) == kItguru) {
    url.remove_prefix(kItguru.size());
  }

  return url;
}

}  // namespace md2

