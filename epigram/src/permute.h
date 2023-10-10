#ifndef PERMUTE_H__
#define PERMUTE_H__

#include <vector>
#include <bitset>
#include <bitstring.h>

void random_permutation(std::bitset<128> seed, std::span<std::uint32_t>);
std::vector<std::uint32_t> random_permutation(std::bitset<128> seed, std::size_t n);


std::vector<std::uint32_t> invert_permutation(std::span<const std::uint32_t>);

void apply_permutation(
    std::span<const std::uint32_t>,
    std::span<std::uint32_t>);



/**
 * Permute a collection of garbled width-w words according to a permutation
 * chosen by G.
 *
 * The permutation should specify the target for each source.
 * For example, { 3, 0, 1, 2 } will move the current third element to slot 0.
 */
template <Mode m>
void gpermute(
    std::size_t w,
    std::span<const std::uint32_t> permutation,
    std::span<Garbled::Bit<m>>);

#endif
