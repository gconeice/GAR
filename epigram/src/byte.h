#ifndef BYTE_H__
#define BYTE_H__


#include <bit.h>

namespace Garbled { template <Mode m> using Byte = Tuple<WLabel, m>; }

/**
 * Transform each party's garbled share to a simple XOR share.
 */
template <Mode m>
std::byte color(const Garbled::Byte<m>& x) {
  const auto [gx, ex] = x;
  GEN { return gx.lsb(); }
  EVAL { return ex.lsb(); }
  SPEC { return { }; }
}

/**
 * Populate a full byte with copies of a single bit.
 */
template <Mode m>
Garbled::Byte<m> broadcast(const Garbled::Bit<m>& x) {
  const auto [gx, ex] = x;
  GEN { return WLabel::broadcast(gx); }
  EVAL { return WLabel::broadcast(ex); }
  SPEC { return { }; }
}

/**
 * G chooses a completely random byte-length garbling. Not that this is *NOT* a
 * randomly encoded byte, but rather a uniformly selected *language*.
 * 0 ciphertexts
 */
template <Mode m> Garbled::Byte<m> random_wide_garbling();

/**
 * Parties agree to encode a byte
 */
template <Mode m> Garbled::Byte<m> constant(std::byte);

/**
 * G chooses an input bit.
 */
template <Mode m> Garbled::Byte<m> ginput(std::byte);

/**
 * G chooses an input label.
 */
template <Mode m> Garbled::Byte<m> ginput(const WLabel&);

/**
 * Decode a byte
 * 8 bits
 */
template <Mode m> std::byte dec(const Garbled::Byte<m>&);


template <Mode m> Garbled::Byte<m> reveal(Garbled::Byte<m>);

/**
 * Choose a randomized encoding of a byte.
 * 8 ciphertexts
 */
template <Mode m> Garbled::Byte<m> renc(std::byte);

/**
 * In-place Logical XOR.
 * To XOR two garbled bytes, the parties XOR their parts.
 * 0 ciphertexts
 *
 * https://www.cs.toronto.edu/~vlad/papers/XOR_ICALP08.pdf
 */
template <Mode m>
Garbled::Byte<m>& operator^=(Garbled::Byte<m>& x, const Garbled::Byte<m>& y) {
  auto& [gx, ex] = x;
  const auto & [gy, ey] = y;
  GEN { 
    // std::cout << &gx << std::endl;
    gx ^= gy; 
  }
  EVAL { ex ^= ey; }
  return x;
}

/**
 * Logical XOR.
 * 0 ciphertexts
 */
template <Mode m>
Garbled::Byte<m> operator^(Garbled::Byte<m> x, const Garbled::Byte<m>& y) {
  x ^= y;
  return x;
}

/**
 * A half-gate where E knows the left hand argument.
 * Pointwise scales the bitstrings on the right by the garbled bits on the left.
 * 8 ciphertexts
 *
 * https://eprint.iacr.org/2014/756
 */
template <Mode m> Garbled::Byte<m> eand(const Garbled::Byte<m>&, const Garbled::Byte<m>&);

/**
 * A half-gate where G chooses the left hand argument.
 * 8 ciphertexts
 *
 * https://eprint.iacr.org/2014/756
 */
template <Mode m> Garbled::Byte<m> gand(std::byte, const Garbled::Byte<m>&);

/**
 * A half-gate where G chooses the left hand argument; the left hand argument is an
 * *arbitrary* bitstring, not the garbling of a bit.
 * 8 ciphertexts
 */
template <Mode m> Garbled::Byte<m> gscale(const WLabel&, const Garbled::Byte<m>&);

template <Mode m> Garbled::Byte<m> swap_lang(const WLabel&, const Garbled::Byte<m>&);

/**
 * Logical AND.
 * 16 ciphertexts
 */
template <Mode m> Garbled::Byte<m> operator&(const Garbled::Byte<m>&, const Garbled::Byte<m>&);

/**
 * In-place logical NOT.
 * 0 ciphertexts
 */
template <Mode m> void flip(Garbled::Byte<m>& x) { x ^= enc<m>(std::byte { 0xFF }); }

/**
 * Logical NOT.
 * 0 ciphertexts
 */
template <Mode m> Garbled::Byte<m> operator~(const Garbled::Byte<m>& x) { return x ^ enc<m>(std::byte { 0xFF }); }

/**
 * Logical OR.
 * 16 ciphertexts
 */
template <Mode m> Garbled::Byte<m> operator|(const Garbled::Byte<m>& x, const Garbled::Byte<m>& y) { return ~((~x) & (~y)); }


/**
 * Scale each bitstring on the right by single garbled bit on the left, given
 * the bit is known to E.
 * 8 ciphertexts.
 */
template <Mode m>
Garbled::Byte<m> escale(const Garbled::Bit<m>& x, const Garbled::Byte<m>& y) {
  return eand<m>(broadcast<m>(x), y);
}


/**
 * Scale each bitstring on the right by single garbled bit on the left, given
 * the bit is known to E.
 * 8 ciphertexts.
 */
template <Mode m>
Garbled::Byte<m> gscale(bool x, const Garbled::Byte<m>& y) {
  const std::uint8_t lhs = (0xFF) + !x;
  return gand<m>(static_cast<std::byte>(lhs), y);
}


/**
 * Scale the byte on the right by single garbled bit on the left.
 * 16 ciphertexts.
 */
template <Mode m>
Garbled::Byte<m> scale(const Garbled::Bit<m>& x, const Garbled::Byte<m>& y) {
  return broadcast<m>(x) & y;
}


/**
 * In-place logical AND.
 * 16 ciphertexts
 */
template <Mode m>
Garbled::Byte<m>& operator&=(Garbled::Byte<m>& x, const Garbled::Byte<m>& y) { x = x & y; return x; }


/**
 * G opens her full share of to E
 *
 * WARNING! Use with great care. Revealing a share of a garbling «X, X + xΔ» could reveal Δ.
 */
template <Mode m> Garbled::Byte<m> open(const Garbled::Byte<m>&);

#endif
