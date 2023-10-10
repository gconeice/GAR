#ifndef EPIGRAM_H__
#define EPIGRAM_H__


#include <lazypermutation.h>
#include <functional>


constexpr std::size_t default_scan_preference_point = 128;


struct StorageMetadata {
  std::uint32_t t;
  std::uint32_t addr;
  std::uint32_t lang_ptr;

  friend std::ostream& operator<<(std::ostream& os, const StorageMetadata& d) {
    os << "【" << d.t << "," << d.addr << "," << d.lang_ptr << "】";
    return os;
  }
};


struct Metadata {
  std::vector<Label> languages;
  std::vector<StorageMetadata> storage_metadata;
  std::vector<std::vector<StorageMetadata>> dummy_metadata;
  std::vector<std::uint32_t> storage_permutations;
  std::vector<std::uint32_t> lazy_permutation_mask;
};


template <Mode m>
struct EpiGRAMLeaf {
  public:
    EpiGRAMLeaf() { }
    EpiGRAMLeaf(
        std::size_t logn,
        std::size_t w,
        std::span<const Label> languages,
        std::span<const std::uint32_t> pi,
        std::span<const StorageMetadata> mdata)
      : logn(logn), w(w), languages(languages), pi(pi), mdata(mdata) { }

    std::vector<Garbled::Bit<m>>
    operator()(std::size_t, std::span<const Garbled::Bit<m>>) const;

    std::size_t ninp() const { return logn + 2; }
    std::size_t nout() const { return w + logn + 2; }

    EpiGRAMLeaf<Mode::S> speculate() const {
      return EpiGRAMLeaf<Mode::S> { logn, w, languages, pi, mdata };
    }

  private:
    std::size_t logn;
    std::size_t w;
    std::span<const Label> languages;
    std::span<const std::uint32_t> pi;
    std::span<const StorageMetadata> mdata;
};


template <Mode m>
struct EpiGRAM {
public:
  EpiGRAM() { }

  std::vector<Garbled::Bit<m>> access(
        const std::function<void(std::span<Garbled::Bit<m>>)>&,
        std::span<const Garbled::Bit<m>>);

  std::vector<Garbled::Bit<m>> read(std::span<const Garbled::Bit<m>> i) {
    return access([](auto) { }, i);
  }

  std::vector<Garbled::Bit<m>> write(
        std::span<const Garbled::Bit<m>> i,
        std::span<const Garbled::Bit<m>> x) {
    return access([&](auto buff) {
      assert(x.size() == buff.size());
      std::copy(x.begin(), x.end(), buff.begin());
    }, i);
  }

  std::size_t word_size() const { return w; }
  std::size_t size() const { return n; }

  static EpiGRAM<m> make(
      std::size_t w,
      std::span<const Garbled::Bit<m>> content,
      std::size_t scan_preference_point = default_scan_preference_point) {
    assert(content.size() % w == 0);
    auto n = content.size() / w;
    assert(is_pow2(n));
    EpiGRAM<m> out(w, n, scan_preference_point);
    out.refresh(content);
    return out;
  }

  static EpiGRAM<m> make(
      std::size_t w,
      std::size_t n,
      std::size_t scan_preference_point = default_scan_preference_point) {
    EpiGRAM<m> out(w, n, scan_preference_point);

    std::vector<Garbled::Bit<m>> buff(n*w);
    out.refresh(std::span { buff });
    return out;
  }

private:
  EpiGRAM(std::size_t w, std::size_t n, std::size_t scan_preference_point);

  std::vector<Garbled::Bit<m>> flush();
  void shuffle();
  void refresh(std::span<const Garbled::Bit<m>>);
  void initialize(std::size_t w, std::span<const Garbled::Bit<m>>);

  std::vector<std::size_t> hide(
      std::span<const Garbled::Bit<m>>,
      std::span<Garbled::Bit<m>>);

  std::vector<Garbled::Bit<m>> half_write(
    std::span<const Garbled::Bit<m>> i,
    std::span<const Garbled::Bit<m>> x);

  bool is_scan_ram;
  std::size_t n;
  std::size_t logn;
  std::size_t w;
  std::size_t t;

  LazyPermutation<EpiGRAMLeaf, m> lpn;
  std::vector<Garbled::Bit<m>> storage;
  Metadata mdata;
  std::unique_ptr<EpiGRAM<m>> index_map;
  std::size_t pi_progress;
};

#endif
