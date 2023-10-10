#ifndef LABEL_H__
#define LABEL_H__


#include <bitset>
#include <random>
#include <array>
#include <cstring>
#include <cstddef>

#include <iostream>
#include <iomanip>

#include <immintrin.h>

static std::bitset<128> mask_last_64 = 0ULL - 1;

// A "label" is a κ bit string.
struct Label {
public:
  Label() {
    val = _mm_set_epi64x(0, 0);
  }
  Label(int val) {
    this->val = _mm_set_epi64x(0, val);
  }
  
  Label(const std::bitset<128>& val) { 
    
    long long first_half = (val >> 64).to_ullong();
    long long second_half = (val & mask_last_64).to_ullong();
    //for (int i = 127; i >= 64; i--) first_half = (first_half << 1) + val[i];
    //for (int i = 63; i >= 0; i--) second_half = (second_half << 1) + val[i];
    this->val = _mm_set_epi64x(first_half, second_half);
  }
  
  //Label(const std::bitset<128>& val) : val(*reinterpret_cast<const __m128i*>(&val)) { }
  Label(const __m128i& val) : val(val) { }

  operator __m128i() const {
    return val;
  }

  operator std::bitset<128>() const {
    return *reinterpret_cast<const std::bitset<128>*>(&val);
  }

  const std::bitset<128>& bitset() const {
    return *reinterpret_cast<const std::bitset<128>*>(&val);
  }
  std::bitset<128>& bitset() {
    return *reinterpret_cast<std::bitset<128>*>(&val);
  }

  friend std::ostream& operator<<(std::ostream& os, const Label& l) {
    std::uint64_t xs[2];
    std::memcpy(xs, &l.val, 16);
    os << "addr: " << &l.val << "; ";
    os << std::setfill('0') << std::setw(16) << std::right << std::hex << xs[1];
    os << std::setfill('0') << std::setw(16) << std::right << std::hex << xs[0];
    os << std::dec;
    return os;
  }

  Label operator&(const Label& o) const { return { val & o.val }; }
  Label operator^(const Label& o) const { return { val ^ o.val }; }
  Label& operator^=(const Label& o) { val ^= o.val; return *this; }


  Label operator*(bool b) const {
    if (b) { return *this; } else { return 0; }
  }

  bool operator==(const Label& o) const { return bitset() == o.bitset(); }
  bool operator!=(const Label& o) const { return bitset() != o.bitset(); }

  bool lsb() const { return bitset()[0]; }
  void set_lsb(bool b) { bitset()[0] = b; }

private:
  //alignas(64) 
  __m128i val;
};


// A "wide label" is an eight lane vector, where each lane is κ bits.
struct WLabel {
public:
  WLabel() {
    val[0] = _mm512_set_epi64(0, 0, 0, 0, 0, 0, 0, 0);
    val[1] = _mm512_set_epi64(0, 0, 0, 0, 0, 0, 0, 0);
  }
  WLabel(int x) {
    val[0] = _mm512_set_epi64(x, 0, 0, 0, 0, 0, 0, 0);
    val[1] = _mm512_set_epi64(0, 0, 0, 0, 0, 0, 0, 0);
  }

  constexpr WLabel(__m512i x, __m512i y) {
    val[0] = x;
    val[1] = y;
  }

  WLabel(const std::array<Label, 8>& x) {
    std::memcpy(val, &x[0], 128);
  }

  operator const __m512i*() const {
    return reinterpret_cast<const __m512i*>(val);
  }

  operator __m512i*() {
    return reinterpret_cast<__m512i*>(val);
  }

  static WLabel broadcast(const Label& x) {
    // TODO is there a faster way?
    return { std::array<Label, 8> { x, x, x, x, x, x, x, x } };
  }

  friend std::ostream& operator<<(std::ostream& os, const WLabel& l) {
    std::uint64_t xs[16];
    std::memcpy(xs, l.val, 128);
    for (std::size_t i = 0; i < 8; ++i) {
      os << std::setfill('0') << std::setw(16) << std::right << std::hex << xs[16 - 2*i - 1];
      os << std::setfill('0') << std::setw(16) << std::right << std::hex << xs[16 - 2*i - 2];
      os << ' ';
    }
    os << std::dec;
    return os;
  }

  inline WLabel operator&(const WLabel& o) const {
    WLabel out;
    out.val[0] = val[0] & o.val[0];
    out.val[1] = val[1] & o.val[1];
    return out;
  }

  inline WLabel operator^(const WLabel& o) const {
    WLabel out;
    out.val[0] = val[0] ^ o.val[0];
    out.val[1] = val[1] ^ o.val[1];
    return out;
  }

  inline WLabel& operator^=(const WLabel& o) { (*this) = (*this) ^ o; return *this; }
  inline WLabel& operator&=(const WLabel& o) { (*this) = (*this) & o; return *this; }

  WLabel operator*(std::byte b) const {
    std::bitset<8> bb = std::to_integer<int>(b);
    WLabel out;
    const std::bitset<128>* labels = reinterpret_cast<const std::bitset<128>*>(val);
    std::bitset<128>* olabels = reinterpret_cast<std::bitset<128>*>(out.val);
    for (std::size_t i = 0; i < 8; ++i) {
      olabels[i] = bb[i] ? labels[i] : 0;
    }
    return out;
  }

  std::byte lsb() const {
    const std::bitset<128>* labels = reinterpret_cast<const std::bitset<128>*>(val);
    uint8_t out = 0;
    for (std::size_t i = 0; i < 8; ++i) {
      out ^= labels[i][0] << i;
    }
    return static_cast<std::byte>(out);
  }

  void set_lsb(std::byte b) {
    std::bitset<128>* labels = reinterpret_cast<std::bitset<128>*>(val);
    for (std::size_t i = 0; i < 8; ++i) {
      bool bit = (static_cast<int>(b) >> i) & 1;
      labels[i][0] = bit;
    }
  }

private:
  __m512i val[2];
};


#endif
