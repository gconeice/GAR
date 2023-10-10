#ifndef LAZY_PERMUTATION_H__
#define LAZY_PERMUTATION_H__


#include <simplestack.h>
#include <resource.h>


/**
 * At its most basic, the lazy permutation is a simple read only memory (with one-time access).
 * This simplified lazy permutation stores one piece of data at each of its leaves.
 *
 * However, for EpiGRAM, we need a slight generalization:
 * Namely each leaf stores a *circuit* which is evaluated on private data.
 *
 * Therefore, the lazy permutation is parameterized over a template type `Leaf`.
 * At its most basic level, the Leaf implements a family of `n` circuits.
 * When called `leaf(i, x)`, the leaf executes the `i`th circuit on input string `x`.
 */
template <Mode m>
struct LeafTemplate {
  // Given the leaf index, populate a circuit
  std::vector<Garbled::Bit<m>> operator()(std::size_t, std::span<const Garbled::Bit<m>>) const;
  std::size_t ninp() const;
  std::size_t nout() const;

  LeafTemplate<Mode::S> speculate() const;
};


template <template <Mode> typename Leaf, Mode m>
struct LazyPermutation {
public:
  LazyPermutation() { }
  LazyPermutation(std::size_t n, const Leaf<m>&);

  std::vector<Garbled::Bit<m>> operator()(
      std::span<const Garbled::Bit<m>> index,
      std::span<const Garbled::Bit<m>> leaf_inp);

private:
  Leaf<m> leaf;
  std::size_t n;
  std::vector<Garbled::Bit<m>> input_languages;
  std::vector<SimpleStack<m>> stacks;
  std::size_t timer;
  std::vector<Cost> resource_positions;
};


template <Mode m>
struct LookupLeaf {
  public:
    LookupLeaf() { }
    LookupLeaf(std::size_t w) : w(w) { }
    LookupLeaf(std::size_t w, const std::vector<Garbled::Bit<m>>& data) : w(w), data(data) { }

    std::vector<Garbled::Bit<m>> operator()(std::size_t leaf, std::span<const Garbled::Bit<m>>) const {
      if constexpr (m == Mode::S) {
        std::vector<Garbled::Bit<Mode::S>> out(w);
        return out;
      } else {
        std::vector<Garbled::Bit<m>> out(w);
        std::copy(data.begin() + leaf*w, data.begin() + (leaf+1)*w, out.begin());
        return out;
      }
    }

    std::size_t ninp() const { return 0; }
    std::size_t nout() const { return w; }

    LookupLeaf<Mode::S> speculate() const {
      return LookupLeaf<Mode::S> { w };
    }

  private:
    std::size_t w;
    std::vector<Garbled::Bit<m>> data;
};


#include <lazypermutation.hh>

#endif
