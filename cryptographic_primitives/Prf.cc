#include <random>

#include "Prf.h"

template <int aes_const>
__m128i expand_assist(__m128i k) {
  auto keygened = _mm_aeskeygenassist_si128(k, aes_const);
  keygened = _mm_shuffle_epi32(keygened, _MM_SHUFFLE(3,3,3,3));
  k = _mm_xor_si128(k, _mm_slli_si128(k, 4));
  k = _mm_xor_si128(k, _mm_slli_si128(k, 4));
  k = _mm_xor_si128(k, _mm_slli_si128(k, 4));
  return _mm_xor_si128(k, keygened);
}


template <int round>
void expand(std::array<__m128i, PRF::nrounds+1>& key) {
  if constexpr (round <= PRF::nrounds) {
    key[round] = expand_assist<(1 << (round-1)) % 229>(key[round-1]);
    expand<round+1>(key);
  }
}


PRF::PRF() {
  __m128i buf;
  std::uint32_t* bufslice = (uint32_t *)(&buf);
  std::random_device rand_div;
  for (size_t i = 0; i < sizeof(__m128i) / sizeof(std::uint32_t); ++i) {
    bufslice[i] = rand_div();
  }
  key[0] = buf;
  expand<1>(key);
}


PRF::PRF(std::bitset<128> packed) {
  key[0] = *(const __m128i*)&packed;
  expand<1>(key);
}


std::bitset<128> PRF::operator()(std::bitset<128> inp) const {
  __m128i tar = *(const __m128i*)&inp;
  tar = _mm_xor_si128(tar, key[0]);
  for (std::size_t i = 1; i < nrounds; ++i) {
    tar = _mm_aesenc_si128(tar, key[i]);
  }
  const auto out = _mm_aesenclast_si128(tar, key[nrounds]);
  return *(const std::bitset<128>*)&out;
}

std::bitset<128> *PRF::get_key() {
  return (std::bitset<128> *)(&key[0]);
}