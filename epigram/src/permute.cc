#include <permute.h>
#include <prg.h>
#include <util.h>
#include <resource.h>

void random_permutation(std::bitset<128> seed, std::span<std::uint32_t> perm) {
  const auto n = perm.size();
  assert(is_pow2(n));

  EPPRG g(seed);
  for (std::size_t i = 0; i < n; ++i) { perm[i] = i; }
  for (std::size_t i = 0; i < n; ++i) {
    std::bitset<128> r = g();
    std::size_t j = *(std::size_t*)&r;
    std::swap(perm[i], perm[j % n]);
  }
}

std::vector<std::uint32_t> random_permutation(std::bitset<128> seed, std::size_t n) {
  std::vector<std::uint32_t> perm(n);
  random_permutation(seed, perm);
  return perm;
}


std::vector<std::uint32_t> invert_permutation(std::span<const std::uint32_t> pi) {
  std::vector<std::uint32_t> pi_inv(pi.size());

  for (std::size_t i = 0; i < pi.size(); ++i) {
    pi_inv[pi[i]] = i;
  }
  return pi_inv;
}

void apply_permutation(
    std::span<const std::uint32_t> pi,
    std::span<std::uint32_t> buff) {
  const auto n = buff.size();
  assert(pi.size() == n);
  std::vector<std::uint32_t> temp(n);
  std::copy(buff.begin(), buff.end(), temp.begin());
  for (std::size_t i = 0; i < n; ++i) {
    buff[i] = temp[pi[i]];
  }
}

TEST_CASE("") {
  constexpr std::size_t n = 32;
  rc::prop("invert_permutation correct", [&]() {
    const auto pi = random_permutation(EPPRG()(), n);
    auto pi_inv = invert_permutation(pi);
    apply_permutation(pi, pi_inv);
    for (std::size_t i = 0; i < n; ++i) {
      RC_ASSERT(pi_inv[i] == i);
    }
  });
}


struct BitPtr {
public:
  BitPtr() { }
  BitPtr(std::vector<bool>& data) : data(&data), offset(0) { }
  BitPtr(std::vector<bool>* data, std::size_t offset)
    : data(data), offset(offset) { }

  auto operator[](std::size_t ix) { return (*data)[ix + offset]; }
  auto operator[](std::size_t ix) const { return (*data)[ix + offset]; }

  BitPtr operator+(std::size_t off) const { return { data, offset + off }; }

private:
  std::vector<bool>* data;
  std::size_t offset;
};


template <Mode m, bool visit_start>
inline void permute(
    std::size_t w,
    std::size_t logn,
    std::uint32_t* src_to_tgt,
    std::uint32_t* tgt_to_src,
    BitPtr visited,
    BitPtr buffer,
    std::span<Garbled::Bit<m>> xs) {

  if (logn == 0) {
  } else if (logn == 1) {
    bool b = false;
    if constexpr (m == Mode::G) {
      b = src_to_tgt[0] != 0;
    }
    gswap<m>(b, xs.subspan(0, w), xs.subspan(w, w));
  } else {
    const std::size_t n4 = 1 << (logn - 2);
    const std::size_t n2 = n4 << 1;
    const std::size_t n = n2 << 1;

    const auto modn2 = [&] (std::size_t x) { return x >= n2 ? x - n2 : x; };
    const auto cong = [&] (std::size_t x) { return x >= n2 ? x - n2 : x + n2; };

    if constexpr (m == Mode::G) {
      for (std::size_t i = 0; i < n; ++i) { tgt_to_src[src_to_tgt[i]] = i; }

      // Program the first layer of swaps
      // The key idea is to send each member of a congruent pair of points (i.e.,
      // x and x + n/2) into different halves of the array.
      // Then, assuming permutations on the two halves, at the end we need only
      // swap the two congruent and aligned elements into the correct slot.
      const auto follow = [&] (std::size_t start) {
        auto src = start;
        bool s = false;
        std::size_t srcmod = modn2(src);
        do {
          // mark the current point as visited
          visited[srcmod] = !visit_start;
          // set the permutation bit
          // as a special case, 0 is always statically known to be false and
          // need not be handled
          if (srcmod != 0) { buffer[srcmod - 1] = s; }
          // find the source whose target is congruent to tgt mod n/2
          const std::size_t tgt = src_to_tgt[src];
          const std::size_t ctgt = cong(tgt);
          const auto csrc = tgt_to_src[ctgt];
          // swap the congruent target's source if the source's output bucket
          // is currently equal to the congruent target's source's bucket
          s = ((s != (src >= n2)) == (csrc >= n2));
          // focus on the source congruent to the congruent target's source
          // which is now constrained by s
          src = cong(csrc);
          srcmod = modn2(src);
        } while (srcmod != modn2(start));
      };

      // special case: we always need to follow the permutation starting at 0
      follow(0);
      follow(n2);

      for (std::size_t cursor = 1; cursor < n2; ++cursor) {
        if (visited[cursor] == visit_start) {
          // follow the permutation only if we have not yet visited this point
          follow(cursor);
          follow(cursor + n2);
        }
      }
    }

    // apply the first layer of swaps
    for (std::size_t i = 1; i < n2; ++i) {
      bool b = false;
      if constexpr (m == Mode::G) {
        if (buffer[i-1]) {
          std::swap(src_to_tgt[i], src_to_tgt[i + n2]);
          b = true;
        }
      }
      gswap<m>(b, xs.subspan(i*w, w), xs.subspan((i+n2)*w, w));
    }


    // program the final layer of swaps
    // place the programming into the end of the buffer such that it is not
    // overwritten by recursive calls
    if constexpr (m == Mode::G) {
      for (std::size_t i = 0; i < n2; ++i) {
        auto& tgt0 = src_to_tgt[i];
        auto& tgt1 = src_to_tgt[i + n2];
        buffer[modn2(tgt0) + n2] = tgt0 >= n2;
        tgt0 = modn2(tgt0);
        tgt1 = modn2(tgt1);
      }
    }


    // recursively permute the two halves
    permute<m, !visit_start>(
        w,
        logn-1,
        src_to_tgt,
        tgt_to_src,
        visited,
        buffer,
        xs.subspan(0, n2*w));
    permute<m, !visit_start>(
        w,
        logn-1,
        src_to_tgt + n2,
        tgt_to_src + n2,
        visited + n4,
        buffer,
        xs.subspan(n2*w));

    // apply the final layer of swaps
    for (std::size_t i = 0; i < n2; ++i) {
      bool b = false;
      if constexpr (m == Mode::G) {
        b = buffer[i + n2];
      }
      gswap<m>(b, xs.subspan(i*w, w), xs.subspan((i+n2)*w, w));
    }
  }
}


template <Mode m>
void gpermute(
    std::size_t w,
    std::span<const std::uint32_t> permutation,
    std::span<Garbled::Bit<m>> xs) {
  // permutation only set up for powers of two
  assert(xs.size() % w == 0);
  const auto n = xs.size()/w;
  assert(is_pow2(n));
  const auto logn = log2(n);

  std::vector<bool> visited;
  std::vector<bool> programming_buffer;
  std::vector<std::uint32_t> tgt_to_src;
  std::vector<std::uint32_t> src_to_tgt;

  if constexpr (m == Mode::G) {
    visited.resize(n/2);
    programming_buffer.resize(n);
    tgt_to_src = { permutation.begin(), permutation.end() };
    src_to_tgt.resize(n);
    for (std::size_t i = 0; i < n; ++i) { src_to_tgt[permutation[i]] = i; }
  }

  permute<m, false>(
      w,
      logn,
      src_to_tgt.data(),
      tgt_to_src.data(),
      visited,
      programming_buffer,
      xs);
}

template void gpermute(std::size_t, std::span<const std::uint32_t>, std::span<Garbled::Bit<Mode::G>>);
template void gpermute(std::size_t, std::span<const std::uint32_t>, std::span<Garbled::Bit<Mode::E>>);
template void gpermute(std::size_t, std::span<const std::uint32_t>, std::span<Garbled::Bit<Mode::S>>);


TEST_CASE("") {
  constexpr std::size_t n = 256;
  constexpr std::size_t w = 18;

  const auto spec = [&](std::span<const std::uint32_t> pi, const std::vector<bool>& xs) {
    std::vector<bool> ys(xs.size());
    for (std::size_t i = 0; i < n; ++i) {
      for (std::size_t j = 0; j < w; ++j) {
        ys[i*w + j] = xs[pi[i]*w + j];
      }
    }
    return ys;
  };

  rc::prop("gpermute correct", [&](std::array<bool, n*w> contentarr) {
    const auto pi = random_permutation(EPPRG()(), n);
    const auto content = bool_arr_to_vec(contentarr);
    test_circuit([&]<Mode m> {
      auto xs = renc<m>(content);

      gpermute(w, std::span { pi }, std::span { xs });
      return dec(std::span<const Garbled::Bit<m>> { xs });
    }, spec(pi, content));
  });

}
