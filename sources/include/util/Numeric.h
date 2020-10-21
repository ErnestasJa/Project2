#ifndef THEPROJECT2_INCLUDE_UTILS_NUMERIC_H_
#define THEPROJECT2_INCLUDE_UTILS_NUMERIC_H_

#include <cstdlib>

namespace util {
namespace numeric {
  constexpr float FloatingPointRoundingError = 0.00001f;
  bool equals(float a, float b){
    return std::abs(a-b) <= FloatingPointRoundingError;
  }
}
}

#endif // THEPROJECT2_INCLUDE_UTILS_NUMERIC_H_
