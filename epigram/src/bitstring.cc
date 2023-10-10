#include <bitstring.h>
#include <resource.h>
#include <tuple>


inline std::pair<std::span<const WLabel>, std::span<const Label>>
split(std::span<const Label> xs) {
  const auto n = xs.size();
  const auto nbytes = 0; //  n / 8;

  const auto tail = xs.subspan(nbytes*8);

  const auto head = std::span {
    reinterpret_cast<const WLabel*>(xs.data()),
    nbytes
  };

  return { head, tail };
}


inline std::pair<std::span<WLabel>, std::span<Label>>
split(std::span<Label> xs) {
  const auto n = xs.size();
  const auto nbytes = 0; // n / 8;

  const auto tail = xs.subspan(nbytes*8);

  const auto head = std::span {
    reinterpret_cast<WLabel*>(xs.data()),
    nbytes
  };

  return { head, tail };
}


template <Mode m>
inline
std::pair<std::span<Garbled::Byte<m>>, std::span<Garbled::Bit<m>>>
split(std::span<Garbled::Bit<m>> xs) {
  const auto n = xs.size();
  const auto nbytes = 0; // n / 8;

  //std::cout << "CAOAOAOAO " << xs[0] << std::endl;

  const auto tail = xs.subspan(nbytes*8);

  const auto head = std::span {
    reinterpret_cast<Garbled::Byte<m>*>(xs.data()),
    nbytes
  };

  return { head, tail };
}


template <Mode m>
inline
std::pair<std::span<const Garbled::Byte<m>>, std::span<const Garbled::Bit<m>>>
split(std::span<const Garbled::Bit<m>> xs) {
  const auto n = xs.size();
  const auto nbytes = 0; // n / 8;

  const auto tail = xs.subspan(nbytes*8);

  const auto head = std::span {
    reinterpret_cast<const Garbled::Byte<m>*>(xs.data()),
    nbytes
  };

  return { head, tail };
}


template <Mode m>
std::span<Garbled::Bit<m>> operator&=(
    std::span<Garbled::Bit<m>> xs,
    std::span<const Garbled::Bit<m>> ys) {

  assert(xs.size() == ys.size());

  Garbled::Byte<m> tmpx;
  Garbled::Byte<m> tmpy;
  int i = 0;
  for ( ; i + 8 <= xs.size(); i += 8) {
    //std::cout << sizeof(WLabel) << std::endl;
    std::memcpy(&tmpx, xs.data()+i, sizeof(tmpx));
    std::memcpy(&tmpy, ys.data()+i, sizeof(tmpy));
    tmpx &= tmpy;
    std::memcpy(xs.data()+i, &tmpx, sizeof(tmpx));
  }
  for ( ; i < xs.size(); i++) xs[i] &= ys[i];
  return xs;
  
  /*
  const auto [xs0, xs1] = split(xs);
  const auto [ys0, ys1] = split(ys);

  for (std::size_t i = 0; i < xs0.size(); ++i) { xs0[i] &= ys0[i]; }
  for (std::size_t i = 0; i < xs1.size(); ++i) { xs1[i] &= ys1[i]; }

  return xs;
  */
}


template std::span<Garbled::Bit<Mode::G>> operator&=(
    std::span<Garbled::Bit<Mode::G>>, std::span<const Garbled::Bit<Mode::G>>);
template std::span<Garbled::Bit<Mode::E>> operator&=(
    std::span<Garbled::Bit<Mode::E>>, std::span<const Garbled::Bit<Mode::E>>);
template std::span<Garbled::Bit<Mode::S>> operator&=(
    std::span<Garbled::Bit<Mode::S>>, std::span<const Garbled::Bit<Mode::S>>);


TEST_CASE("bitsand") {
  rc::prop("bitstring and", [](std::array<bool, 21> xarr, std::array<bool, 21> yarr) {
    std::vector<bool> xs, ys;
    std::copy(xarr.begin(), xarr.end(), std::back_inserter(xs));
    std::copy(yarr.begin(), yarr.end(), std::back_inserter(ys));

    std::vector<bool> expected;
    for (std::size_t i = 0; i < xs.size(); ++i) {
      expected.emplace_back(xs[i] & ys[i]);
    }

    test_circuit([&]<Mode m> {
      auto xxs = renc<m>(xs);
      const auto yys = renc<m>(ys);

      xxs &= yys;
      return dec(std::span<const Garbled::Bit<m>> { xxs });
    }, expected);
  });
}


template <Mode m>
std::span<Garbled::Bit<m>> operator&=(
    std::span<Garbled::Bit<m>> xs, const Garbled::Bit<m>& y) {

  const auto yy = broadcast<m>(y);
  Garbled::Byte<m> tmpx;
  int i = 0;
  for ( ; i + 8 <= xs.size(); i += 8) {
    //std::cout << sizeof(WLabel) << std::endl;
    std::memcpy(&tmpx, xs.data()+i, sizeof(tmpx));
    tmpx &= yy;
    std::memcpy(xs.data()+i, &tmpx, sizeof(tmpx));
  }
  for ( ; i < xs.size(); i++) xs[i] &= y;
  return xs;

  /*
  const auto [xs0, xs1] = split(xs);


  for (auto& x : xs0) { x &= yy; }
  for (auto& x : xs1) { x &= y; }

  return xs;
  */
}

template std::span<Garbled::Bit<Mode::G>> operator&=(
    std::span<Garbled::Bit<Mode::G>>, const Garbled::Bit<Mode::G>&);
template std::span<Garbled::Bit<Mode::E>> operator&=(
    std::span<Garbled::Bit<Mode::E>>, const Garbled::Bit<Mode::E>&);
template std::span<Garbled::Bit<Mode::S>> operator&=(
    std::span<Garbled::Bit<Mode::S>>, const Garbled::Bit<Mode::S>&);


TEST_CASE("") {
  rc::prop("bitstring/scalar and", [](std::array<bool, 21> xs, bool y) {
    std::vector<bool> bits;
    std::copy(xs.begin(), xs.end(), std::back_inserter(bits));

    std::vector<bool> expected;
    for (auto x : xs) { expected.emplace_back(x & y); }

    test_circuit([&]<Mode m> {
      auto xxs = renc<m>(bits);
      const auto yy = renc<m>(y);

      xxs &= yy;
      return dec(std::span<const Garbled::Bit<m>> { xxs });
    }, expected);
  });
}



template <Mode m>
std::span<Garbled::Bit<m>> operator^=(
    std::span<Garbled::Bit<m>> xs,
    std::span<const Garbled::Bit<m>> ys) {

  assert(xs.size() == ys.size());

  Garbled::Byte<m> tmpx;
  Garbled::Byte<m> tmpy;
  int i = 0;
  for ( ; i + 8 <= xs.size(); i += 8) {
    //std::cout << sizeof(WLabel) << std::endl;
    std::memcpy(&tmpx, xs.data()+i, sizeof(tmpx));
    std::memcpy(&tmpy, ys.data()+i, sizeof(tmpy));
    tmpx ^= tmpy;
    std::memcpy(xs.data()+i, &tmpx, sizeof(tmpx));
  }
  for ( ; i < xs.size(); i++) xs[i] ^= ys[i];
  return xs;

  /*
  const auto [xs0, xs1] = split(xs);
  const auto [ys0, ys1] = split(ys);

  for (std::size_t i = 0; i < xs0.size(); ++i) { xs0[i] ^= ys0[i]; }
  for (std::size_t i = 0; i < xs1.size(); ++i) { xs1[i] ^= ys1[i]; }

  return xs;
  */
}


template std::span<Garbled::Bit<Mode::G>> operator^=(
    std::span<Garbled::Bit<Mode::G>>, std::span<const Garbled::Bit<Mode::G>>);
template std::span<Garbled::Bit<Mode::E>> operator^=(
    std::span<Garbled::Bit<Mode::E>>, std::span<const Garbled::Bit<Mode::E>>);
template std::span<Garbled::Bit<Mode::S>> operator^=(
    std::span<Garbled::Bit<Mode::S>>, std::span<const Garbled::Bit<Mode::S>>);


TEST_CASE("bitsxor") {
  rc::prop("bitstring xor", [](std::array<bool, 8> xarr, std::array<bool, 8> yarr) {
    std::vector<bool> xs, ys;
    std::copy(xarr.begin(), xarr.end(), std::back_inserter(xs));
    std::copy(yarr.begin(), yarr.end(), std::back_inserter(ys));

    std::vector<bool> expected;
    for (std::size_t i = 0; i < xs.size(); ++i) {
      expected.emplace_back(xs[i] ^ ys[i]);
    }

    test_circuit([&]<Mode m> {
      auto xxs = renc<m>(xs);
      const auto yys = renc<m>(ys);

      xxs ^= yys;
      return dec(std::span<const Garbled::Bit<m>> { xxs });
    }, expected);
  });
}


template <Mode m>
std::span<Garbled::Bit<m>> operator^=(
    std::span<Garbled::Bit<m>> xs, const Garbled::Bit<m>& y) {

  const auto yy = broadcast<m>(y);
  Garbled::Byte<m> tmpx;
  int i = 0;
  for ( ; i + 8 <= xs.size(); i += 8) {
    //std::cout << sizeof(WLabel) << std::endl;
    std::memcpy(&tmpx, xs.data()+i, sizeof(tmpx));
    tmpx ^= yy;
    std::memcpy(xs.data()+i, &tmpx, sizeof(tmpx));
  }
  for ( ; i < xs.size(); i++) xs[i] ^= y;
  return xs;

  /*
  const auto [xs0, xs1] = split(xs);
  const auto yy = broadcast<m>(y);

  for (auto& x : xs0) { x ^= yy; }
  for (auto& x : xs1) { x ^= y; }

  return xs;
  */
}

template std::span<Garbled::Bit<Mode::G>> operator^=(
    std::span<Garbled::Bit<Mode::G>>, const Garbled::Bit<Mode::G>&);
template std::span<Garbled::Bit<Mode::E>> operator^=(
    std::span<Garbled::Bit<Mode::E>>, const Garbled::Bit<Mode::E>&);
template std::span<Garbled::Bit<Mode::S>> operator^=(
    std::span<Garbled::Bit<Mode::S>>, const Garbled::Bit<Mode::S>&);


TEST_CASE("") {
  rc::prop("bitstring/scalar xor", [](std::array<bool, 21> xs, bool y) {
    std::vector<bool> bits;
    std::copy(xs.begin(), xs.end(), std::back_inserter(bits));

    std::vector<bool> expected;
    for (auto x : xs) { expected.emplace_back(x ^ y); }

    test_circuit([&]<Mode m> {
      auto xxs = renc<m>(bits);
      const auto yy = renc<m>(y);

      xxs ^= yy;
      return dec(std::span<const Garbled::Bit<m>> { xxs });
    }, expected);
  });
}


std::vector<Label> operator^(std::span<const Label> xs, std::span<const Label> ys) {
  assert(xs.size() == ys.size());
  std::vector<Label> zs(xs.size());

  WLabel tmpx;
  WLabel tmpy;
  int i = 0;
  for ( ; i + 8 <= xs.size(); i += 8) {
    //std::cout << sizeof(WLabel) << std::endl;
    std::memcpy(&tmpx, xs.data()+i, sizeof(tmpx));
    std::memcpy(&tmpy, ys.data()+i, sizeof(tmpy));
    tmpx ^= tmpy;
    std::memcpy(zs.data()+i, &tmpx, sizeof(tmpx));
  }
  for ( ; i < xs.size(); i++) zs[i] = xs[i] ^ ys[i];

  return zs;
  /*

  const auto [xs0, xs1] = split(xs);
  const auto [ys0, ys1] = split(ys);
  const auto [zs0, zs1] = split(std::span { zs });

  for (std::size_t i = 0; i < xs0.size(); ++i) { zs0[i] = xs0[i] ^ ys0[i]; }
  for (std::size_t i = 0; i < xs1.size(); ++i) { zs1[i] = xs1[i] ^ ys1[i]; }
  return zs;
  */
}


template <Mode m>
void swap(
    const Garbled::Bit<m>& s,
    std::span<Garbled::Bit<m>> xs,
    std::span<Garbled::Bit<m>> ys) {

  assert(xs.size() == ys.size());

  const auto ss = broadcast<m>(s);

  Garbled::Byte<m> tmpx;
  Garbled::Byte<m> tmpy;
  int i = 0;
  for ( ; i + 8 <= xs.size(); i += 8) {
    //std::cout << sizeof(WLabel) << std::endl;
    std::memcpy(&tmpx, xs.data()+i, sizeof(tmpx));
    std::memcpy(&tmpy, ys.data()+i, sizeof(tmpy));
    const auto diff = ss & (tmpx ^ tmpy);
    tmpx ^= diff;
    tmpy ^= diff;
    std::memcpy(xs.data()+i, &tmpx, sizeof(tmpx));
    std::memcpy(ys.data()+i, &tmpy, sizeof(tmpy));
  }
  for ( ; i < xs.size(); i++) {
    const auto diff = s & (xs[i] ^ ys[i]);
    xs[i] ^= diff;
    ys[i] ^= diff;
  }

  /*
  const auto [xs0, xs1] = split(xs);
  const auto [ys0, ys1] = split(ys);

  const auto ss = broadcast<m>(s);

  for (std::size_t i = 0; i < xs0.size(); ++i) {
    const auto diff = ss & (xs0[i] ^ ys0[i]);
    xs0[i] ^= diff;
    ys0[i] ^= diff;
  }
  for (std::size_t i = 0; i < xs1.size(); ++i) {
    const auto diff = s & (xs1[i] ^ ys1[i]);
    xs1[i] ^= diff;
    ys1[i] ^= diff;
  }
  */
}


template void swap(
    const Garbled::Bit<Mode::G>&, std::span<Garbled::Bit<Mode::G>>, std::span<Garbled::Bit<Mode::G>>);
template void swap(
    const Garbled::Bit<Mode::E>&, std::span<Garbled::Bit<Mode::E>>, std::span<Garbled::Bit<Mode::E>>);
template void swap(
    const Garbled::Bit<Mode::S>&, std::span<Garbled::Bit<Mode::S>>, std::span<Garbled::Bit<Mode::S>>);


template <Mode m>
void eswap(
    const Garbled::Bit<m>& s,
    std::span<Garbled::Bit<m>> xs,
    std::span<Garbled::Bit<m>> ys) {

  assert(xs.size() == ys.size());


  const auto ss = broadcast<m>(s);

  Garbled::Byte<m> tmpx;
  Garbled::Byte<m> tmpy;
  int i = 0;
  for ( ; i + 8 <= xs.size(); i += 8) {
    //std::cout << sizeof(WLabel) << std::endl;
    std::memcpy(&tmpx, xs.data()+i, sizeof(tmpx));
    std::memcpy(&tmpy, ys.data()+i, sizeof(tmpy));
    const auto diff = eand(ss, tmpx ^ tmpy);
    tmpx ^= diff;
    tmpy ^= diff;
    std::memcpy(xs.data()+i, &tmpx, sizeof(tmpx));
    std::memcpy(ys.data()+i, &tmpy, sizeof(tmpy));
  }
  for ( ; i < xs.size(); i++) {
    const auto diff = eand(s, xs[i] ^ ys[i]);
    xs[i] ^= diff;
    ys[i] ^= diff;
  }  

  /*
  const auto [xs0, xs1] = split(xs);
  const auto [ys0, ys1] = split(ys);

  const auto ss = broadcast<m>(s);

  for (std::size_t i = 0; i < xs0.size(); ++i) {
    const auto diff = eand(ss, xs0[i] ^ ys0[i]);
    xs0[i] ^= diff;
    ys0[i] ^= diff;
  }
  for (std::size_t i = 0; i < xs1.size(); ++i) {
    const auto diff = eand(s, xs1[i] ^ ys1[i]);
    xs1[i] ^= diff;
    ys1[i] ^= diff;
  }
  */
}


template void eswap(
    const Garbled::Bit<Mode::G>&, std::span<Garbled::Bit<Mode::G>>, std::span<Garbled::Bit<Mode::G>>);
template void eswap(
    const Garbled::Bit<Mode::E>&, std::span<Garbled::Bit<Mode::E>>, std::span<Garbled::Bit<Mode::E>>);
template void eswap(
    const Garbled::Bit<Mode::S>&, std::span<Garbled::Bit<Mode::S>>, std::span<Garbled::Bit<Mode::S>>);


template <Mode m>
void gswap(
    bool s,
    std::span<Garbled::Bit<m>> xs,
    std::span<Garbled::Bit<m>> ys) {

  assert(xs.size() == ys.size());

  const std::uint8_t lhs = 0xFF + !s;
  const auto ss = static_cast<std::byte>(lhs);

  Garbled::Byte<m> tmpx;
  Garbled::Byte<m> tmpy;
  int i = 0;
  for ( ; i + 8 <= xs.size(); i += 8) {
    //std::cout << sizeof(WLabel) << std::endl;
    std::memcpy(&tmpx, xs.data()+i, sizeof(tmpx));
    std::memcpy(&tmpy, ys.data()+i, sizeof(tmpy));
    const auto diff = gand(ss, tmpx ^ tmpy);
    tmpx ^= diff;
    tmpy ^= diff;
    std::memcpy(xs.data()+i, &tmpx, sizeof(tmpx));
    std::memcpy(ys.data()+i, &tmpy, sizeof(tmpy));
  }
  for ( ; i < xs.size(); i++) {
    const auto diff = gand(s, xs[i] ^ ys[i]);
    xs[i] ^= diff;
    ys[i] ^= diff;
  }  

  /*
  const auto [xs0, xs1] = split(xs);
  const auto [ys0, ys1] = split(ys);

  const std::uint8_t lhs = 0xFF + !s;
  const auto ss = static_cast<std::byte>(lhs);

  for (std::size_t i = 0; i < xs0.size(); ++i) {
    const auto diff = gand(ss, xs0[i] ^ ys0[i]);
    xs0[i] ^= diff;
    ys0[i] ^= diff;
  }
  for (std::size_t i = 0; i < xs1.size(); ++i) {
    const auto diff = gand(s, xs1[i] ^ ys1[i]);
    xs1[i] ^= diff;
    ys1[i] ^= diff;
  }
  */
}


template void gswap(
    bool, std::span<Garbled::Bit<Mode::G>>, std::span<Garbled::Bit<Mode::G>>);
template void gswap(
    bool, std::span<Garbled::Bit<Mode::E>>, std::span<Garbled::Bit<Mode::E>>);
template void gswap(
    bool, std::span<Garbled::Bit<Mode::S>>, std::span<Garbled::Bit<Mode::S>>);


template <Mode m> void open(std::span<Garbled::Bit<m>> xs) {
  int i = 0;
  Garbled::Byte<m> tmpx;
  for ( ; i + 8 <= xs.size(); i += 8) {
    //std::cout << sizeof(WLabel) << std::endl;
    std::memcpy(&tmpx, xs.data()+i, sizeof(tmpx));
    tmpx = open(tmpx);
    std::memcpy(xs.data()+i, &tmpx, sizeof(tmpx));
  }
  for ( ; i < xs.size(); i++) {
    xs[i] = open(xs[i]);
  }  
  /*
  auto [xs0, xs1] = split(xs);
  for (auto& x: xs0) { x = open(x); }
  for (auto& x: xs1) { x = open(x); }
  */
}

template void open(std::span<Garbled::Bit<Mode::G>>);
template void open(std::span<Garbled::Bit<Mode::E>>);
template void open(std::span<Garbled::Bit<Mode::S>>);


template <Mode m>
std::vector<Garbled::Bit<m>> random_garbled_string(std::size_t n) {
  std::vector<Garbled::Bit<m>> xs(n);
  int i = 0;
  Garbled::Byte<m> tmpx;
  for ( ; i + 8 <= xs.size(); i += 8) {
    tmpx = random_wide_garbling<m>();
    std::memcpy(xs.data()+i, &tmpx, sizeof(tmpx));
  }
  for ( ; i < xs.size(); i++) {
    xs[i] = random_garbling<m>();
  }  
  return xs;  
  /*  
  auto [xs0, xs1] = split(std::span<Garbled::Bit<m>> { xs });
  for (auto& x: xs0) { x = random_wide_garbling<m>(); }
  for (auto& x: xs1) { x = random_garbling<m>(); }
  return xs;
  */
}

template std::vector<Garbled::Bit<Mode::G>> random_garbled_string(std::size_t);
template std::vector<Garbled::Bit<Mode::E>> random_garbled_string(std::size_t);
template std::vector<Garbled::Bit<Mode::S>> random_garbled_string(std::size_t);


std::vector<Label> random_labels(std::size_t n) {
  std::vector<Label> xs(n);
  int i = 0;
  WLabel tmpx;
  for ( ; i + 8 <= xs.size(); i += 8) {
    tmpx = random_wlabel();
    std::memcpy(xs.data()+i, &tmpx, sizeof(tmpx));
  }
  for ( ; i < xs.size(); i++) {
    xs[i] = random_label();
  }  
  return xs;
  /*
  auto [xs0, xs1] = split(std::span { xs });
  for (auto& x: xs0) { x = random_wlabel(); }
  for (auto& x: xs1) { x = random_label(); }
  return xs;
  */
}


template <Mode m>
std::vector<Garbled::Bit<m>> gscale(const Garbled::Bit<m>& x, std::span<const Label> ys) {
  std::vector<Garbled::Bit<m>> zs(ys.size());
  const auto xx = broadcast<m>(x);
  int i = 0;
  Garbled::Byte<m> tmpz;
  WLabel tmpy;
  for ( ; i + 8 <= ys.size(); i += 8) {
    std::memcpy(&tmpy, ys.data()+i, sizeof(tmpy));
    tmpz = gscale(tmpy, xx);
    std::memcpy(zs.data()+i, &tmpz, sizeof(tmpz));
  }
  for ( ; i < ys.size(); i++) {
    zs[i] = gscale(ys[i], x);
  }  
  return zs;
  /*
  const auto [ys0, ys1] = split(ys);
  auto [zs0, zs1] = split(std::span { zs });

  const auto xx = broadcast<m>(x);
  for (std::size_t i = 0; i < ys0.size(); ++i) {
    zs0[i] = gscale(ys0[i], xx);
  }
  for (std::size_t i = 0; i < ys1.size(); ++i) {
    zs1[i] = gscale(ys1[i], x);
  }
  return zs;
  */
}

template std::vector<Garbled::Bit<Mode::G>>
gscale(const Garbled::Bit<Mode::G>&, std::span<const Label>);
template std::vector<Garbled::Bit<Mode::E>>
gscale(const Garbled::Bit<Mode::E>&, std::span<const Label>);
template std::vector<Garbled::Bit<Mode::S>>
gscale(const Garbled::Bit<Mode::S>&, std::span<const Label>);


template <Mode m>
void ginput(std::span<const Label> ys, std::span<Garbled::Bit<m>> zs) {
  assert(ys.size() == zs.size());
  int i = 0;
  WLabel tmpy;
  Garbled::Byte<m> tmpz;
  for ( ; i + 8 <= ys.size(); i += 8) {
    std::memcpy(&tmpy, ys.data()+i, sizeof(tmpy));
    tmpz = ginput<m>(tmpy);
    std::memcpy(zs.data()+i, &tmpz, sizeof(tmpz));
  }
  for ( ; i < ys.size(); i++) {
    zs[i] = ginput<m>(ys[i]);
  }  
  /*
  const auto [ys0, ys1] = split(ys);
  auto [zs0, zs1] = split(zs);

  for (std::size_t i = 0; i < ys0.size(); ++i) {
    zs0[i] = ginput<m>(ys0[i]);
  }
  for (std::size_t i = 0; i < ys1.size(); ++i) {
    zs1[i] = ginput<m>(ys1[i]);
  }
  */
}

template void ginput(std::span<const Label>, std::span<Garbled::Bit<Mode::G>>);
template void ginput(std::span<const Label>, std::span<Garbled::Bit<Mode::E>>);
template void ginput(std::span<const Label>, std::span<Garbled::Bit<Mode::S>>);


template <Mode m>
void swap_lang(std::span<const Label> ys, std::span<Garbled::Bit<m>> zs) {
  assert(ys.size() == zs.size());
  int i = 0;
  WLabel tmpy;
  Garbled::Byte<m> tmpz;
  for ( ; i + 8 <= ys.size(); i += 8) {
    std::memcpy(&tmpy, ys.data()+i, sizeof(tmpy));
    std::memcpy(&tmpz, zs.data()+i, sizeof(tmpz));
    tmpz = swap_lang(tmpy, tmpz);
    std::memcpy(zs.data()+i, &tmpz, sizeof(tmpz));
  }
  for ( ; i < ys.size(); i++) {
    zs[i] = swap_lang(ys[i], zs[i]);
  }  
  /*

  const auto [ys0, ys1] = split(ys);
  auto [zs0, zs1] = split(zs);

  for (std::size_t i = 0; i < ys0.size(); ++i) {
    zs0[i] = swap_lang(ys0[i], zs0[i]);
  }
  for (std::size_t i = 0; i < ys1.size(); ++i) {
    zs1[i] = swap_lang(ys1[i], zs1[i]);
  }
  */
}


template void swap_lang(std::span<const Label>, std::span<Garbled::Bit<Mode::G>>);
template void swap_lang(std::span<const Label>, std::span<Garbled::Bit<Mode::E>>);
template void swap_lang(std::span<const Label>, std::span<Garbled::Bit<Mode::S>>);
