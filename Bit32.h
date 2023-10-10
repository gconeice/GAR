#ifndef SGC_BIT32_H__
#define SGC_BIT32_H__

#include "Bit.h"
#include <cstring>
#include <vector>
#include <bitset>
#include <variant>

namespace BIT32 {
    const int SIZE = 32;
};

namespace SGC {
template <Role role>
struct Bit32;

template<>
struct Bit32<Role::Garbler> {
    //template<Role role>
    //std::vector<std::variant< Bit<Role::Garbler>, Bit<Role::Cleartext> >> bits;
    Bit<Role::Garbler> bits[BIT32::SIZE];
    Bit32(std::bitset<BIT32::SIZE> constant);
    Bit32(bool getran);
    Bit32(PRG &prg);
    Bit32();
    Bit32(SGC::BitOwner owner, uint32_t garbler_input = 0);
    Bit32 operator ~() const;
    Bit32 operator &(const Bit32& rhs) const;
    Bit32 operator ^(const Bit32& rhs) const;    
    Bit32 operator +(const Bit32& rhs) const;
    Bit32 Add(const Bit32& rhs, std::vector<KappaBitString> &gc_material) const;
    Bit32 Sub(const Bit32& rhs, std::vector<KappaBitString> &gc_material) const;
    Bit32 Sub(const Bit32& rhs) const;
    Bit32 Cmp(const Bit32& rhs, std::vector<KappaBitString> &gc_material) const;
    Bit32 Cmp(const Bit32& rhs) const;
    Bit32 Eq(const Bit32& rhs, std::vector<KappaBitString> &gc_material) const;
    Bit32 Eq(const Bit32& rhs) const;
    Bit32 And_Bit(const Bit32& rhs, std::vector<KappaBitString> &gc_material, const int &bid, const bool &not_mark) const;
    Bit32 And_Bit(const Bit32& rhs, const int &bid, const bool &not_mark) const;
    Bit32 operator *(const Bit32& rhs) const;
    Bit32 Multiply(const Bit32& rhs, std::vector<KappaBitString> &gc_material) const;
    Bit32 operator <<(const uint32_t& offset) const;
    //Bit32 operator &(const Bit& rhs) const;
    //Bit operator ==(const Bit32& rhs) const;

    void Reveal();
    void copy_wires(std::vector<KappaBitString> &input);

};



template<Role R>
Bit32<R> operator &(const Bit32<R> &lhs, const Bit<R> &rhs) {
    Bit32<R> res;
    for (int i = 0; i < BIT32::SIZE; i++) res.bits[i] = lhs.bits[i] & rhs;
    return res;
}

template<Role R>
Bit32<R> AND(const Bit32<R> &lhs, const Bit<R> &rhs, std::vector<KappaBitString> &gc_material) {
  Bit32<R> res;
  for (int i = 0; i < BIT32::SIZE; i++) res.bits[i] = lhs.bits[i].AND(rhs, gc_material);
  return res;
}

template<Role R>
Bit32<R> operator &(const Bit<R> &lhs, const Bit32<R> &rhs) {
    Bit32<R> res;
    for (int i = 0; i < BIT32::SIZE; i++) res.bits[i] = rhs.bits[i] & lhs;
    return res;
}

template<Role R>
Bit<R> operator==(const Bit32<R> &lhs, const Bit32<R> &rhs) {
    Bit<R> res = (lhs.bits[0] == rhs.bits[0]);
    for (int i = 1; i < BIT32::SIZE; i++) res = res & (lhs.bits[i] == rhs.bits[i]);
    return res;
}

Bit<Role::Garbler> operator==(const Bit32<Role::Garbler> &lhs, const uint32_t &rhs);

Bit<Role::Evaluator> operator==(const Bit32<Role::Evaluator> &lhs, const uint32_t &rhs);

template<>
struct Bit32<Role::Evaluator> {
    Bit<Role::Evaluator> bits[BIT32::SIZE];
    Bit32();
    Bit32(bool input);
    // const Bit32?
    Bit32(std::bitset<BIT32::SIZE> constant);
    Bit32(uint32_t constant);
    Bit32(SGC::BitOwner owner);
    Bit32 operator ~() const;
    Bit32 operator &(const Bit32& rhs) const;
    Bit32 operator ^(const Bit32& rhs) const;    
    Bit32 operator +(const Bit32& rhs) const;
    Bit32 Add(const Bit32& rhs, std::vector<KappaBitString> &gc_material) const;
    Bit32 Sub(const Bit32& rhs, std::vector<KappaBitString> &gc_material) const;
    Bit32 Sub(const Bit32& rhs) const;
    Bit32 Cmp(const Bit32& rhs, std::vector<KappaBitString> &gc_material) const;
    Bit32 Cmp(const Bit32& rhs) const;
    Bit32 Eq(const Bit32& rhs, std::vector<KappaBitString> &gc_material) const;
    Bit32 Eq(const Bit32& rhs) const;
    Bit32 And_Bit(const Bit32& rhs, std::vector<KappaBitString> &gc_material, const int &bid, const bool &not_mark) const;
    Bit32 And_Bit(const Bit32& rhs, const int &bid, const bool &not_mark) const;
    Bit32 operator *(const Bit32& rhs) const;
    Bit32 Multiply(const Bit32& rhs, std::vector<KappaBitString> &gc_material) const;
    Bit32 operator <<(const uint32_t& offset) const;

    uint32_t Reveal();
    void copy_wires(std::vector<KappaBitString> &input);
};

inline Bit32<Role::Garbler> GbGetImm(const uint32_t &imm) {
    Bit32<Role::Garbler> res;
    for (int i = 0; i < BIT32::SIZE; i++) 
        if ((imm >> i) & 1) res.bits[i].wire = SGC::delta;
    return res;
}

//#include "Bit32.hpp"
};

#endif