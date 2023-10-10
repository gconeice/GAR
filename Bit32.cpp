#include "Bit32.h"

namespace SGC {

Bit32<Role::Garbler>::Bit32(std::bitset<BIT32::SIZE> constant) {
    for (int i = 0; i < BIT32::SIZE; i++) bits[i] = Bit<Role::Garbler>::constant(constant[i]);
}
    
Bit32<Role::Garbler>::Bit32(bool getran) {
    //bits[1] = Bit<Role::Cleartext>(1);
    if (getran)
        for (int i = 0; i < BIT32::SIZE; i++) bits[i] = Bit<Role::Garbler>(true);
}

Bit32<Role::Garbler>::Bit32() {        
}    

Bit32<Role::Garbler>::Bit32(PRG &prg_) {
    for (int i = 0; i < BIT32::SIZE; i++) bits[i] = Bit<Role::Garbler>(prg_);
}

/*
Bit(bool getran, SGC::BitOwner owner = SGC::INTERMEDIATE, bool garbler_input = false);
Bit<Role::Garbler>::Bit(bool getran, SGC::BitOwner owner, bool garbler_input) {
  if (getran) {
    wire = SGC::prg();
    if (owner == SGC::GARBLER) {
      SGC::garbler_inputs.push_back(garbler_input ? (wire ^ SGC::delta) : wire);
    }
    if (owner == SGC::EVALUATOR) {
      SGC::evaluator_inputs_false.push_back(wire);
      SGC::evaluator_inputs_true.push_back(wire ^ SGC::delta);
    }
    // std::cout << "[DEBUG] Garbled Label \nfalse: "  << wire << " \ntrue: " << (wire^SGC::delta) << std::endl;
  }
  isConstant = 0;
}
*/

Bit32<Role::Garbler>::Bit32(SGC::BitOwner owner, uint32_t garbler_input) {
    for (int i = 0; i < BIT32::SIZE; i++, garbler_input >>= 1) bits[i] = Bit<Role::Garbler>(true, owner, garbler_input & 1);
    // if (owner == SGC::GARBLER) {
    //     for (int i = 0; i < BIT32::SIZE; i++, garbler_input >>= 1) bits[i] = Bit<Role::Garbler>(true, SGC::GARBLER, garbler_input & 1);
    // } else if (owner == SGC::EVALUATOR) {
    //     for (int i = 0; i < BIT32::SIZE; i++) bits[i] = Bit<Role::Garbler>(true, SGC::EVALUATOR);
    // }
}

void Bit32<Role::Garbler>::Reveal() {

    // Try Function in Bit
    for (int i = 0; i < BIT32::SIZE; i++) bits[i].Reveal();
    return;

    // Change to OT
    for (int i = 0; i < BIT32::SIZE; i++) 
        std::cout << bits[i].wire << ' ' << (bits[i].wire^SGC::delta) << std::endl;
}

Bit32<Role::Evaluator>::Bit32() {
}

Bit32<Role::Evaluator>::Bit32(bool input) {
    if (input)
        for (int i = 0; i < BIT32::SIZE; i++) bits[i] = Bit<Role::Evaluator>(true);
}

// const Bit32?
Bit32<Role::Evaluator>::Bit32(std::bitset<BIT32::SIZE> constant) {
    for (int i = 0; i < BIT32::SIZE; i++) bits[i] = Bit<Role::Evaluator>::constant(constant[i]);
}    

Bit32<Role::Evaluator>::Bit32(uint32_t constant) {
    // change to OTs
    std::bitset<128> labels[2];
    for (int i = 0; i < BIT32::SIZE; i++) 
    {
        std::cin >> labels[0] >> labels[1];
        bits[i] = Bit<Role::Evaluator>(labels[(constant >> i) & 1], 0);
    }
    // for (int i = 0; i < BIT32::SIZE; i++) std::cout << "DEBUG " << bits[i].wire << std::endl;
}

Bit32<Role::Evaluator>::Bit32(SGC::BitOwner owner) {
    for (int i = 0; i < BIT32::SIZE; i++) bits[i] = Bit<Role::Evaluator>(true, owner);
}

uint32_t Bit32<Role::Evaluator>::Reveal() {

    // Try Reveal Bit Function
    for (int i = 0; i < BIT32::SIZE; i++) bits[i].Reveal();
    return 0;

    // Change to OTs
    uint32_t res = 0;
    for (int i = 0; i < BIT32::SIZE; i++) {
        if (bits[i].isConstant) {
            if (bits[i].wire != 0) {
                res = res ^ (1 << i);
                std::cout << 1;
            } else {
                std::cout << 0;
            }
            continue;
        }
        std::bitset<128> labels[2];
        std::cin >> labels[0] >> labels[1];
        if (bits[i].wire == labels[0]) {
            std::cout << 0;
        } else if (bits[i].wire == labels[1]) {
            res = res ^ (1 << i);
            std::cout << 1;
        } else {
            std::cout << "ERROR" << std::endl;
            exit(255);
        }
    }
    std::cout << std::endl;
    return res;
}

Bit32<Role::Garbler> Bit32<Role::Garbler>::operator &(const Bit32<Role::Garbler> &rhs) const {
    Bit32<Role::Garbler> res;
    for (int i = 0; i < BIT32::SIZE; i++) res.bits[i] = bits[i] & rhs.bits[i];
    return res;
}

Bit32<Role::Garbler> Bit32<Role::Garbler>::operator ^(const Bit32<Role::Garbler> &rhs) const {
    Bit32<Role::Garbler> res;
    for (int i = 0; i < BIT32::SIZE; i++) res.bits[i] = bits[i] ^ rhs.bits[i];
    return res;
}

Bit32<Role::Garbler> Bit32<Role::Garbler>::operator ~() const {
    Bit32<Role::Garbler> res;
    for (int i = 0; i < BIT32::SIZE; i++) res.bits[i] = ~bits[i];
    return res;
}

// unsigned
Bit32<Role::Garbler> Bit32<Role::Garbler>::operator +(const Bit32<Role::Garbler> &rhs) const {
    Bit32<Role::Garbler> res;

    res.bits[0] = bits[0] ^ rhs.bits[0];
    Bit<Role::Garbler> carry = bits[0] & rhs.bits[0];
    for (int i = 1; i < BIT32::SIZE; i++) {
        res.bits[i] = bits[i] ^ rhs.bits[i] ^ carry;
        carry = carry ^ ((bits[i] ^ carry) & (rhs.bits[i] ^ carry));
    }

    return res;    
}

Bit32<Role::Garbler> Bit32<Role::Garbler>::Add(const Bit32& rhs, std::vector<KappaBitString> &gc_material) const {
  Bit32<Role::Garbler> res;

  res.bits[0] = bits[0] ^ rhs.bits[0];
  Bit<Role::Garbler> carry = bits[0].AND(rhs.bits[0], gc_material);
  for (int i = 1; i < BIT32::SIZE; i++) {
    res.bits[i] = bits[i] ^ rhs.bits[i] ^ carry;
    carry = carry ^ ((bits[i] ^ carry).AND(rhs.bits[i] ^ carry, gc_material));
  }
  return res;
}

Bit32<Role::Garbler> Bit32<Role::Garbler>::Sub(const Bit32& rhs, std::vector<KappaBitString> &gc_material) const {
  Bit32<Role::Garbler> res;

  Bit<Role::Garbler> carry;
  carry.wire = SGC::delta;

  for (int i = 0; i < BIT32::SIZE; i++) {
    res.bits[i] = bits[i] ^ (~rhs.bits[i]) ^ carry;
    carry = carry ^ ((bits[i] ^ carry).AND((~rhs.bits[i]) ^ carry, gc_material));
  }

  return res;
}

Bit32<Role::Garbler> Bit32<Role::Garbler>::Sub(const Bit32& rhs) const {
  Bit32<Role::Garbler> res;

  Bit<Role::Garbler> carry;
  carry.wire = SGC::delta;

  for (int i = 0; i < BIT32::SIZE; i++) {
    res.bits[i] = bits[i] ^ (~rhs.bits[i]) ^ carry;
    carry = carry ^ ((bits[i] ^ carry) & ((~rhs.bits[i]) ^ carry));
  }

  return res;
}

// Set signed less than (1), unsigned less than (0)
Bit32<Role::Garbler> Bit32<Role::Garbler>::Cmp(const Bit32& rhs, std::vector<KappaBitString> &gc_material) const {
  Bit32<Role::Garbler> subres;

  Bit<Role::Garbler> carry;
  carry.wire = SGC::delta;

  for (int i = 0; i < BIT32::SIZE; i++) {
    subres.bits[i] = bits[i] ^ (~rhs.bits[i]) ^ carry;
    carry = carry ^ ((bits[i] ^ carry).AND((~rhs.bits[i]) ^ carry, gc_material));
  }

  Bit32<Role::Garbler> res;
  //res.bits[0] = (*this == rhs);
  carry = ~carry;
  res.bits[0].wire = carry.wire;
  // Potential OPT here?
  auto neg_pos = bits[BIT32::SIZE-1].AND((~rhs.bits[BIT32::SIZE-1]), gc_material);
  auto same_sign = ~(bits[BIT32::SIZE-1] ^ rhs.bits[BIT32::SIZE-1]);
  res.bits[1] = neg_pos ^ same_sign.AND(subres.bits[BIT32::SIZE-1], gc_material);
  //res.bits[2] = bits[BIT2::SIZE-1]

  return res;
}

// Set signed less than (1), unsigned less than (0)
Bit32<Role::Garbler> Bit32<Role::Garbler>::Cmp(const Bit32& rhs) const {
  Bit32<Role::Garbler> subres;

  Bit<Role::Garbler> carry;
  carry.wire = SGC::delta;

  for (int i = 0; i < BIT32::SIZE; i++) {
    subres.bits[i] = bits[i] ^ (~rhs.bits[i]) ^ carry;
    carry = carry ^ ((bits[i] ^ carry) & ((~rhs.bits[i]) ^ carry));
  }

  Bit32<Role::Garbler> res;
  //res.bits[0] = (*this == rhs);
  carry = ~carry;
  res.bits[0].wire = carry.wire;
  // Potential OPT here?
  auto neg_pos = bits[BIT32::SIZE-1] & ((~rhs.bits[BIT32::SIZE-1]));
  auto same_sign = ~(bits[BIT32::SIZE-1] ^ rhs.bits[BIT32::SIZE-1]);
  res.bits[1] = neg_pos ^ (same_sign & (subres.bits[BIT32::SIZE-1]));
  //res.bits[2] = bits[BIT2::SIZE-1]

  return res;
}

// result saved in the positon (0)
Bit32<Role::Garbler> Bit32<Role::Garbler>::Eq(const Bit32& rhs, std::vector<KappaBitString> &gc_material) const {
    Bit32<Role::Garbler> res;

    res.bits[0] = (bits[0] == rhs.bits[0]);
    for (int i = 1; i < BIT32::SIZE; i++) 
        res.bits[0] = res.bits[0].AND((bits[i] == rhs.bits[i]), gc_material);

    return res;
}

// result saved in the positon (0)
Bit32<Role::Garbler> Bit32<Role::Garbler>::Eq(const Bit32& rhs) const {
    Bit32<Role::Garbler> res;

    res.bits[0] = (bits[0] == rhs.bits[0]);
    for (int i = 1; i < BIT32::SIZE; i++) 
        res.bits[0] = res.bits[0] & ((bits[i] == rhs.bits[i]));

    return res;
}

Bit32<Role::Garbler> Bit32<Role::Garbler>::And_Bit(const Bit32& rhs, std::vector<KappaBitString> &gc_material, const int &bid, const bool &not_mark) const {
    Bit32<Role::Garbler> res;

    auto andbit = not_mark ? ~rhs.bits[bid] : rhs.bits[bid];
    for (int i = 0; i < BIT32::SIZE; i++) 
        res.bits[i] = bits[i].AND(andbit, gc_material);

    return res;
}

Bit32<Role::Garbler> Bit32<Role::Garbler>::And_Bit(const Bit32& rhs, const int &bid, const bool &not_mark) const {
    Bit32<Role::Garbler> res;

    auto andbit = not_mark ? ~rhs.bits[bid] : rhs.bits[bid];
    for (int i = 0; i < BIT32::SIZE; i++) 
        res.bits[i] = bits[i] & andbit;

    return res;
}

Bit32<Role::Garbler> Bit32<Role::Garbler>::operator <<(const uint32_t& offset) const {

    if (offset >= BIT32::SIZE) {
        Bit32<Role::Garbler> res;
        for (uint32_t i = 0; i < BIT32::SIZE; i++) res.bits[i] = Bit<Role::Garbler>::constant(0);
        return res;
    }

    if (offset == 0) return *this;

    Bit32<Role::Garbler> res;
    for (uint32_t i = BIT32::SIZE-1; i >= offset; i--) res.bits[i] = bits[i - offset];
    for (uint32_t i = 0; i < offset; i++) res.bits[i] = Bit<Role::Garbler>::constant(0);
    return res;
}

Bit<Role::Garbler> operator==(const Bit32<Role::Garbler> &lhs, const uint32_t &rhs) {
    uint32_t tmp = rhs;
    Bit<Role::Garbler> res = lhs.bits[0] == (tmp & 1);
    tmp >>= 1;
    for (int i = 1; i < BIT32::SIZE; i++, tmp >>= 1) res = res & (lhs.bits[i] == (tmp & 1));
    return res;
}

Bit<Role::Evaluator> operator==(const Bit32<Role::Evaluator> &lhs, const uint32_t &rhs) {
    uint32_t tmp = rhs;
    Bit<Role::Evaluator> res = lhs.bits[0] == (tmp & 1);
    tmp >>= 1;
    for (int i = 1; i < BIT32::SIZE; i++, tmp >>= 1) res = res & (lhs.bits[i] == (tmp & 1));
    return res;
}

Bit32<Role::Garbler> Bit32<Role::Garbler>::operator *(const Bit32<Role::Garbler> &rhs) const {

    // TODO: optimize it
    std::bitset<BIT32::SIZE> zero = 0;
    Bit32<Role::Garbler> res(zero);
    // TODO: constant Bit32?

    for (int i = 0; i < BIT32::SIZE; i++) {
        res = res + ( ((*this) << i) & rhs.bits[i] );
    }

    return res;
}

Bit32<Role::Garbler> Bit32<Role::Garbler>::Multiply(const Bit32& rhs, std::vector<KappaBitString> &gc_material) const {
  // TODO: optimize it
  std::bitset<BIT32::SIZE> zero = 0;
  Bit32<Role::Garbler> res(zero);
  // TODO: constant Bit32?

  for (int i = 0; i < BIT32::SIZE; i++) {
    res = res.Add(AND<Role::Garbler>(((*this) << i), rhs.bits[i], gc_material), gc_material);
  }

  return res;
}

Bit32<Role::Evaluator> Bit32<Role::Evaluator>::operator &(const Bit32<Role::Evaluator> &rhs) const {
    Bit32<Role::Evaluator> res;
    for (int i = 0; i < BIT32::SIZE; i++) res.bits[i] = bits[i] & rhs.bits[i];
    return res;
}

Bit32<Role::Evaluator> Bit32<Role::Evaluator>::operator ^(const Bit32<Role::Evaluator> &rhs) const {
    Bit32<Role::Evaluator> res;
    for (int i = 0; i < BIT32::SIZE; i++) res.bits[i] = bits[i] ^ rhs.bits[i];
    return res;
}

Bit32<Role::Evaluator> Bit32<Role::Evaluator>::operator ~() const {
    Bit32<Role::Evaluator> res;
    for (int i = 0; i < BIT32::SIZE; i++) res.bits[i] = ~bits[i];
    return res;
}

// unsigned
Bit32<Role::Evaluator> Bit32<Role::Evaluator>::operator +(const Bit32<Role::Evaluator> &rhs) const {
    Bit32<Role::Evaluator> res;

    res.bits[0] = bits[0] ^ rhs.bits[0];
    Bit<Role::Evaluator> carry = bits[0] & rhs.bits[0];
    for (int i = 1; i < BIT32::SIZE; i++) {
        res.bits[i] = bits[i] ^ rhs.bits[i] ^ carry;
        carry = carry ^ ((bits[i] ^ carry) & (rhs.bits[i] ^ carry));
    }

    return res;    
}

Bit32<Role::Evaluator> Bit32<Role::Evaluator>::Add(const Bit32& rhs, std::vector<KappaBitString> &gc_material) const {
  Bit32<Role::Evaluator> res;

  res.bits[0] = bits[0] ^ rhs.bits[0];
  Bit<Role::Evaluator> carry = bits[0].AND(rhs.bits[0], gc_material);
  for (int i = 1; i < BIT32::SIZE; i++) {
    res.bits[i] = bits[i] ^ rhs.bits[i] ^ carry;
    carry = carry ^ ((bits[i] ^ carry).AND(rhs.bits[i] ^ carry, gc_material));
  }
  return res;
}

Bit32<Role::Evaluator> Bit32<Role::Evaluator>::Sub(const Bit32& rhs, std::vector<KappaBitString> &gc_material) const {
  Bit32<Role::Evaluator> res;

  Bit<Role::Evaluator> carry;
  carry.wire = 0;

  for (int i = 0; i < BIT32::SIZE; i++) {
    res.bits[i] = bits[i] ^ (~rhs.bits[i]) ^ carry;
    carry = carry ^ ((bits[i] ^ carry).AND((~rhs.bits[i]) ^ carry, gc_material));
  }
  return res;
}

Bit32<Role::Evaluator> Bit32<Role::Evaluator>::Sub(const Bit32& rhs) const {
  Bit32<Role::Evaluator> res;

  Bit<Role::Evaluator> carry;
  carry.wire = 0;

  for (int i = 0; i < BIT32::SIZE; i++) {
    res.bits[i] = bits[i] ^ (~rhs.bits[i]) ^ carry;
    carry = carry ^ ((bits[i] ^ carry) & ((~rhs.bits[i]) ^ carry));
  }
  return res;
}

Bit32<Role::Evaluator> Bit32<Role::Evaluator>::Cmp(const Bit32& rhs, std::vector<KappaBitString> &gc_material) const {
  Bit32<Role::Evaluator> subres;

  Bit<Role::Evaluator> carry;
  carry.wire = 0;

  for (int i = 0; i < BIT32::SIZE; i++) {
    subres.bits[i] = bits[i] ^ (~rhs.bits[i]) ^ carry;
    carry = carry ^ ((bits[i] ^ carry).AND((~rhs.bits[i]) ^ carry, gc_material));
  }

  Bit32<Role::Evaluator> res;
  //res.bits[0] = (*this == rhs);
  carry = ~carry;
  res.bits[0].wire = carry.wire;
  // Potential OPT here?
  auto neg_pos = bits[BIT32::SIZE-1].AND((~rhs.bits[BIT32::SIZE-1]), gc_material);
  auto same_sign = ~(bits[BIT32::SIZE-1] ^ rhs.bits[BIT32::SIZE-1]);
  res.bits[1] = neg_pos ^ same_sign.AND(subres.bits[BIT32::SIZE-1], gc_material);
  //res.bits[2] = bits[BIT2::SIZE-1]

  return res;
}

Bit32<Role::Evaluator> Bit32<Role::Evaluator>::Cmp(const Bit32& rhs) const {
  Bit32<Role::Evaluator> subres;

  Bit<Role::Evaluator> carry;
  carry.wire = 0;

  for (int i = 0; i < BIT32::SIZE; i++) {
    subres.bits[i] = bits[i] ^ (~rhs.bits[i]) ^ carry;
    carry = carry ^ ((bits[i] ^ carry) & ((~rhs.bits[i]) ^ carry));
  }

  Bit32<Role::Evaluator> res;
  //res.bits[0] = (*this == rhs);
  carry = ~carry;
  res.bits[0].wire = carry.wire;
  // Potential OPT here?
  auto neg_pos = bits[BIT32::SIZE-1] & ((~rhs.bits[BIT32::SIZE-1]));
  auto same_sign = ~(bits[BIT32::SIZE-1] ^ rhs.bits[BIT32::SIZE-1]);
  res.bits[1] = neg_pos ^ (same_sign & (subres.bits[BIT32::SIZE-1]));
  //res.bits[2] = bits[BIT2::SIZE-1]

  return res;
}

// result saved in the positon (0)
Bit32<Role::Evaluator> Bit32<Role::Evaluator>::Eq(const Bit32& rhs, std::vector<KappaBitString> &gc_material) const {
    Bit32<Role::Evaluator> res;

    res.bits[0] = (bits[0] == rhs.bits[0]);
    for (int i = 1; i < BIT32::SIZE; i++) 
        res.bits[0] = res.bits[0].AND((bits[i] == rhs.bits[i]), gc_material);

    return res;
}

// result saved in the positon (0)
Bit32<Role::Evaluator> Bit32<Role::Evaluator>::Eq(const Bit32& rhs) const {
    Bit32<Role::Evaluator> res;

    res.bits[0] = (bits[0] == rhs.bits[0]);
    for (int i = 1; i < BIT32::SIZE; i++) 
        res.bits[0] = res.bits[0] & ((bits[i] == rhs.bits[i]));

    return res;
}

Bit32<Role::Evaluator> Bit32<Role::Evaluator>::And_Bit(const Bit32& rhs, std::vector<KappaBitString> &gc_material, const int &bid, const bool &not_mark) const {
    Bit32<Role::Evaluator> res;

    auto andbit = not_mark ? ~rhs.bits[bid] : rhs.bits[bid];
    for (int i = 0; i < BIT32::SIZE; i++) 
        res.bits[i] = bits[i].AND(andbit, gc_material);

    return res;
}

Bit32<Role::Evaluator> Bit32<Role::Evaluator>::And_Bit(const Bit32& rhs, const int &bid, const bool &not_mark) const {
    Bit32<Role::Evaluator> res;

    auto andbit = not_mark ? ~rhs.bits[bid] : rhs.bits[bid];
    for (int i = 0; i < BIT32::SIZE; i++) 
        res.bits[i] = bits[i] & andbit;

    return res;
}

Bit32<Role::Evaluator> Bit32<Role::Evaluator>::operator <<(const uint32_t& offset) const {

    if (offset >= BIT32::SIZE) {
        Bit32<Role::Evaluator> res;
        for (uint32_t i = 0; i < BIT32::SIZE; i++) res.bits[i] = Bit<Role::Evaluator>::constant(0);
        return res;
    }

    if (offset == 0) return *this;

    Bit32<Role::Evaluator> res;
    for (uint32_t i = BIT32::SIZE-1; i >= offset; i--) res.bits[i] = bits[i - offset];
    for (uint32_t i = 0; i < offset; i++) res.bits[i] = Bit<Role::Evaluator>::constant(0);
    return res;
}

/*
Bit32<Role::Evaluator> operator &(const Bit32<Role::Evaluator> &lhs, const Bit<Role::Evaluator> &rhs) {
    Bit32<Role::Evaluator> res;
    for (int i = 0; i < BIT32::SIZE; i++) res.bits[i] = lhs.bits[i] & rhs;
    return res;
}

Bit32<Role::Evaluator> operator &(const Bit<Role::Evaluator> &lhs, const Bit32<Role::Evaluator> &rhs) {
    Bit32<Role::Evaluator> res;
    for (int i = 0; i < BIT32::SIZE; i++) res.bits[i] = rhs.bits[i] & lhs;
    return res;
}
*/

Bit32<Role::Evaluator> Bit32<Role::Evaluator>::operator *(const Bit32<Role::Evaluator> &rhs) const {

    // TODO: optimize it
    std::bitset<BIT32::SIZE> zero = 0;
    Bit32<Role::Evaluator> res(zero);
    // TODO: constant Bit32?

    for (int i = 0; i < BIT32::SIZE; i++) {
        res = res + ( ((*this) << i) & rhs.bits[i] );
    }

    return res;
}

Bit32<Role::Evaluator> Bit32<Role::Evaluator>::Multiply(const Bit32& rhs, std::vector<KappaBitString> &gc_material) const {
  // TODO: optimize it
  std::bitset<BIT32::SIZE> zero = 0;
  Bit32<Role::Evaluator> res(zero);
  // TODO: constant Bit32?

  for (int i = 0; i < BIT32::SIZE; i++) {
    res = res.Add(AND<Role::Evaluator>(((*this) << i), rhs.bits[i], gc_material), gc_material);
  }

  return res;
}


void Bit32<Role::Garbler>::copy_wires(std::vector<KappaBitString> &input) {
    for (int i = 0; i < BIT32::SIZE; i++) input.push_back(bits[i].wire);
}

void Bit32<Role::Evaluator>::copy_wires(std::vector<KappaBitString> &input) {    
    for (int i = 0; i < BIT32::SIZE; i++) input.push_back(bits[i].wire);
}


}; // namespace SGC
