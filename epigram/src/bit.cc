#include <bit.h>
#include <resource.h>
#include <util.h>


template <Mode m>
Garbled::Bit<m> constant(bool b) {
  GEN { return (delta() ^ 1) * b; }
  EVAL { return { b }; }
  SPEC { return { }; }
}

template Garbled::Bit<Mode::G> constant(bool);
template Garbled::Bit<Mode::E> constant(bool);
template Garbled::Bit<Mode::S> constant(bool);


template <Mode m>
Garbled::Bit<m> ginput(bool b) {
  GEN { return delta() * b; }
  EVAL { return { 0 }; }
  SPEC { return { }; }
}

template Garbled::Bit<Mode::G> ginput(bool);
template Garbled::Bit<Mode::E> ginput(bool);
template Garbled::Bit<Mode::S> ginput(bool);


template <Mode m>
Garbled::Bit<m> ginput(const Label& x) {
  GEN { return { x }; }
  EVAL { return { 0 }; }
  SPEC { return { }; }
}

template Garbled::Bit<Mode::G> ginput(const Label&);
template Garbled::Bit<Mode::E> ginput(const Label&);
template Garbled::Bit<Mode::S> ginput(const Label&);


template <Mode m>
Garbled::Bit<m> renc(bool b) {
  GEN {
    const auto g = random_label();
    send((g ^ (delta() * b)));
    return { g };
  }
  EVAL { return { recv() }; }
  SPEC { ++scost.rows; return { }; }
}

template Garbled::Bit<Mode::G> renc(bool);
template Garbled::Bit<Mode::E> renc(bool);
template Garbled::Bit<Mode::S> renc(bool);


template <Mode m>
std::vector<Garbled::Bit<m>> renc(const std::vector<bool>& xs) {
  return map([](const auto& x) { return renc<m>(x); }, xs);
}

template std::vector<Garbled::Bit<Mode::G>> renc(const std::vector<bool>&);
template std::vector<Garbled::Bit<Mode::E>> renc(const std::vector<bool>&);
template std::vector<Garbled::Bit<Mode::S>> renc(const std::vector<bool>&);


template <Mode m>
bool dec(const Garbled::Bit<m>& x) {
  const auto& [gx, ex] = x;
  GEN { sendbit(gx.lsb()); return 0; }
  EVAL { const auto gcolor = recvbit(); return ex.lsb() ^ gcolor; }
  SPEC { ++scost.bits; return 0; }
}

template bool dec(const Garbled::Bit<Mode::G>&);
template bool dec(const Garbled::Bit<Mode::E>&);
template bool dec(const Garbled::Bit<Mode::S>&);


template <Mode m>
Garbled::Bit<m> reveal(Garbled::Bit<m> x) {
  const auto col = dec(x);
  auto& [gx, ex] = x;
  GEN { gx.set_lsb(0); }
  EVAL { ex.set_lsb(col); }
  return x;
}

template Garbled::Bit<Mode::G> reveal(Garbled::Bit<Mode::G>);
template Garbled::Bit<Mode::E> reveal(Garbled::Bit<Mode::E>);
template Garbled::Bit<Mode::S> reveal(Garbled::Bit<Mode::S>);


template <Mode m>
std::vector<bool> dec(std::span<const Garbled::Bit<m>> xs) {
  return map([](const auto& x) { return dec<m>(x); }, xs);
}

template std::vector<bool> dec(std::span<const Garbled::Bit<Mode::G>>);
template std::vector<bool> dec(std::span<const Garbled::Bit<Mode::E>>);
template std::vector<bool> dec(std::span<const Garbled::Bit<Mode::S>>);


TEST_CASE("") {
  rc::prop("bit encoding: dec ο renc = id", [](bool x) {
    test_circuit([&]<Mode m> { return dec(renc<m>(x)); }, x);
  });
}

template <Mode m>
Garbled::Bit<m> random_garbling() {
  if constexpr (m == Mode::G) { return { random_label() }; }
  if constexpr (m == Mode::E) { return { 0 }; }
  if constexpr (m == Mode::S) { return { }; }
}

template Garbled::Bit<Mode::G> random_garbling();
template Garbled::Bit<Mode::E> random_garbling();
template Garbled::Bit<Mode::S> random_garbling();


template <Mode m> Garbled::Bit<m> open(const Garbled::Bit<m>& x) {
  const auto [gx, ex] = x;
  GEN { send(gx); return { 0 }; }
  EVAL { const auto ggx = recv(); return { ex ^ ggx }; }
  SPEC { ++scost.rows; return { }; }
}

template Garbled::Bit<Mode::G> open(const Garbled::Bit<Mode::G>&);
template Garbled::Bit<Mode::E> open(const Garbled::Bit<Mode::E>&);
template Garbled::Bit<Mode::S> open(const Garbled::Bit<Mode::S>&);


TEST_CASE("") {
  rc::prop("bit XOR: dec (enc x ^ enc y) = x ^ y", [](bool x, bool y) {
    test_circuit([&]<Mode m> { return dec(renc<m>(x) ^ renc<m>(y)); }, x ^ y);
  });
}


template <Mode m>
Garbled::Bit<m> eand(const Garbled::Bit<m>& x, const Garbled::Bit<m>& y) {
  SPEC {
    ++scost.nonces;
    ++scost.rows;
    return { };
  }

  const auto [gx, ex] = x;
  const auto [gy, ey] = y;

  GEN {
    const auto Hx0 = H(gx);
    const auto Hx1 = H(gx ^ delta());
    const auto gz = Hx0;
    ++gcost.nonces;

    send(Hx1 ^ gz ^ gy);

    return { gz };
  }
  EVAL {
    const auto Hx = H(ex);
    ++ecost.nonces;

    const auto row = recv();
    const auto ez = Hx ^ ((row ^ ey) * ex.lsb());

    return { ez };
  }
}

template Garbled::Bit<Mode::G> eand(const Garbled::Bit<Mode::G>&, const Garbled::Bit<Mode::G>&);
template Garbled::Bit<Mode::E> eand(const Garbled::Bit<Mode::E>&, const Garbled::Bit<Mode::E>&);
template Garbled::Bit<Mode::S> eand(const Garbled::Bit<Mode::S>&, const Garbled::Bit<Mode::S>&);

TEST_CASE("") {
  rc::prop("E AND correct", [](bool x, bool y) {
    test_circuit([&]<Mode m> {
      return dec(eand(reveal(renc<m>(x)), renc<m>(y)));
    }, x & y);
  });
}


template <Mode m> Garbled::Bit<m> gand(bool x, const Garbled::Bit<m>& y) {
  const auto Y_ = color(y);
  const auto xY_ = x & Y_;
  const auto Y = ginput<m>(Y_);
  return eand(y ^ Y, ginput<m>(x)) ^ ginput<m>(xY_);
}

template Garbled::Bit<Mode::G> gand(bool, const Garbled::Bit<Mode::G>&);
template Garbled::Bit<Mode::E> gand(bool, const Garbled::Bit<Mode::E>&);
template Garbled::Bit<Mode::S> gand(bool, const Garbled::Bit<Mode::S>&);

TEST_CASE("") {
  rc::prop("G AND correct", [](bool x, bool y) {
    test_circuit([&]<Mode m> {
      return dec(gand(x, renc<m>(y)));
    }, x & y);
  });
}

template <Mode m> Garbled::Bit<m> gscale(const Label& x, const Garbled::Bit<m>& y) {
  const auto Y_ = color(y);
  const auto xY_ = x * Y_;
  const auto Y = ginput<m>(Y_);
  return eand(y ^ Y, ginput<m>(x)) ^ ginput<m>(xY_);
}

template Garbled::Bit<Mode::G> gscale(const Label&, const Garbled::Bit<Mode::G>&);
template Garbled::Bit<Mode::E> gscale(const Label&, const Garbled::Bit<Mode::E>&);
template Garbled::Bit<Mode::S> gscale(const Label&, const Garbled::Bit<Mode::S>&);


template <Mode m> Garbled::Bit<m> swap_lang(const Label& x, const Garbled::Bit<m>& y) {
  const auto X = ginput<m>(x);
  const auto Z = open(X ^ y);
  if constexpr (m == Mode::G) {
    return { x };
  } else {
    return Z;
  }
}

template Garbled::Bit<Mode::G> swap_lang(const Label&, const Garbled::Bit<Mode::G>&);
template Garbled::Bit<Mode::E> swap_lang(const Label&, const Garbled::Bit<Mode::E>&);
template Garbled::Bit<Mode::S> swap_lang(const Label&, const Garbled::Bit<Mode::S>&);


template <Mode m> Garbled::Bit<m> eand_full(const Garbled::Bit<m>& x, const Garbled::Bit<m>& y) {
  auto [gz, ez] = eand(x, y);
  const auto colx = color(x);
  const auto coly = color(y);

  GEN { gz.set_lsb(0); }
  EVAL { ez.set_lsb(colx && coly); }
  return { gz, ez };
}

template Garbled::Bit<Mode::G> eand_full(const Garbled::Bit<Mode::G>&, const Garbled::Bit<Mode::G>&);
template Garbled::Bit<Mode::E> eand_full(const Garbled::Bit<Mode::E>&, const Garbled::Bit<Mode::E>&);
template Garbled::Bit<Mode::S> eand_full(const Garbled::Bit<Mode::S>&, const Garbled::Bit<Mode::S>&);


template <Mode m>
Garbled::Bit<m> operator&(const Garbled::Bit<m>& x, const Garbled::Bit<m>& y) {
  const auto X_ = color(x);
  const auto Y_ = color(y);
  const auto XY_ = X_ & Y_;

  const auto X = ginput<m>(X_);
  const auto Y = ginput<m>(Y_);
  const auto XY = ginput<m>(XY_);
  return eand<m>(x ^ X, y) ^ eand<m>(y ^ Y, X) ^ XY;
}

template Garbled::Bit<Mode::G> operator&(const Garbled::Bit<Mode::G>&, const Garbled::Bit<Mode::G>&);
template Garbled::Bit<Mode::E> operator&(const Garbled::Bit<Mode::E>&, const Garbled::Bit<Mode::E>&);
template Garbled::Bit<Mode::S> operator&(const Garbled::Bit<Mode::S>&, const Garbled::Bit<Mode::S>&);

TEST_CASE("") {
  rc::prop("AND correct", [](bool x, bool y) {
    test_circuit([&]<Mode m> { return dec(renc<m>(x) & renc<m>(y)); }, x & y);
  });
}
