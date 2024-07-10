#ifndef B_BITCOUNT_HPP
#define B_BITCOUNT_HPP

#include "BTypes.h"

namespace BBoard
{

enum BitCountType {
    CNT64,
    CNT64_MAX15,
    CNT32,
    CNT32_MAX15,
    CNT_POPCNT
};

/// count_1s() counts the number of nonzero bits in a bitboard.
/// We have different optimized versions according if platform
/// is 32 or 64 bits, and to the maximum number of nonzero bits.
/// We also support hardware popcnt instruction. See Readme.txt
/// on how to pgo compile with popcnt support.
template<BitCountType> inline int count_1s(Bitboard);

template<>
inline int count_1s<CNT64>(Bitboard b) {
  b -= ((b>>1) & 0x5555555555555555ULL);
  b = ((b>>2) & 0x3333333333333333ULL) + (b & 0x3333333333333333ULL);
  b = ((b>>4) + b) & 0x0F0F0F0F0F0F0F0FULL;
  b *= 0x0101010101010101ULL;
  return int(b >> 56);
}

template<>
inline int count_1s<CNT64_MAX15>(Bitboard b) {
  b -= (b>>1) & 0x5555555555555555ULL;
  b = ((b>>2) & 0x3333333333333333ULL) + (b & 0x3333333333333333ULL);
  b *= 0x1111111111111111ULL;
  return int(b >> 60);
}

template<>
inline int count_1s<CNT32>(Bitboard b) {
  unsigned w = unsigned(b >> 32), v = unsigned(b);
  v -= (v >> 1) & 0x55555555; // 0-2 in 2 bits
  w -= (w >> 1) & 0x55555555;
  v = ((v >> 2) & 0x33333333) + (v & 0x33333333); // 0-4 in 4 bits
  w = ((w >> 2) & 0x33333333) + (w & 0x33333333);
  v = ((v >> 4) + v) & 0x0F0F0F0F; // 0-8 in 8 bits
  v += (((w >> 4) + w) & 0x0F0F0F0F);  // 0-16 in 8 bits
  v *= 0x01010101; // mul is fast on amd procs
  return int(v >> 24);
}

template<>
inline int count_1s<CNT32_MAX15>(Bitboard b) {
  unsigned w = unsigned(b >> 32), v = unsigned(b);
  v -= (v >> 1) & 0x55555555; // 0-2 in 2 bits
  w -= (w >> 1) & 0x55555555;
  v = ((v >> 2) & 0x33333333) + (v & 0x33333333); // 0-4 in 4 bits
  w = ((w >> 2) & 0x33333333) + (w & 0x33333333);
  v += w; // 0-8 in 4 bits
  v *= 0x11111111;
  return int(v >> 28);
}

template<>
inline int count_1s<CNT_POPCNT>(Bitboard b) {
#if !defined(USE_POPCNT)
  return int(b != 0); // Avoid 'b not used' warning
#elif defined(_MSC_VER) && defined(__INTEL_COMPILER)
  return _mm_popcnt_u64(b);
#elif defined(_MSC_VER)
  return (int)__popcnt64(b);
#elif defined(__GNUC__)
  unsigned long ret;
  __asm__("popcnt %1, %0" : "=r" (ret) : "r" (b));
  return ret;
#endif
}
}

#endif // !defined(BITCOUNT_H_INCLUDED)
