#include <byte.h>
#include <resource.h>
#include <util.h>

template <Mode m>
Garbled::Byte<m> constant(std::byte b) {
  GEN {
    const WLabel ones = {{ 1,1,1,1,1,1,1,1 }};
    return (deltas() ^ ones)  * b;
  }
  EVAL {
    int bb = static_cast<int>(b);
    const WLabel cols = { std::array<Label, 8> {
      (bb & 0x80) > 0,
      (bb & 0x40) > 0,
      (bb & 0x20) > 0,
      (bb & 0x10) > 0,
      (bb & 0x08) > 0,
      (bb & 0x04) > 0,
      (bb & 0x02) > 0,
      (bb & 0x01) > 0 }};
    return cols;
  }
  SPEC { return { }; }
}

template Garbled::Byte<Mode::G> constant(std::byte);
template Garbled::Byte<Mode::E> constant(std::byte);
template Garbled::Byte<Mode::S> constant(std::byte);


template <Mode m>
Garbled::Byte<m> ginput(std::byte b) {
  GEN { return deltas() * b; }
  EVAL { return { }; }
  SPEC { return { }; }
}

template Garbled::Byte<Mode::G> ginput(std::byte);
template Garbled::Byte<Mode::E> ginput(std::byte);
template Garbled::Byte<Mode::S> ginput(std::byte);


template <Mode m>
Garbled::Byte<m> ginput(const WLabel& x) {
  GEN { return x; }
  EVAL { return { }; }
  SPEC { return { }; }
}

template Garbled::Byte<Mode::G> ginput(const WLabel& x);
template Garbled::Byte<Mode::E> ginput(const WLabel& x);
template Garbled::Byte<Mode::S> ginput(const WLabel& x);


template <Mode m>
std::byte dec(const Garbled::Byte<m>& x) {
  const auto [gx, ex] = x;
  GEN { sendbyte(gx.lsb()); return std::byte { 0 }; }
  EVAL { const auto gcolor = recvbyte(); return ex.lsb() ^ gcolor; }
  SPEC { ++scost.bytes; return std::byte { 0 }; }
}

template std::byte dec(const Garbled::Byte<Mode::G>&);
template std::byte dec(const Garbled::Byte<Mode::E>&);
template std::byte dec(const Garbled::Byte<Mode::S>&);


template <Mode m>
Garbled::Byte<m> reveal(Garbled::Byte<m> x) {
  const auto col = dec(x);
  auto& [gx, ex] = x;
  GEN { gx.set_lsb(std::byte { 0 }); }
  EVAL { ex.set_lsb(col); }
  return x;
}

template Garbled::Byte<Mode::G> reveal(Garbled::Byte<Mode::G>);
template Garbled::Byte<Mode::E> reveal(Garbled::Byte<Mode::E>);
template Garbled::Byte<Mode::S> reveal(Garbled::Byte<Mode::S>);


template <Mode m>
Garbled::Byte<m> renc(std::byte b) {
  GEN {
    const auto g = random_wlabel();
    wsend(g ^ (deltas() * b));
    return { g };
  }
  EVAL { return { wrecv() }; }
  SPEC { ++scost.wrows; return { }; }
}

template Garbled::Byte<Mode::G> renc(std::byte);
template Garbled::Byte<Mode::E> renc(std::byte);
template Garbled::Byte<Mode::S> renc(std::byte);


template <Mode m>
std::vector<Garbled::Byte<m>> renc(std::span<const std::byte> xs) {
  return map([](const auto& x) { return renc<m>(x); }, xs);
}

template std::vector<Garbled::Byte<Mode::G>> renc(std::span<const std::byte>);
template std::vector<Garbled::Byte<Mode::E>> renc(std::span<const std::byte>);
template std::vector<Garbled::Byte<Mode::S>> renc(std::span<const std::byte>);


TEST_CASE("") {
  rc::prop("byte encoding: dec ο renc = id", [](std::byte x) {
    test_circuit([&]<Mode m> { return dec(renc<m>(x)); }, x);
  });
}


template <Mode m>
Garbled::Byte<m> random_wide_garbling() {
  if constexpr (m == Mode::G) { return { random_wlabel() }; }
  if constexpr (m == Mode::E) { return { 0 }; }
  if constexpr (m == Mode::S) { return { }; }
}

template Garbled::Byte<Mode::G> random_wide_garbling();
template Garbled::Byte<Mode::E> random_wide_garbling();
template Garbled::Byte<Mode::S> random_wide_garbling();


template <Mode m> Garbled::Byte<m> open(const Garbled::Byte<m>& x) {
  const auto [gx, ex] = x;
  GEN { wsend(gx); return { 0 }; }
  EVAL { const auto ggx = wrecv(); return { ex ^ ggx }; }
  SPEC { ++scost.wrows; return { }; }
}

template Garbled::Byte<Mode::G> open(const Garbled::Byte<Mode::G>&);
template Garbled::Byte<Mode::E> open(const Garbled::Byte<Mode::E>&);
template Garbled::Byte<Mode::S> open(const Garbled::Byte<Mode::S>&);


TEST_CASE("") {
  rc::prop("byte XOR: dec (enc x ^ enc y) = x ^ y", [](std::byte x, std::byte y) {
    test_circuit([&]<Mode m> { return dec(renc<m>(x) ^ renc<m>(y)); }, x ^ y);
  });
}


template <Mode m>
Garbled::Byte<m> eand(const Garbled::Byte<m>& x, const Garbled::Byte<m>& y) {
  SPEC {
    scost.nonces += 8;
    ++scost.wrows;
    return { };
  }

  const auto & [gx, ex] = x;
  const auto & [gy, ey] = y;

  GEN {
    const auto Hx0 = H(gx);
    const auto Hx1 = H(gx ^ deltas());
    const auto gz = Hx0;
    gcost.nonces += 8;

    wsend(Hx1 ^ gz ^ gy);

    return { gz };
  }
  EVAL {
    const auto Hx = H(ex);
    ecost.nonces += 8;

    const auto row = wrecv();
    const auto ez = Hx ^ ((row ^ ey) * ex.lsb());

    return { ez };
  }
}

template Garbled::Byte<Mode::G> eand(const Garbled::Byte<Mode::G>&, const Garbled::Byte<Mode::G>&);
template Garbled::Byte<Mode::E> eand(const Garbled::Byte<Mode::E>&, const Garbled::Byte<Mode::E>&);
template Garbled::Byte<Mode::S> eand(const Garbled::Byte<Mode::S>&, const Garbled::Byte<Mode::S>&);

TEST_CASE("") {
  rc::prop("byte E AND correct", [](std::byte x, std::byte y) {
    test_circuit([&]<Mode m> {
      return dec(eand(reveal(renc<m>(x)), renc<m>(y)));
    }, (x & y));
  });
}


template <Mode m> Garbled::Byte<m> gand(std::byte x, const Garbled::Byte<m>& y) {
  const auto Y_ = color(y);
  const auto xY_ = x & Y_;
  const auto Y = ginput<m>(Y_);
  return eand(y ^ Y, ginput<m>(x)) ^ ginput<m>(xY_);
}

template Garbled::Byte<Mode::G> gand(std::byte, const Garbled::Byte<Mode::G>&);
template Garbled::Byte<Mode::E> gand(std::byte, const Garbled::Byte<Mode::E>&);
template Garbled::Byte<Mode::S> gand(std::byte, const Garbled::Byte<Mode::S>&);

TEST_CASE("") {
  rc::prop("byte G AND correct", [](std::byte x, std::byte y) {
    test_circuit([&]<Mode m> {
      return dec(gand(x, renc<m>(y)));
    }, x & y);
  });
}

template <Mode m> Garbled::Byte<m> gscale(const WLabel& x, const Garbled::Byte<m>& y) {
  const auto Y_ = color(y);
  const auto xY_ = x * Y_;
  const auto Y = ginput<m>(Y_);
  return eand(y ^ Y, ginput<m>(x)) ^ ginput<m>(xY_);
}

template Garbled::Byte<Mode::G> gscale(const WLabel&, const Garbled::Byte<Mode::G>&);
template Garbled::Byte<Mode::E> gscale(const WLabel&, const Garbled::Byte<Mode::E>&);
template Garbled::Byte<Mode::S> gscale(const WLabel&, const Garbled::Byte<Mode::S>&);

template <Mode m> Garbled::Byte<m> swap_lang(const WLabel& x, const Garbled::Byte<m>& y) {
  const auto X = ginput<m>(x);
  const auto Z = open(X ^ y);
  if constexpr (m == Mode::G) {
    return { x };
  } else {
    return Z;
  }
}

template Garbled::Byte<Mode::G> swap_lang(const WLabel&, const Garbled::Byte<Mode::G>&);
template Garbled::Byte<Mode::E> swap_lang(const WLabel&, const Garbled::Byte<Mode::E>&);
template Garbled::Byte<Mode::S> swap_lang(const WLabel&, const Garbled::Byte<Mode::S>&);


template <Mode m>
Garbled::Byte<m> operator&(const Garbled::Byte<m>& x, const Garbled::Byte<m>& y) {
  const auto X_ = color(x);
  const auto Y_ = color(y);
  const auto XY_ = X_ & Y_;

  const auto X = ginput<m>(X_);
  const auto Y = ginput<m>(Y_);
  return eand<m>(x ^ X, y) ^ eand<m>(y ^ Y, X) ^ ginput<m>(XY_);
}

template Garbled::Byte<Mode::G> operator&(const Garbled::Byte<Mode::G>&, const Garbled::Byte<Mode::G>&);
template Garbled::Byte<Mode::E> operator&(const Garbled::Byte<Mode::E>&, const Garbled::Byte<Mode::E>&);
template Garbled::Byte<Mode::S> operator&(const Garbled::Byte<Mode::S>&, const Garbled::Byte<Mode::S>&);

TEST_CASE("") {
  rc::prop("wide AND correct", [](std::byte x, std::byte y) {
    test_circuit([&]<Mode m> { return dec(renc<m>(x) & renc<m>(y)); },  (x & y));
  });
}
