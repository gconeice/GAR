#include <onehot.h>
#include <resource.h>


template <Mode m>
std::vector<Garbled::Bit<m>> onehot_op(std::span<const Garbled::Bit<m>> x, std::span<const Garbled::Bit<m>> y) {
  const auto n = x.size();

  SPEC {
    scost.rows += 2*(n-1) + y.size();
    scost.nonces += n;
    scost.bits += n;
    std::vector<Garbled::Bit<m>> out((1<<n) * y.size());
    return out;
  }

  // We maintain the seed buffer by putting seeds into appropriate tree locations.
  // The buffer only has to be large enough for the final layer as we only
  // store intermediate seeds temporarily.
  std::vector<Garbled::Bit<m>> seeds(1 << n);

  // E keeps track of the missing tree node.
  std::size_t missing = 0;

  // As a base case, we can derive the first two seeds from the possible labels for x[n-1].
  const auto xn = dec(x[n-1]);
  const auto [gxn, exn] = x[n-1];
  const auto one = ginput<m>(1);
  const auto [go, eo] = one;

  GEN {
    seeds[0] = { H(gxn ^ go) };
    seeds[1] = { H(gxn) };
    ++gcost.nonces;
  }
  EVAL {
    seeds[0] = { xn ? H(exn) : 0 };
    seeds[1] = { xn ? 0 : H(exn) };
    missing |= xn;
    ++ecost.nonces;
  }

  // General case -- construct the GGM tree where E is missing all seeds along
  // the path to x
  for (std::size_t i = 1; i < n; ++i) {
    const auto xi = dec(x[n-i-1]);
    const auto [gxi, exi] = x[n-i-1];

    auto odds = constant<m>(0);
    auto evens = constant<m>(0);
    for (int j = 1 << (i-1); j >= 0; --j) {
      const auto& [seedG, seedE] = seeds[j];
      GEN {
        seeds[j*2 + 1].G() = H(seedG, 0);
        seeds[j*2].G() = H(seedG, 1);
        evens.G() ^= seeds[j*2].G();
        odds.G() ^= seeds[j*2 + 1].G();
      }
      EVAL {
        if (j != static_cast<int>(missing)) {
          seeds[j*2 + 1] = H(seedE, 0);
          seeds[j*2] = H(seedE, 1);
          evens ^= seeds[j*2];
          odds ^= seeds[j*2 + 1];
        }
      }
    }

    GEN {
      send(H(gxi ^ go) ^ evens.G());
      send(H(gxi) ^ odds.G());
      ++gcost.nonces;
    }
    EVAL {
      const auto even_row = recv();
      const auto odd_row = recv();
      missing = (missing << 1) | xi;
      seeds[missing ^ 1].E() = H(exi) ^ (xi ? (even_row ^ evens.E()) : (odd_row ^ odds.E()));
      seeds[missing].E() = 0;
      ++ecost.nonces;
    }
  }

  // With the leaf seeds computed, construct the outer product
  std::vector<Garbled::Bit<m>> out((1<<n) * y.size());
  for (std::size_t i = 0; i < y.size(); ++i) {
    const auto [gy, ey] = y[i];
    auto sum = constant<m>(0);
    for (std::size_t j = 0; j < (1 << n); ++j) {
      const auto& [seedG, seedE] = seeds[j];
      GEN {
        out[j*y.size() + i].G() = H(seedG, i);
        sum.G() ^= out[j*y.size() + i].G();
      }
      EVAL {
        if (j != missing) {
          out[j*y.size() + i] = H(seedE, i);
          sum ^= out[j*y.size() + i];
        }
      }
    }
    GEN {
      send(sum.G() ^ gy);
    }
    EVAL {
      const auto total = recv();
      out[missing*y.size() + i].E() = total ^ sum.E() ^ ey;
    }
  }
  return out;
}

TEST_CASE("") {
  const auto onehot = [](bool x, bool y, bool z) {
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

  const auto op = [](const std::vector<bool>& xs, const std::vector<bool>& ys) {
    std::vector<bool> out(xs.size()*ys.size());
    for (std::size_t i = 0; i < xs.size(); ++i) {
      for (std::size_t j = 0; j < ys.size(); ++j) {
        out[i*ys.size() + j] = xs[i] & ys[j];
      }
    }
    return out;
  };

  rc::prop("One hot correct", [&] (bool x, bool y, bool z, bool a, bool b) {
    test_circuit([&]<Mode m> {
      const auto xs = renc<m>({ x, y, z });
      const auto ys = renc<m>({ a, b });
      const auto zs = onehot_op(std::span { xs }, std::span { ys });
      return dec(std::span { zs });
    }, op(onehot(x,y,z), {a,b}));
  });
}

template std::vector<Garbled::Bit<Mode::G>> onehot_op(
    std::span<const Garbled::Bit<Mode::G>>, std::span<const Garbled::Bit<Mode::G>>);
template std::vector<Garbled::Bit<Mode::E>> onehot_op(
    std::span<const Garbled::Bit<Mode::E>>, std::span<const Garbled::Bit<Mode::E>>);
template std::vector<Garbled::Bit<Mode::S>> onehot_op(
    std::span<const Garbled::Bit<Mode::S>>, std::span<const Garbled::Bit<Mode::S>>);
