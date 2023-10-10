#include <epigram.h>
#include <linearscan.h>
#include <util.h>
#include <permute.h>
#include <arithmetic.h>

constexpr std::size_t perm_size(std::size_t t) {
  const auto lowest_set_bit = (log2(t & -t) + 1);
  return 1 << lowest_set_bit;
}

constexpr std::size_t num_levels(std::size_t t) {
  // popcount gives the number of populated levels of storage
  return static_cast<std::size_t>(std::popcount(t));
}


std::vector<std::uint32_t> schedule_permutations(std::size_t n) {
  std::size_t total_perm_size = 0;
  for (std::size_t t = n; t <= 2*n; ++t) {
    total_perm_size += perm_size(t);
  }

  std::vector<std::uint32_t> perm(total_perm_size);
  std::size_t ptr = 0;
  for (std::size_t t = n; t <= 2*n; ++t) {
    std::size_t psize = perm_size(t);
    random_permutation(EPPRG()(), to_span(perm, ptr, psize));
    ptr += psize;
  }
  return perm;
}


Metadata gschedule(
    const std::vector<std::uint32_t>& lpn_mask,
    std::size_t n,
    std::size_t w) {
  const auto logn = log2(n);

  Metadata mdata;
  mdata.storage_permutations = schedule_permutations(n);
  mdata.languages = random_labels(mdata.storage_permutations.size()*w);
  mdata.lazy_permutation_mask = lpn_mask;
  mdata.storage_metadata.resize(2*n*(logn+1));
  mdata.dummy_metadata.resize(2*n);

  std::vector<std::uint32_t> mdata_counters(2*n);

  std::vector<std::uint32_t> buffer(4*n);
  for (std::size_t i = 0; i < 4*n; ++i) { buffer[i] = i; }

  std::size_t pptr = 0;
  for (std::size_t t = n; t <= 2*n; ++t) {
    std::size_t psize = perm_size(t);
    const auto pi = to_span(mdata.storage_permutations, pptr, psize);

    apply_permutation(pi, to_span(buffer, 2*t - psize, psize));

    for (std::size_t i = 0; i < psize; ++i) {
      // scan the permuted elements and, for those real elements, write down the metadata
      const auto addr = 2*t - psize + i;
      const auto index = buffer[addr];
      if (index % 2 == 0) {
        auto r = index/2;
        std::size_t offset = mdata_counters[r];
        ++mdata_counters[r];

        mdata.storage_metadata[r*(logn+1) + offset] =
          { static_cast<std::uint32_t>(t),
            static_cast<std::uint32_t>(addr),
            static_cast<std::uint32_t>(pptr + i) };
      } else if (t < 2*n) {
        const auto d = index / 2;
        const auto order = d - (t - (psize/2));
        mdata.dummy_metadata[t - n + order].push_back(
            { static_cast<std::uint32_t>(t)
            , static_cast<std::uint32_t>(addr)
            , static_cast<std::uint32_t>(pptr + i) });
      }
    }
    pptr += psize;
  }

  // pad the metadata such that each one-time index has exactly logn + 1 entries
  for (std::size_t i = 0; i < 2*n; ++i) {
    auto offset = mdata_counters[i];
    while (offset < logn + 1) {
      mdata.storage_metadata[i*(logn+1) + offset] =
        mdata.storage_metadata[i*(logn+1) + offset - 1];
      ++offset;
    }
  }

  return mdata;
}


template <Mode m>
std::vector<Garbled::Bit<m>>
EpiGRAMLeaf<m>::operator()(std::size_t leaf, std::span<const Garbled::Bit<m>> T) const {
  std::span<const StorageMetadata> leaf_mdata;
  if constexpr (m == Mode::G) {
    leaf_mdata = mdata.subspan(pi[leaf] * (logn+1), logn+1);
  } else {
    leaf_mdata = mdata.subspan(0, logn+1);
  }

  std::vector<Garbled::Bit<m>> out(logn+2 + w);
  auto addr = to_span(out, 0, logn+2);
  auto lang = to_span(out, logn+2);

  auto inlang = languages.subspan(leaf_mdata[0].lang_ptr*w, w);

  ginput_nat(leaf_mdata[0].addr, addr);
  ginput(inlang, lang);

  for (std::size_t i = 1; i < logn + 1; ++i) {
    auto [tt, aa, lptr] = leaf_mdata[i];
    const auto t = ginput_nat<m>(logn + 2, tt);
    const auto cmp = std::span { t } <= T;

    auto a = ginput_nat<m>(logn + 2, aa);
    std::span { a } ^= addr;
    std::span { a } &= cmp;
    addr ^= std::span { a };

    const auto ldiff =
        languages.subspan(leaf_mdata[i-1].lang_ptr*w, w) ^
        languages.subspan(lptr * w, w);

    lang ^= gscale(cmp, ldiff);
  }
  return out;
}


template <Mode m>
std::vector<std::size_t> EpiGRAM<m>::hide(
    std::span<const Garbled::Bit<m>> addr,
    std::span<Garbled::Bit<m>> out_lang) {

  const auto dummy_mdata = std::span { mdata.dummy_metadata[t-n] };
  const auto logn = log2(n);

  assert (dummy_mdata.size() == num_levels(t));
  assert (addr.size() == logn + 2);
  assert (out_lang.size() == w);

  // keep track of two bits that indicate if addr is in the current range of storage
  // initially, the left range starts at address 0, so addr is definitely greater than this
  auto occurs_after = constant<m>(1);

  std::vector<std::size_t> output_addresses(num_levels(t));
  for (std::size_t i = 0; i < num_levels(t); ++i) {
    auto [tt, aa, lptr] = dummy_mdata[i];
    const auto right_bound = ginput_nat<m>(logn+2, 2*t);
    const auto occurs_before = addr < std::span { right_bound };
    const auto in_range = occurs_after & occurs_before;

    // sum together all dummy languages and subtract out the single unused dummy
    const auto dummy_lang = to_span(mdata.languages, dummy_mdata[i].lang_ptr*w, w);
    // encode the dummy language as a garbling
    out_lang ^= ginput<m>(dummy_lang);
    out_lang ^= gscale(in_range, dummy_lang);

    auto a = ginput_nat<m>(logn + 2, aa);
    std::span { a } ^= addr;
    std::span { a } &= ~in_range;
    std::span { a } ^= addr;
    output_addresses[i] = dec_nat(std::span<const Garbled::Bit<m>> { a });

    // address could be in next range if it is after the end of this one
    occurs_after = ~occurs_before;
  }
  return output_addresses;
}


/**
 * The recursive RAMs store double-wide words so that we can divide the size of
 * RAM in half on each iteration.
 * Thus, we need a method for writing to double-wide words.
 */
template <Mode m>
std::vector<Garbled::Bit<m>> EpiGRAM<m>::half_write(
    std::span<const Garbled::Bit<m>> i,
    std::span<const Garbled::Bit<m>> x) {

  if (is_scan_ram) {
    return write_scan(x, i, std::span { storage });
  } else {
    auto out = access([&](auto slot) {
      assert(slot.size() == 2*x.size());

      auto half0 = slot.subspan(0, x.size());
      auto half1 = slot.subspan(x.size());

      // i[0] = 0 => half0 <- x
      // ---
      // (half0 + x)*i[0] + x
      // ---
      // { x      if i[0] = 0
      //   half0  otherwise
      // }
      half0 ^= x;
      half0 &= i[0];
      half0 ^= x;

      half1 ^= x;
      half1 &= ~i[0];
      half1 ^= x;
    }, i.subspan(1));
    auto half0 = to_span(out, 0, w/2);
    auto half1 = to_span(out, w/2);

    std::vector<Garbled::Bit<m>> buff(w/2);
    std::span { buff } ^= half0;
    std::span { buff } ^= half1;
    std::span { buff } &= i[0];
    std::span { buff } ^= half0;
    return buff;
  }
}

template <Mode m>
std::vector<Garbled::Bit<m>> EpiGRAM<m>::flush() {
  if (is_scan_ram) {
    return storage;
  } else {
    const auto logn = log2(n);

    // Final shuffle to put all elements in top level of RAM
    shuffle();

    const auto indices = index_map->flush();

    std::vector<Garbled::Bit<m>> content(w*n);
    const auto T = ginput_nat<m>(logn+2, t);
    for (std::size_t i = 0; i < n; ++i) {
      auto slice = to_span(content, i*w, w);

      const auto ix = to_span(indices, i*(logn + 1), logn + 1);
      const auto lpn_out = lpn(ix, std::span { T });
      const auto addr = dec_nat(to_span(lpn_out, 0, logn+2));
      const auto lang = to_span(lpn_out, logn+2);

      slice ^= lang;
      if constexpr (m == Mode::E) {
        slice ^= to_span(storage, addr*w, w);
      }
    }
    return content;
  }
}


template <Mode m>
void EpiGRAM<m>::refresh(std::span<const Garbled::Bit<m>> content) {
  if (is_scan_ram) {
    std::copy(content.begin(), content.end(), storage.begin());
  } else {
    t = n;
    pi_progress = 0;

    std::vector<Garbled::Bit<m>> rec(2*(logn + 1)*(n/2));

    const auto lpn_mask = random_permutation(EPPRG()(), 2*n);
    for (std::size_t i = 0; i < n; ++i) {
      ginput_nat(lpn_mask[i], to_span(rec, i*(logn + 1), logn + 1));
    }

    index_map->refresh(std::span { rec });

    // TODO this is expensive and unneeded for E and S
    mdata = gschedule(lpn_mask, n, w);

    auto pi_inv = invert_permutation(std::span { mdata.lazy_permutation_mask });

    auto leaf = EpiGRAMLeaf<m>(
          logn,
          w,
          std::span { mdata.languages },
          std::span { pi_inv },
          std::span { mdata.storage_metadata });

    lpn = LazyPermutation(
        2*n,
        std::move(leaf));

    storage.clear();
    storage.resize(4*n*w);

    // We store the real content in the even slots of ram
    for (std::size_t i = 0; i < n; ++i) {
      std::copy(
          content.begin() + i*w,
          content.begin() + (i+1)*w,
          storage.begin() + 2*i*w);
    }
  }
}


template <Mode m>
EpiGRAM<m>::EpiGRAM(std::size_t w, std::size_t n, std::size_t scan_preference_point) {
  this->w = w;
  this->n = n;
  this->logn = log2(n);
  this->t = n;
  this->is_scan_ram = n <= scan_preference_point;
  if (is_scan_ram) {
    storage.resize(n*w);
  } else {
    index_map = std::unique_ptr<EpiGRAM<m>>(new EpiGRAM(2*(logn + 1), n/2, scan_preference_point));
  }
}


template <Mode m>
void EpiGRAM<m>::shuffle() {
  const auto psize = perm_size(t);
  std::span<uint32_t> pi;
  if constexpr (m == Mode::G) {
    pi = to_span(mdata.storage_permutations, pi_progress, psize);
  }

  auto to_permute = to_span(storage, w*(2*t - psize), w*psize);
  gpermute(w, pi, to_permute);
  swap_lang(to_span(mdata.languages, pi_progress*w, psize*w), to_permute);
  pi_progress += psize;
}


template <Mode m>
std::vector<Garbled::Bit<m>> EpiGRAM<m>::access(
    const std::function<void(std::span<Garbled::Bit<m>>)>& f,
    std::span<const Garbled::Bit<m>> i) {

  if (is_scan_ram) {
    const auto out = read_scan(w, i, std::span<const Garbled::Bit<m>> { storage });
    auto buff = out;
    f(std::span { buff });
    write_scan(std::span<const Garbled::Bit<m>> { buff }, i, std::span { storage });
    return out;
  } else {
    if (t == 2*n) {
      // EpiGRAM is only good for n accesses; after n accesses, flush and reinitialize
      const auto content = flush();
      refresh(content);
    }

    shuffle();

    std::size_t pi_t = 0;
    if constexpr (m == Mode::G) { pi_t = mdata.lazy_permutation_mask[t]; }

    const auto Pi_t = ginput_nat<m>(logn+1, pi_t);
    const auto T = ginput_nat<m>(logn+2, t);

    // Look up the one-time index p corresponding to α and write back π(t)
    const auto Pi_p = index_map->half_write(i, std::span { Pi_t });

    // Look up the address and language for index p
    const auto lpn_out = lpn(std::span { Pi_p }, std::span { T });
    const auto addr = to_span(lpn_out, 0, logn+2);
    const auto lang = to_span(lpn_out, logn+2);

    std::vector<Garbled::Bit<m>> out(w);

    // Compute each physical address and account for the last dummy mask
    const auto addresses = hide(addr, std::span { out });
    std::span { out } ^= lang;

    // E looks up each address
    if constexpr (m == Mode::E) {
      for (auto a : addresses) {
        std::span { out } ^= to_span(storage, a*w, w);
      }
    }
    // apply f to the element read from RAM so we can write it back
    std::vector<Garbled::Bit<m>> to_store(w);
    std::copy(out.begin(), out.end(), to_store.begin());
    f(std::span { to_store });

    // write back to a fresh slot
    std::copy(to_store.begin(), to_store.end(), storage.begin() + w*2*t);
    // write a fresh dummy
    for (std::size_t i = 0; i < w; ++i) {
      storage[w*2*t + w + i] = constant<m>(0);
    }

    ++t;
    return out;
  }
}


template struct EpiGRAM<Mode::G>; 
template struct EpiGRAM<Mode::E>; 
template struct EpiGRAM<Mode::S>; 


struct Spec {
  Spec();
  Spec(std::size_t n) : content(n) { }

  std::size_t write(std::size_t ix, std::size_t val) {
    const auto out = content[ix];
    content[ix] = val;
    return out;
  }

  std::vector<std::size_t> content;
};


TEST_CASE("") {
  constexpr std::size_t n = 64;
  constexpr std::size_t w = 10;
  constexpr std::size_t t = 128;
  rc::prop("EpiGRAM Correct", [](std::array<std::size_t, t> ixs, std::array<std::size_t, t> vals) {
    std::vector<std::size_t> expected;
    Spec s(n);
    for (std::size_t i = 0; i < t; ++i) {
      const auto ix = ixs[i] % n;
      const auto val = vals[i] % (1 << w);
      expected.push_back(s.write(ix, val));
    }

    test_circuit([&]<Mode m> {
      std::vector<std::size_t> actual;

      auto arr = EpiGRAM<m>::make(w, n);

      for (std::size_t i = 0; i < t; ++i) {
        const auto ix = constant_nat<m>(log2(n), ixs[i] % n);
        const auto val = constant_nat<m>(w, vals[i] % (1 << w));
        const auto out = arr.write(ix, val);
        actual.push_back(dec_nat<m>(std::span { out }));
      }

      return actual;
    }, expected);
  });
}
