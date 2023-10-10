#ifndef BITSTRING_H__
#define BITSTRING_H__


#include <byte.h>


template <Mode m>
std::span<Garbled::Bit<m>> operator&=(
    std::span<Garbled::Bit<m>>,
    std::span<const Garbled::Bit<m>>);


template <Mode m>
std::span<Garbled::Bit<m>> operator&=(
    std::span<Garbled::Bit<m>>,
    const Garbled::Bit<m>&);


template <Mode m>
std::span<Garbled::Bit<m>> operator^=(
    std::span<Garbled::Bit<m>>,
    std::span<const Garbled::Bit<m>>);


template <Mode m>
std::span<Garbled::Bit<m>> operator^=(
    std::span<Garbled::Bit<m>> xs,
    std::span<Garbled::Bit<m>> ys) {
  xs ^= std::span<const Garbled::Bit<m>> { ys };
  return xs;
}


template <Mode m>
std::span<Garbled::Bit<m>> operator^=(
    std::span<Garbled::Bit<m>> xs,
    const std::vector<Garbled::Bit<m>>& ys) {
  xs ^= std::span<const Garbled::Bit<m>> { ys };
  return xs;
}


template <Mode m>
std::span<Garbled::Bit<m>> operator^=(
    std::span<Garbled::Bit<m>>,
    const Garbled::Bit<m>&);

std::vector<Label> operator^(std::span<const Label>, std::span<const Label>);


template <Mode m>
void flip(std::span<Garbled::Bit<m>> xs) {
  xs ^= enc<m>(1);
}


template <Mode m>
void swap(
    const Garbled::Bit<m>&,
    std::span<Garbled::Bit<m>>,
    std::span<Garbled::Bit<m>>);


/**
 * Conditionally swap two bitstrings of the same length, depending on the value
 * of some garbled bit `s` whose value is known to E.
 */
template <Mode m>
void eswap(
    const Garbled::Bit<m>& s,
    std::span<Garbled::Bit<m>>,
    std::span<Garbled::Bit<m>>);


/**
 * Conditionally swap two bitstrings of the same length, depending on the value
 * of some bit `s` chosen by G.
 */
template <Mode m>
void gswap(
    bool s,
    std::span<Garbled::Bit<m>>,
    std::span<Garbled::Bit<m>>);


template <Mode m>
std::vector<Garbled::Bit<m>>& operator&=(
    std::vector<Garbled::Bit<m>>& xs,
    const std::vector<Garbled::Bit<m>>& ys) {
  std::span { xs } &= std::span { ys };
  return xs;
}


template <Mode m>
std::vector<Garbled::Bit<m>>& operator&=(
    std::vector<Garbled::Bit<m>>& xs,
    const Garbled::Bit<m>& y) {
  std::span { xs } &= y;
  return xs;
}


template <Mode m>
std::vector<Garbled::Bit<m>>& operator^=(
    std::vector<Garbled::Bit<m>>& xs,
    const std::vector<Garbled::Bit<m>>& ys) {
  std::span { xs } ^= std::span { ys };
  return xs;
}


template <Mode m>
std::vector<Garbled::Bit<m>>& operator^=(
    std::vector<Garbled::Bit<m>>& xs,
    const Garbled::Bit<m>& y) {
  std::span { xs } ^= y;
  return xs;
}

/**
 * G opens her full share of to E
 *
 * WARNING! Use with great care. Revealing a share of a garbling «X, X + xΔ» could reveal Δ.
 */
template <Mode m> void open(std::span<Garbled::Bit<m>>);


template <Mode m>
std::vector<Garbled::Bit<m>> random_garbled_string(std::size_t);

std::vector<Label> random_labels(std::size_t);


/**
 * A half-gate where G chooses the right hand argument; the right hand argument holds
 * *arbitrary* bitstrings, not garblings of bits.
 */
template <Mode m>
std::vector<Garbled::Bit<m>> gscale(const Garbled::Bit<m>&, std::span<const Label>);


template <Mode m>
void ginput(std::span<const Label>, std::span<Garbled::Bit<m>>);

template <Mode m>
std::vector<Garbled::Bit<m>> ginput(std::span<const Label> x) {
  std::vector<Garbled::Bit<m>> out(x.size());
  ginput(x, std::span { out });
  return out;
}


template <Mode m>
void swap_lang(std::span<const Label>, std::span<Garbled::Bit<m>>);


#endif
