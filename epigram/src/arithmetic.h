#ifndef ARITHMETIC_H__
#define ARITHMETIC_H__


#include <bitstring.h>


template <Mode m>
std::vector<Garbled::Bit<m>>
constant_nat(std::size_t w, std::size_t x);


template <Mode m>
std::vector<Garbled::Bit<m>>
ginput_nat(std::size_t w, std::size_t x);


template <Mode m>
void ginput_nat(std::size_t x, std::span<Garbled::Bit<m>>);


template <Mode m>
Garbled::Bit<m> operator<=(
    std::span<const Garbled::Bit<m>>,
    std::span<const Garbled::Bit<m>>);

template <Mode m>
Garbled::Bit<m> operator<(
    std::span<const Garbled::Bit<m>> x,
    std::span<const Garbled::Bit<m>> y) {
  return ~(y <= x);
}

template <Mode m>
Garbled::Bit<m> operator==(
    std::span<const Garbled::Bit<m>>,
    std::span<const Garbled::Bit<m>>);

template <Mode m>
std::size_t dec_nat(std::span<const Garbled::Bit<m>>);

template <Mode m>
std::span<Garbled::Bit<m>> operator+=(
    std::span<Garbled::Bit<m>>,
    std::span<const Garbled::Bit<m>>);

template <Mode m>
std::span<Garbled::Bit<m>> operator-=(
    std::span<Garbled::Bit<m>>,
    std::span<const Garbled::Bit<m>>);

#endif
