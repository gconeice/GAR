#ifndef UTIL_H__
#define UTIL_H__

#include <cstdint>
#include <cstddef>
#include <vector>
#include <array>
#include <span>


template <typename T>
inline std::span<const T> to_span(
    const std::vector<T>& xs, std::size_t start, std::size_t n) {
  std::span<const T> s = xs;
  return s.subspan(start, n);
}

template <typename T>
inline std::span<const T> to_span(
    const std::vector<T>& xs, std::size_t start) {
  std::span<const T> s = xs;
  return s.subspan(start);
}

template <typename T>
inline std::span<T> to_span(
    std::vector<T>& xs, std::size_t start, std::size_t n) {
  std::span<T> s = xs;
  return s.subspan(start, n);
}

template <typename T>
inline std::span<T> to_span(
    std::vector<T>& xs, std::size_t start) {
  std::span<T> s = xs;
  return s.subspan(start);
}

template <std::size_t n>
std::vector<bool> bool_arr_to_vec(const std::array<bool, n>& arr) {
  std::vector<bool> out;
  out.reserve(n);
  for (auto b: arr) { out.emplace_back(b); }
  return out;
}


template <typename F, typename S>
std::vector<decltype(F()(S()))> map(const F& f, const std::vector<S>& x) {
  using T = decltype(F()(S()));
  std::vector<T> y(x.size());
  for (std::size_t i = 0; i < x.size(); ++i) { y[i] = f(x[i]); }
  return y;
}

template <typename F, typename S>
std::vector<decltype(F()(S()))> map(const F& f, std::span<const S> x) {
  using T = decltype(F()(S()));
  std::vector<T> y(x.size());
  for (std::size_t i = 0; i < x.size(); ++i) { y[i] = f(x[i]); }
  return y;
}

constexpr std::size_t log2(std::size_t n) {
  std::size_t out = 0;
  n = n * 2 - 1;
  while (n > 1) {
    ++out;
    n >>= 1;
  }
  return out;
}

constexpr bool is_pow2(std::size_t n) {
  if (n == 0) { return false; }
  return (1 << log2(n)) == n;
}

constexpr std::size_t ceil_div(std::size_t x, std::size_t y) {
  return (x + y - 1) / y;
}

constexpr bool divides(std::size_t x, std::size_t y) {
  if (y == 0) { return true; }
  else { return y % x == 0; }
}

constexpr std::size_t natpow(std::size_t x, std::size_t p) {
  if (p == 0) { return 1; }
  if (p == 1) { return x; }

  const std::size_t xp2 = natpow(x, p/2);
  if (p % 2 == 0) {
    return xp2 * xp2;
  } else {
    return x * xp2 * xp2;
  }
}

#endif
