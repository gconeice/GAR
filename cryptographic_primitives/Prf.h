#ifndef SGC2PC_2PC_PRF_H_
#define SGC2PC_2PC_PRF_H_

#include <array>
#include <bitset>

// Codes from: https://github.com/emp-toolkit/emp-tool/blob/b07a7d9ab3053a3e16991751402742d418377f63/emp-tool/utils/block.h
#ifndef EMP_UTIL_BLOCK_H__
#ifdef __x86_64__
#include <immintrin.h>
#elif __aarch64__
#include "Sse2neon.h"
inline __m128i _mm_aesimc_si128(__m128i a) {
	return vreinterpretq_m128i_u8(vaesimcq_u8(vreinterpretq_u8_m128i(a)));
}
inline __m128i _mm_aesdec_si128 (__m128i a, __m128i RoundKey)
{
    return vreinterpretq_m128i_u8(vaesimcq_u8(vaesdq_u8(vreinterpretq_u8_m128i(a), vdupq_n_u8(0)) ^ vreinterpretq_u8_m128i(RoundKey)));
}

inline __m128i _mm_aesdeclast_si128 (__m128i a, __m128i RoundKey)
{
    return vreinterpretq_m128i_u8(vaesdq_u8(vreinterpretq_u8_m128i(a), vdupq_n_u8(0)) ^ vreinterpretq_u8_m128i(RoundKey));
}
#endif
#endif
// Until here

struct PRF {
 public:
  static constexpr std::size_t nrounds = 10;

  PRF();
  PRF(std::bitset<128>);

  std::bitset<128> operator()(std::bitset<128> inp) const;

  std::bitset<128> *get_key();

 private:
  std::array<__m128i, nrounds+1> key;
};

#endif //SGC2PC_2PC_PRF_H_
