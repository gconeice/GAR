#ifndef WORD_H__
#define WORD_H__

#include <arithmetic.h>
#include <array>

constexpr std::size_t word_size = 32;

namespace Garbled {

template <Mode m>
struct Word {
public:
  Word() {
    val.resize(word_size);
  }
  Word(const std::vector<Garbled::Bit<m>>& v) {
    assert(v.size() == word_size);
    val.resize(word_size);
    std::copy(v.begin(), v.end(), val.begin());
  }

  Word(std::span<const Garbled::Bit<m>> v) {
    assert(v.size() == word_size);
    val.resize(word_size);
    std::copy(v.begin(), v.end(), val.begin());
  }

  static Word<m> constant(std::size_t x) {
    return constant_nat<m>(word_size, x);
  }

  static Word<m> ginput(std::size_t x) {
    return ginput_nat<m>(word_size, x);
  }

  Bit<m> operator<=(const Word<m>& o) const {
    return
      std::span<const Garbled::Bit<m>> { val }
      <=
      std::span<const Garbled::Bit<m>> { o.val };
  }

  Bit<m> operator>=(const Word<m>& o) const { return o <= *this; }
  Bit<m> operator<(const Word<m>& o) const { return ~(*this <= o); }
  Bit<m> operator>(const Word<m>& o) const { return ~(*this >= o); }
  Bit<m> operator==(const Word<m>& o) const {
    return
      std::span<const Garbled::Bit<m>> { val }
      ==
      std::span<const Garbled::Bit<m>> { o.val };
  }
  Bit<m> operator!=(const Word<m>& o) const { return ~(*this == o); }

  Word<m>& operator+=(const Word<m>& o) {
    std::span { val } += std::span<const Garbled::Bit<m>> { o.val };
    return *this;
  }
  Word<m>& operator-=(const Word<m>& o) {
    std::span { val } -= std::span<const Garbled::Bit<m>> { o.val };
    return *this;
  }

  Word<m> operator+(const Word<m>& o) const { Word out = *this; out += o; return out; }
  Word<m> operator-(const Word<m>& o) const { Word out = *this; out -= o; return out; }
  Word<m>& operator*=(const Bit<m>& o) {
    std::span { val } &= o;
    return *this;
  }
  Word<m> operator*(const Bit<m>& o) const {
    Word<m> out = *this;
    out *= o;
    return out;
  }
  Word<m>& operator^=(const Word<m>& o) {
    std::span { val } ^= std::span { o.val };
    return *this;
  }
  Word<m> operator^(const Word<m>& o) const {
    Word<m> out = *this;
    out ^= o;
    return out;
  }

  operator std::span<Garbled::Bit<m>>() { return val; }
  operator std::span<const Garbled::Bit<m>>() const { return val; }

  std::span<Garbled::Bit<m>> to_span() { return val; }
  std::span<const Garbled::Bit<m>> to_span() const { return val; }
  std::span<Garbled::Bit<m>> to_span(std::size_t start) {
    return to_span().subspan(start);
  }
  std::span<const Garbled::Bit<m>> to_span(std::size_t start) const {
    return to_span().subspan(start);
  }
  std::span<Garbled::Bit<m>> to_span(std::size_t start, std::size_t n) {
    return to_span().subspan(start, n);
  }
  std::span<const Garbled::Bit<m>> to_span(std::size_t start, std::size_t n) const {
    return to_span().subspan(start, n);
  }


  Word<m>& operator++() { *this += Word::constant(1); return *this; }
  Word<m>& operator--() { *this -= Word::constant(1); return *this; }
  Word<m> operator++(int) {
    const auto temp = *this;
    *this += Word::constant(1);
    return temp;
  }
  Word<m> operator--(int) {
    const auto temp = *this;
    *this -= Word::constant(1);
    return temp;
  }

  std::size_t dec() const {
    return dec_nat(std::span { val });
  }

private:
  std::vector<Garbled::Bit<m>> val;
};


template struct Word<Mode::G>;
template struct Word<Mode::E>;
template struct Word<Mode::S>;

}

#endif
