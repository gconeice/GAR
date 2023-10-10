#ifndef SGC_BIT_H__
#define SGC_BIT_H__

#include "Const.h"
#include "Role.h"

#include <cstdint>
#include <iostream>
#include <bitset>

namespace SGC{

template <Role role>
struct Bit;

template <>
struct Bit<Role::Speculate> {
 public:
  Bit();
  Bit(SGC::BitOwner owner = SGC::INTERMEDIATE) {
    if (owner == SGC::GARBLER || owner == SGC::EVALUATOR) { ++circuit_desc.nInp; }
  }

  static Bit<Role::Speculate> constant(bool b) noexcept;

  Bit<Role::Speculate> operator~() const { return *this; }
  Bit<Role::Speculate> operator&(const Bit<Role::Speculate>& rhs) { ++circuit_desc.nAnd; return *this; }
  Bit<Role::Speculate> operator^(const Bit<Role::Speculate>& rhs) { return *this; }
  void Reveal() { ++circuit_desc.nOut; }
};

template<>
struct Bit<Role::Cleartext> {
  bool val;
  Bit();
  Bit(bool _val);
  Bit<Role::Cleartext> operator~() const;
  Bit<Role::Cleartext> operator &(const Bit<Role::Cleartext>& rhs) const;
  Bit<Role::Cleartext> operator ^(const Bit<Role::Cleartext>& rhs) const;

  //friend std::ostream& operator<<(std::ostream& os, const Bit<Role::Cleartext>& b);
};

template<>
struct Bit<Role::Garbler> {
  std::bitset<128> wire;
  bool isConstant;
  Bit(bool getran, SGC::BitOwner owner = SGC::INTERMEDIATE, bool garbler_input = false);
  Bit(SGC::PRG &prg);
  Bit(std::bitset<128> _wire, bool _isConstant);
  Bit();
  static Bit<Role::Garbler> constant(bool b) noexcept;
  void Reveal();
  // TODO: more constructor to avoid useless random
  Bit<Role::Garbler> operator~() const;
  Bit<Role::Garbler> operator &(const Bit<Role::Garbler>& rhs) const;
  Bit<Role::Garbler> operator ^(const Bit<Role::Garbler>& rhs) const;
  Bit<Role::Garbler> AND(const Bit<Role::Garbler>& rhs, std::vector<KappaBitString> &gc_material) const;
};

// static Bit<Role::Garbler> zerogarbler, onegarbler;

template<>
struct Bit<Role::Evaluator> {
 public:
  std::bitset<128> wire;
  bool isConstant;
  Bit(bool input, SGC::BitOwner owner = SGC::INTERMEDIATE);
  Bit();
  Bit(std::bitset<128> _wire, bool _isConstant);
  static Bit<Role::Evaluator> constant(bool b) noexcept;
  void Reveal();
  Bit<Role::Evaluator> operator ~() const;
  Bit<Role::Evaluator> operator &(const Bit<Role::Evaluator>& rhs) const;
  Bit<Role::Evaluator> operator ^(const Bit<Role::Evaluator>& rhs) const;
  Bit<Role::Evaluator> AND(const Bit<Role::Evaluator>& rhs, std::vector<KappaBitString> &gc_material) const;
};

// Expanding the number of operators


template <Role R>
Bit<R> operator|(const Bit<R>& l, const Bit<R>& r) {
  auto out = ~l & ~r;
  out = ~out;
  return out;
}

template <Role R>
Bit<R> operator!=(const Bit<R>& l, const Bit<R>& r) { return l ^ r; }

template <Role R>
Bit<R> operator==(const Bit<R>& l, const Bit<R>& r) { return ~(l != r); }

template <Role R>
Bit<R> operator<=(const Bit<R>& l, const Bit<R>& r) { return ~l | r; }

template <Role R>
Bit<R> operator<(const Bit<R>& l, const Bit<R>& r) { return ~l & r; }

template <Role R>
Bit<R> operator>=(const Bit<R>& l, const Bit<R>& r) { return r <= l; }

template <Role R>
Bit<R> operator>(const Bit<R>& l, const Bit<R>& r) { return r < l; }

Bit<Role::Garbler> operator==(const Bit<Role::Garbler> &l, const bool &r);
Bit<Role::Evaluator> operator==(const Bit<Role::Evaluator> &l, const bool &r);

};

#endif