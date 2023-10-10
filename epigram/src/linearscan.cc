#include <linearscan.h>
#include <bitstring.h>
#include <util.h>
#include <resource.h>


template <Mode m>
std::vector<Garbled::Bit<m>> read_selection(
    std::size_t w,
    std::span<const Garbled::Bit<m>> selection,
    std::span<const Garbled::Bit<m>> buff) {

  const auto n = selection.size();
  assert(n * w == buff.size());

  std::vector<Garbled::Bit<m>> out(w);
  std::vector<Garbled::Bit<m>> temp(w);

  for (std::size_t i = 0; i < n; ++i) {
    const auto slot = buff.subspan(i*w, w);
    std::copy(slot.begin(), slot.end(), temp.begin());

    temp &= selection[i];
    out ^= temp;
  }
  return out;
}

template std::vector<Garbled::Bit<Mode::G>> read_selection(
    std::size_t,
    std::span<const Garbled::Bit<Mode::G>>,
    std::span<const Garbled::Bit<Mode::G>>);
template std::vector<Garbled::Bit<Mode::E>> read_selection(
    std::size_t,
    std::span<const Garbled::Bit<Mode::E>>,
    std::span<const Garbled::Bit<Mode::E>>);
template std::vector<Garbled::Bit<Mode::S>> read_selection(
    std::size_t,
    std::span<const Garbled::Bit<Mode::S>>,
    std::span<const Garbled::Bit<Mode::S>>);

TEST_CASE("") {
  constexpr std::size_t n = 64;
  constexpr std::size_t w = 4;
  constexpr std::size_t logn = log2(n);
  rc::prop("linear scan read correct", [](std::array<bool, n * w> contentarr, std::array<bool, logn> ixarr) {
    const auto content = bool_arr_to_vec(contentarr);
    const auto ix = bool_arr_to_vec(ixarr);
    std::size_t index = 0;
    for (std::size_t i = 0; i < logn; ++i) { index <<= 1; index ^= ix[logn-i-1]; }
    std::vector<bool> expected;
    for (std::size_t i = 0; i < w; ++i) {
      expected.push_back(content[index*w + i]);
    }

    test_circuit([&]<Mode m> {
      const auto buff = renc<m>(content);
      const auto index = renc<m>(ix);
      const auto out = read_scan(w, std::span { index }, std::span { buff });
      return dec(std::span { out });
    }, expected);
  });
}


template <Mode m>
std::vector<Garbled::Bit<m>> write_selection(
    std::span<const Garbled::Bit<m>> to_write,
    std::span<const Garbled::Bit<m>> selection,
    std::span<Garbled::Bit<m>> buff) {

  const auto n = selection.size();
  const auto w = to_write.size();

  assert(n * w == buff.size());
  assert(to_write.size() == w);

  std::vector<Garbled::Bit<m>> temp(w);
  std::copy(to_write.begin(), to_write.end(), temp.begin());

  for (std::size_t i = 0; i < n; ++i) {
    auto slot = buff.subspan(i*w, w);
    swap(selection[i], slot, std::span { temp });
  }
  return temp;
}

template std::vector<Garbled::Bit<Mode::G>> write_selection(
    std::span<const Garbled::Bit<Mode::G>>,
    std::span<const Garbled::Bit<Mode::G>>,
    std::span<Garbled::Bit<Mode::G>>);
template std::vector<Garbled::Bit<Mode::E>> write_selection(
    std::span<const Garbled::Bit<Mode::E>>,
    std::span<const Garbled::Bit<Mode::E>>,
    std::span<Garbled::Bit<Mode::E>>);
template std::vector<Garbled::Bit<Mode::S>> write_selection(
    std::span<const Garbled::Bit<Mode::S>>,
    std::span<const Garbled::Bit<Mode::S>>,
    std::span<Garbled::Bit<Mode::S>>);


TEST_CASE("") {
  constexpr std::size_t n = 64;
  constexpr std::size_t w = 4;
  constexpr std::size_t logn = log2(n);
  rc::prop("linear scan write correct", [](
        std::array<bool, w> to_writearr,
        std::array<bool, n * w> contentarr,
        std::array<bool, logn> ixarr) {
    const auto to_write = bool_arr_to_vec(to_writearr);
    const auto content = bool_arr_to_vec(contentarr);
    const auto ix = bool_arr_to_vec(ixarr);
    std::size_t index = 0;
    for (std::size_t i = 0; i < logn; ++i) { index <<= 1; index ^= ix[logn-i-1]; }
    std::vector<bool> old;
    for (std::size_t i = 0; i < w; ++i) {
      old.push_back(content[index*w + i]);
    }

    test_circuit([&]<Mode m> {
      const auto write = renc<m>(to_write);
      auto buff = renc<m>(content);
      const auto index = renc<m>(ix);
      const auto before = write_scan(std::span { write }, std::span { index }, std::span { buff });
      const auto after = write_scan(std::span { write }, std::span { index }, std::span { buff });

      return std::pair { dec(std::span { before }), dec(std::span { after }) };

    }, std::pair { old, to_write });
  });
}


// Constructing a one-hot encoding:
//
// The overall goal is to construct a mapping from binary numbers to
// one hot numbers via a Boolean circuit.
// We wish to do so using the lowest possible multiplicative complexity.
//
// The first step is to compute, not one hot, but rather a "full expansion" of
// all combinations of variable products.
//
// For example, given variables a and b, we wish to construct the following
// products:
//
// 0: 1
// 1: a
// 2: ~a
// 3: b
// 4: ab
// 5: ~ab
// 6: ~b
// 7: a~b
// 8: ~a~b
//
// That is, we wish to populate a table where each variable can independently
// (1) not appear, (2) appear in positive position, or (3) appear in negative
// position.
// This table can be iteratively constructed starting from index 0.
// For a table with `n` variables, `2^(n-1)` multiplications are required.
// The remaining rows are computed by linear operations.
// With the full expansion calculated, the one hot expansion of a binary number
// just selects particular rows of this table.

// Because each variable can take on three states, we need a ternary logic
using Trit = std::uint8_t;


std::size_t pow3(std::size_t x) {
  constexpr std::array<std::size_t, 20> pows=
  { 1, 3, 9, 27, 81
  , 243, 729, 2187, 6561, 19683
  , 59049, 177417, 531441, 1594323, 4782969
  , 14348907, 43046721, 129140163, 387420489, 1162261467
  };

  return pows[x];
}


void decompose_trits(std::size_t inp, std::span<Trit> out) {
  for (std::size_t j = out.size(); j > 0; --j) {
    const std::size_t p = pow3(j-1);
    out[j-1] = 0;
    if (inp >= p) { inp -= p; ++out[j-1]; }
    if (inp >= p) { inp -= p; ++out[j-1]; }
  }
};


std::size_t compose_trits(std::span<const Trit> inp) {
  std::size_t p = 1;
  std::size_t out = 0;
  for (auto i : inp) {
    out += i*p;
    p *= 3;
  }
  return out;
}


bool all_zero_one(std::span<const Trit> inp) {
  for (auto i: inp) {
    if (i > 1) { return false; }
  }
  return true;
}


bool singleton(std::span<const Trit> inp, std::size_t& where) {
  bool single = false;
  for (std::size_t i = 0; i < inp.size(); ++i) {
    if (inp[i] > 0 && !single) {
      single = true;
      where = i;
    } else if (inp[i] > 0) {
      where = 0;
      return false;
    }
  }
  return single;
}


std::pair<std::size_t, std::size_t> split_mst(std::size_t n) {
  std::size_t p = 1;
  while (p <= n) { p *= 3; }

  p /= 3;
  const std::size_t top = (n/p) * p;
  const std::size_t rest = n - top;

  return { top, rest };
}


std::size_t top_two(std::span<const Trit> ts) {
  std::size_t out = -1;
  for (std::size_t i = 0; i < ts.size(); ++i) {
    if (ts[i] == 2) { out = i; }
  }
  return out;
}


template <Mode mode>
void expansions(std::span<const Garbled::Bit<mode>> inp, std::span<Garbled::Bit<mode>> out) {
  const std::size_t n = inp.size();
  const std::size_t m = pow3(n);

  assert(out.size() == m);

  std::vector<Trit> trits(n);
  out[0] = constant<mode>(1);
  for (std::size_t i = 1; i < m; ++i) {
    decompose_trits(i, trits);

    std::size_t where = 0;
    if (all_zero_one(trits)) {
      if (singleton(trits, where)) {
        out[i] = inp[where];
      } else {
        const auto [t, r] = split_mst(i);
        out[i] = out[t] & out[r];
      }
    } else {
      const auto tt = pow3(top_two(trits));
      out[i] = out[i - tt] ^ out[i - 2*tt];
    }
  }
}


template <Mode mode>
void one_hot(std::span<const Garbled::Bit<mode>> inp, std::span<Garbled::Bit<mode>> out) {
  const std::size_t n = inp.size();
  const std::size_t m = 1 << n;
  assert(out.size() == m);

  std::vector<Garbled::Bit<mode>> exps(pow3(inp.size()));
  expansions<mode>(inp, exps);

  std::vector<Trit> trits(n);
  for (std::size_t i = 0; i < m; ++i) {
    for (std::size_t j = 0; j < n; ++j) {
      trits[j] = 2 - ((i & (1 << j)) > 0);
    }
    out[i] = exps[compose_trits(trits)];
  }
}

template void one_hot(std::span<const Garbled::Bit<Mode::G>>, std::span<Garbled::Bit<Mode::G>>);
template void one_hot(std::span<const Garbled::Bit<Mode::E>>, std::span<Garbled::Bit<Mode::E>>);
template void one_hot(std::span<const Garbled::Bit<Mode::S>>, std::span<Garbled::Bit<Mode::S>>);

TEST_CASE("") {
  const auto spec = [](bool x, bool y, bool z) {
    std::vector<bool> out(8);
    out[0] = !x & !y & !z;
    out[1] =  x & !y & !z;
    out[2] = !x &  y & !z;
    out[3] =  x &  y & !z;
    out[4] = !x & !y &  z;
    out[5] =  x & !y &  z;
    out[6] = !x &  y &  z;
    out[7] =  x &  y &  z;
    return out;
  };

  rc::prop("One hot expansion correct", [&] (bool x, bool y, bool z) {
    test_circuit([&]<Mode m> {
      const auto xs = renc<m>({ x, y, z });
      const auto zs = one_hot<m>(std::span { xs });
      return dec(std::span { zs });
    }, spec(x,y,z));
  });
}
