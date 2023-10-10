#ifndef PRF_H__
#define PRF_H__


#include <bitset>
#include <array>
#include <random>
#include <immintrin.h>
#include <label.h>


/* struct PRF { */
/* public: */
/*   static constexpr std::size_t nrounds = 10; */

/*   PRF(); */
/*   PRF(std::bitset<128>); */
/*   std::bitset<128> operator()(std::bitset<128> inp) const; */

/* private: */
/*   std::array<__m128i, nrounds+1> key; */
/* }; */


/* inline std::bitset<128> rand_key() { */
/*   std::bitset<128> buf; */
/*   std::uint32_t* bufslice = (uint32_t *)(&buf); */
/*   std::random_device dev; */
/*   for (size_t i = 0; i < sizeof(std::bitset<128>) / sizeof(std::uint32_t); ++i) { */
/*     bufslice[i] = dev(); */
/*   } */
/*   return buf; */
/* } */


/* template <int aes_const> */
/* __m128i expand_assist(__m128 k) { */
/*   auto keygened = _mm_aeskeygenassist_si128(k, aes_const); */
/*   keygened = _mm_shuffle_epi32(keygened, _MM_SHUFFLE(3,3,3,3)); */
/*   k = _mm_xor_si128(k, _mm_slli_si128(k, 4)); */
/*   k = _mm_xor_si128(k, _mm_slli_si128(k, 4)); */
/*   k = _mm_xor_si128(k, _mm_slli_si128(k, 4)); */
/*   return _mm_xor_si128(k, keygened); */
/* } */


/* template <int round> */
/* void expand(std::array<__m128i, PRF::nrounds+1>& key) { */
/*   if constexpr (round <= PRF::nrounds) { */
/*     key[round] = expand_assist<(1 << (round-1)) % 229>(key[round-1]); */
/*     expand<round+1>(key); */
/*   } */
/* } */


/* inline PRF::PRF() { */
/*   std::bitset<128> packed = rand_key(); */
/*   key[0] = *(const __m128i*)&packed; */
/*   expand<1>(key); */
/* } */


/* inline PRF::PRF(std::bitset<128> packed) { */
/*   key[0] = *(const __m128i*)&packed; */
/*   expand<1>(key); */
/* } */


/* inline std::bitset<128> PRF::operator()(std::bitset<128> inp) const { */
/*   __m128i tar = *(const __m128i*)&inp; */
/*   tar = _mm_xor_si128(tar, key[0]); */
/*   for (std::size_t i = 1; i < nrounds; ++i) { */
/*     tar = _mm_aesenc_si128(tar, key[i]); */
/*   } */
/*   const auto out = _mm_aesenclast_si128(tar, key[nrounds]); */
/*   return *(const std::bitset<128>*)&out; */
/* } */


/**
 * Circular correlation robust hash function (EPCCRH)
 * https://eprint.iacr.org/2019/074.pdf
 *
 * H(x) := π(σ(x)) + σ(x)
 * where π is implemented by AES_k
 *   and σ is a "linear orthomorphism"
 */
struct EPCCRH {
public:
  static constexpr std::size_t nrounds = 10;

  EPCCRH();
  EPCCRH(Label);
  Label operator()(const Label&) const;
  WLabel operator()(const WLabel&) const;

private:
  std::array<__m128i, nrounds+1> key;
  std::array<__m512i, nrounds+1> wide_key;
};

inline std::bitset<128> rand_key() {
  std::bitset<128> buf;
  std::uint32_t* bufslice = (uint32_t *)(&buf);
  std::random_device dev;
  for (size_t i = 0; i < sizeof(std::bitset<128>) / sizeof(std::uint32_t); ++i) {
    bufslice[i] = dev();
  }
  return buf;
}


template <int aes_const>
__m128i expand_assist(__m128i k) {
  auto keygened = _mm_aeskeygenassist_si128(k, aes_const);
  keygened = _mm_shuffle_epi32(keygened, _MM_SHUFFLE(3,3,3,3));
  k ^= _mm_slli_si128(k, 4);
  k ^= _mm_slli_si128(k, 4);
  k ^= _mm_slli_si128(k, 4);
  return k ^ keygened;
}


inline __m512i project(__m128i x) {
  static __m128i arr[4];
  arr[0] = x;
  arr[1] = x;
  arr[2] = x;
  arr[3] = x;
  return *reinterpret_cast<const __m512i*>(arr);
}


template <int round>
void expand(std::array<__m128i, EPCCRH::nrounds+1>& key, std::array<__m512i, EPCCRH::nrounds+1>& wide_key) {
  if constexpr (round <= EPCCRH::nrounds) {
    key[round] = expand_assist<(1 << (round-1)) % 229>(key[round-1]);
    wide_key[round] = project(key[round]);
    expand<round+1>(key, wide_key);
  }
}


inline EPCCRH::EPCCRH() {
  Label packed = rand_key();
  key[0] = packed;
  wide_key[0] = project(key[0]);
  expand<1>(key, wide_key);
}


inline EPCCRH::EPCCRH(Label packed) {
  key[0] = packed;
  wide_key[0] = project(key[0]);
  expand<1>(key, wide_key);
}


inline Label sigma(Label x) {
  static const Label mask = _mm_set_epi64x(0xFFFFFFFFFFFFFFFF, 0x00);
  return Label { _mm_shuffle_epi32(x, static_cast<_MM_PERM_ENUM>(78)) } ^ (x & mask);
}


inline Label EPCCRH::operator()(const Label& inp) const {
  const auto sx = sigma(inp);
  auto tar = sx ^ key[0];
  for (std::size_t i = 1; i < nrounds; ++i) {
    tar = _mm_aesenc_si128(tar, key[i]);
  }
  return sx ^ _mm_aesenclast_si128(tar, key[nrounds]);
}


inline __m512i sigma(const __m512i& x) {
  static constexpr __int64_t ones = 0xFFFFFFFFFFFFFFFF;
  static const __m512i mask = _mm512_set_epi64(ones, 0, ones, 0, ones, 0, ones, 0);

  return _mm512_shuffle_epi32(x, static_cast<_MM_PERM_ENUM>(78)) ^ (x & mask);
}


inline WLabel EPCCRH::operator()(const WLabel& x) const {
  const __m512i* buf = x;

  __m512i out[2];
  for (std::size_t j = 0; j < 2; ++j ) {
    const auto sx = sigma(buf[j]);
    auto tar = sx ^ wide_key[0];
    for (std::size_t i = 1; i < nrounds; ++i) {
      tar = _mm512_aesenc_epi128(tar, wide_key[i]);
    }
    out[j] = sx ^ _mm512_aesenclast_epi128(tar, wide_key[nrounds]);
  }
  return { out[0], out[1] };
}


#endif

