#ifndef THEPROJECT2_MULTIDIMARRAYINDEX_H
#define THEPROJECT2_MULTIDIMARRAYINDEX_H

namespace util{

  template <int32_t DimX>
  static inline int32_t ArrayIndex(int32_t x, int32_t y){
    return y*DimX + x;
  }

  template <int32_t DimX, int32_t DimY>
  static inline int32_t ArrayIndex(int32_t x, int32_t y, int32_t z){
    return z*DimX*DimY + y*DimX + x;
  }
}

#endif // THEPROJECT2_MULTIDIMARRAYINDEX_H
