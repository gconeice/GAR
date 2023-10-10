#include <arithmetic.h>
#include <resource.h>

template <Mode m>
std::vector<Garbled::Bit<m>> constant_nat(std::size_t w, std::size_t x) {
  std::vector<Garbled::Bit<m>> out(w);
  for (std::size_t i = 0; i < w; ++i) {
    out[i] = constant<m>((x & (1 << i)) > 0);
  }
  return out;
}

template std::vector<Garbled::Bit<Mode::G>> constant_nat(std::size_t, std::size_t);
template std::vector<Garbled::Bit<Mode::E>> constant_nat(std::size_t, std::size_t);
template std::vector<Garbled::Bit<Mode::S>> constant_nat(std::size_t, std::size_t);

template <Mode m>
void ginput_nat(std::size_t x, std::span<Garbled::Bit<m>> out) {
  const auto w = out.size();
  for (std::size_t i = 0; i < w; ++i) {
    out[i] = ginput<m>((x & (1 << i)) > 0);
  }
}

template void ginput_nat(std::size_t, std::span<Garbled::Bit<Mode::G>>);
template void ginput_nat(std::size_t, std::span<Garbled::Bit<Mode::E>>);
template void ginput_nat(std::size_t, std::span<Garbled::Bit<Mode::S>>);

template <Mode m>
std::vector<Garbled::Bit<m>> ginput_nat(std::size_t w, std::size_t x) {
  std::vector<Garbled::Bit<m>> out(w);
  ginput_nat(x, std::span { out });
  return out;
}

template std::vector<Garbled::Bit<Mode::G>> ginput_nat(std::size_t, std::size_t);
template std::vector<Garbled::Bit<Mode::E>> ginput_nat(std::size_t, std::size_t);
template std::vector<Garbled::Bit<Mode::S>> ginput_nat(std::size_t, std::size_t);

template <Mode m>
Garbled::Bit<m> operator<=(
    std::span<const Garbled::Bit<m>> x,
    std::span<const Garbled::Bit<m>> y) {

  const auto w = x.size();
  assert(y.size() == w);

  if (w == 0) {
    return constant<m>(1);
  } else {
    Garbled::Bit<m> r = x[0] <= y[0];
    for (std::size_t i = 1; i < w; ++i) {
      r = (x[i] < y[i]) ^ ((x[i] == y[i]) & r);
    }
    return r;
  }
}

template Garbled::Bit<Mode::G> operator<=(
    std::span<const Garbled::Bit<Mode::G>>,
    std::span<const Garbled::Bit<Mode::G>>);
template Garbled::Bit<Mode::E> operator<=(
    std::span<const Garbled::Bit<Mode::E>>,
    std::span<const Garbled::Bit<Mode::E>>);
template Garbled::Bit<Mode::S> operator<=(
    std::span<const Garbled::Bit<Mode::S>>,
    std::span<const Garbled::Bit<Mode::S>>);

TEST_CASE("") {
  constexpr std::size_t w = 8;
  rc::prop("arithmetic <=", [](std::size_t x, std::size_t y) {
    x = x % (1 << w);
    y = y % (1 << w);
    test_circuit([&]<Mode m> {
      const auto xx = constant_nat<m>(w, x);
      const auto yy = constant_nat<m>(w, y);
      return dec(std::span { xx } <= std::span { yy });
    }, x <= y);
  });
}


template <Mode m>
Garbled::Bit<m> operator==(
    std::span<const Garbled::Bit<m>> x,
    std::span<const Garbled::Bit<m>> y) {

  const auto w = x.size();
  assert(y.size() == w);

  if (w == 0) {
    return constant<m>(1);
  } else {
    Garbled::Bit<m> r = x[0] == y[0];
    for (std::size_t i = 1; i < w; ++i) {
      r = (x[i] == y[i]) & r;
    }
    return r;
  }
}

template Garbled::Bit<Mode::G> operator==(
    std::span<const Garbled::Bit<Mode::G>>,
    std::span<const Garbled::Bit<Mode::G>>);
template Garbled::Bit<Mode::E> operator==(
    std::span<const Garbled::Bit<Mode::E>>,
    std::span<const Garbled::Bit<Mode::E>>);
template Garbled::Bit<Mode::S> operator==(
    std::span<const Garbled::Bit<Mode::S>>,
    std::span<const Garbled::Bit<Mode::S>>);

TEST_CASE("") {
  constexpr std::size_t w = 8;
  rc::prop("arithmetic ==", [](std::size_t x, std::size_t y) {
    x = x % (1 << w);
    y = y % (1 << w);
    test_circuit([&]<Mode m> {
      const auto xx = constant_nat<m>(w, x);
      const auto yy = constant_nat<m>(w, y);
      return dec(std::span { xx } == std::span { yy });
    }, x == y);
  });
}


template <Mode m>
std::size_t dec_nat(std::span<const Garbled::Bit<m>> x) {
  std::size_t out = 0;
  for (std::size_t i = 0; i < x.size(); ++i) {
    out ^= (static_cast<std::size_t>(dec(x[i])) << i);
  }
  return out;
}

template std::size_t dec_nat(std::span<const Garbled::Bit<Mode::G>>);
template std::size_t dec_nat(std::span<const Garbled::Bit<Mode::E>>);
template std::size_t dec_nat(std::span<const Garbled::Bit<Mode::S>>);


template <Mode m>
std::span<Garbled::Bit<m>> operator+=(
    std::span<Garbled::Bit<m>> x,
    std::span<const Garbled::Bit<m>> y) {

  const auto w = x.size();
  assert(y.size() == w);

  if (w > 0) {
    auto carry = x[0] & y[0];
    x[0] ^= y[0];

    for (std::size_t i = 1; i < w; ++i) {
      const auto new_carry = (x[i] ^ carry) & (y[i] ^ carry) ^ carry;
      x[i] ^= y[i];
      x[i] ^= carry;
      carry = new_carry;
    }
  }
  return x;
}

template std::span<Garbled::Bit<Mode::G>> operator+=(
    std::span<Garbled::Bit<Mode::G>>, std::span<const Garbled::Bit<Mode::G>>);
template std::span<Garbled::Bit<Mode::E>> operator+=(
    std::span<Garbled::Bit<Mode::E>>, std::span<const Garbled::Bit<Mode::E>>);
template std::span<Garbled::Bit<Mode::S>> operator+=(
    std::span<Garbled::Bit<Mode::S>>, std::span<const Garbled::Bit<Mode::S>>);

TEST_CASE("") {
  constexpr std::size_t w = 8;
  rc::prop("arithmetic +=", [](std::size_t x, std::size_t y) {
    x = x % (1 << w);
    y = y % (1 << w);
    test_circuit([&]<Mode m> {
      auto xx = constant_nat<m>(w, x);
      const auto yy = constant_nat<m>(w, y);
      std::span { xx } += std::span { yy };
      return dec_nat(std::span<const Garbled::Bit<m>> { xx });
    }, (x + y) % (1 << w));
  });
}


template <Mode m>
std::span<Garbled::Bit<m>> operator-=(
    std::span<Garbled::Bit<m>> x,
    std::span<const Garbled::Bit<m>> y) {

  const auto w = x.size();
  assert(y.size() == w);

  if (w > 0) {
    auto carry = constant<m>(0);
    for (std::size_t i = 0; i < w-1; ++i) {
      const auto new_carry = (x[i] ^ y[i]) & (y[i] ^ carry) ^ carry;
      x[i] ^= y[i];
      x[i] ^= carry;
      carry = new_carry;
    }
    x[w-1] ^= y[w-1];
    x[w-1] ^= carry;
  }
  return x;
}

template std::span<Garbled::Bit<Mode::G>> operator-=(
    std::span<Garbled::Bit<Mode::G>>, std::span<const Garbled::Bit<Mode::G>>);
template std::span<Garbled::Bit<Mode::E>> operator-=(
    std::span<Garbled::Bit<Mode::E>>, std::span<const Garbled::Bit<Mode::E>>);
template std::span<Garbled::Bit<Mode::S>> operator-=(
    std::span<Garbled::Bit<Mode::S>>, std::span<const Garbled::Bit<Mode::S>>);


TEST_CASE("") {
  constexpr std::size_t w = 8;
  rc::prop("arithmetic -=", [](std::size_t x, std::size_t y) {
    x = x % (1 << w);
    y = y % (1 << w);
    test_circuit([&]<Mode m> {
      auto xx = constant_nat<m>(w, x);
      const auto yy = constant_nat<m>(w, y);
      std::span { xx } -= std::span { yy };
      return dec_nat(std::span<const Garbled::Bit<m>> { xx });
    }, (x - y) % (1 << w));
  });
}
