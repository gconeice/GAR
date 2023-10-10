#ifndef BIT_H__
#define BIT_H__

#include <mode.h>
#include <label.h>
#include <span>

namespace Garbled { template <Mode m> using Bit = Tuple<Label, m>; }

/**
 * Transform each party's garbled share to a simple XOR share.
 */
template <Mode m>
bool color(const Garbled::Bit<m>& x) {
  const auto [gx, ex] = x;
  GEN { return gx.lsb(); }
  EVAL { return ex.lsb(); }
  SPEC { return { }; }
}

/**
 * Parties agree to encode a bit
 */
template <Mode m> Garbled::Bit<m> constant(bool);

/**
 * G chooses an input bit.
 */
template <Mode m> Garbled::Bit<m> ginput(bool);

/**
 * G chooses an input label.
 */
template <Mode m> Garbled::Bit<m> ginput(const Label&);

/**
 * Decode a bit.
 * 1 bit
 */
template <Mode m> bool dec(const Garbled::Bit<m>&);

/**
 * Reveal a garbling to E.
 * The revealed garbling has color equal to the true value.
 * 1 bit
 */
template <Mode m> Garbled::Bit<m> reveal(Garbled::Bit<m>);

/**
 * Decode n bits.
 * n bits
 */
template <Mode m> std::vector<bool> dec(std::span<const Garbled::Bit<m>>);

/**
 * Choose a randomized encoding of a bit.
 * 1 ciphertext
 */
template <Mode m> Garbled::Bit<m> renc(bool);

/**
 * Choose randomized encodings of n bits.
 * n ciphertexts
 */
template <Mode m> std::vector<Garbled::Bit<m>> renc(const std::vector<bool>&);

/**
 * G chooses a completely random garbling. Not that this is *NOT* a randomly
 * encoded bit, but rather a uniformly selected *language*.
 * 0 ciphertexts
 */
template <Mode m> Garbled::Bit<m> random_garbling();

/**
 * G opens her full share of to E
 *
 * WARNING! Use with great care. Revealing a share of a garbling «X, X + xΔ» could reveal Δ.
 */
template <Mode m> Garbled::Bit<m> open(const Garbled::Bit<m>&);

/**
 * In-place Logical XOR.
 * To XOR two garbled bits, the parties XOR their parts.
 * 0 ciphertexts
 *
 * https://www.cs.toronto.edu/~vlad/papers/XOR_ICALP08.pdf
 */
template <Mode m>
Garbled::Bit<m>& operator^=(Garbled::Bit<m>& x, const Garbled::Bit<m>& y) {
  auto& [gx, ex] = x;
  const auto [gy, ey] = y;
  GEN { gx ^= gy; }
  EVAL { ex ^= ey; }
  return x;
}

/**
 * Logical XOR.
 * 0 ciphertexts
 */
template <Mode m>
Garbled::Bit<m> operator^(Garbled::Bit<m> x, const Garbled::Bit<m>& y) {
  x ^= y;
  return x;
}

/**
 * A half-gate where E knows the left hand argument.
 * Scales the bitstring on the right by the garbled bit on the left.
 * 1 ciphertext
 *
 * https://eprint.iacr.org/2014/756
 */
template <Mode m> Garbled::Bit<m> eand(const Garbled::Bit<m>&, const Garbled::Bit<m>&);

/**
 * A half-gate where G chooses the left hand argument.
 * 1 ciphertext
 *
 * https://eprint.iacr.org/2014/756
 */
template <Mode m> Garbled::Bit<m> gand(bool, const Garbled::Bit<m>&);

/**
 * A half-gate where G chooses the left hand argument; the left hand argument is an
 * *arbitrary* bitstring, not the garbling of a bit.
 * 1 ciphertext
 */
template <Mode m> Garbled::Bit<m> gscale(const Label&, const Garbled::Bit<m>&);


template <Mode m> Garbled::Bit<m> swap_lang(const Label&, const Garbled::Bit<m>&);

/**
 * An AND gate where E knows *both* arguments in cleartexts.
 * ANDs the two garbled bits and produces a garbled bit known to E.
 * 1 ciphertext
 *
 * https://eprint.iacr.org/2014/756
 */
template <Mode m> Garbled::Bit<m> eand_full(const Garbled::Bit<m>&, const Garbled::Bit<m>&);

/**
 * Logical AND.
 * 2 ciphertexts
 */
template <Mode m> Garbled::Bit<m> operator&(const Garbled::Bit<m>&, const Garbled::Bit<m>&);

/**
 * In-place logical NOT.
 * 0 ciphertexts
 */
template <Mode m> void flip(Garbled::Bit<m>& x) { x ^= constant<m>(1); }

/**
 * Logical NOT.
 * 0 ciphertexts
 */
template <Mode m> Garbled::Bit<m> operator~(const Garbled::Bit<m>& x) { return x ^ constant<m>(1); }

/**
 * Logical OR.
 * 2 ciphertexts
 */
template <Mode m> Garbled::Bit<m> operator|(const Garbled::Bit<m>& x, const Garbled::Bit<m>& y) { return ~((~x) & (~y)); }

/**
 * Logical GT.
 * 2 ciphertexts
 */
template <Mode m> Garbled::Bit<m> operator>(const Garbled::Bit<m>& x, const Garbled::Bit<m>& y) { return x & (~y); }

/**
 * Logical LT.
 * 2 ciphertexts
 */
template <Mode m> Garbled::Bit<m> operator<(const Garbled::Bit<m>& x, const Garbled::Bit<m>& y) { return (~x) & y; }

/**
 * Logical GTE.
 * 2 ciphertexts
 */
template <Mode m> Garbled::Bit<m> operator>=(const Garbled::Bit<m>& x, const Garbled::Bit<m>& y) { return ~(x < y); }

/**
 * Logical LTE.
 * 2 ciphertexts
 */
template <Mode m> Garbled::Bit<m> operator<=(const Garbled::Bit<m>& x, const Garbled::Bit<m>& y) { return ~(x > y); }

/**
 * Logical NEQ (identical to XOR).
 * 0 ciphertexts
 */
template <Mode m> Garbled::Bit<m> operator!=(const Garbled::Bit<m>& x, const Garbled::Bit<m>& y) { return x ^ y; }

/**
 * Logical EQ.
 * 0 ciphertexts
 */
template <Mode m> Garbled::Bit<m> operator==(const Garbled::Bit<m>& x, const Garbled::Bit<m>& y) { return ~(x ^ y); }

/**
 * Logical OR where E knows in cleartext the left hand argument.
 * 1 ciphertext
 */
template <Mode m> Garbled::Bit<m> eor(const Garbled::Bit<m>& x, const Garbled::Bit<m>& y) { return ~(eand(~x, ~y)); }


/**
 * In-place logical AND.
 * 2 ciphertexts
 */
template <Mode m>
Garbled::Bit<m>& operator&=(Garbled::Bit<m>& x, const Garbled::Bit<m>& y) { x = x & y; return x; }


#endif
