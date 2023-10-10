#include "Bit.h"
#include "Const.h"


namespace SGC {


Bit<Role::Speculate>::Bit() {
}

Bit<Role::Cleartext>::Bit() {    
}    

Bit<Role::Cleartext>::Bit(bool _val) {
    val = _val;
}

// TODO: resolve this bug
/*
std::ostream& Bit<Role::Cleartext>::operator<<(std::ostream& os, const Bit<Role::Cleartext>& b) {
    os << b.val;
    return os;
}
*/


Bit<Role::Garbler>::Bit(bool getran, SGC::BitOwner owner, bool garbler_input) {
  if (getran) {
    wire = SGC::prg();
    // TODO: think about how to glue everything (later)
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

Bit<Role::Garbler>::Bit(SGC::PRG &prg_) {
  wire = prg_();
  // TODO: save to somewhere  
  isConstant = 0;
}

Bit<Role::Garbler>::Bit(std::bitset<128> _wire, bool _isConstant) {
    wire = _wire;
    isConstant = _isConstant;
}

Bit<Role::Garbler>::Bit() {
    isConstant = 0;
}

Bit<Role::Garbler> Bit<Role::Garbler>::constant(bool b) noexcept {
    return b ? Bit(SGC::delta, 1) : Bit(0, 1);
}

Bit<Role::Evaluator>::Bit(bool input, SGC::BitOwner owner) {
  if (input) {
    if (owner == SGC::GARBLER) {
      wire = SGC::garbler_inputs[SGC::garbler_input_idx];
      SGC::garbler_input_idx++;
    }
    if (owner == SGC::EVALUATOR) {
      wire = SGC::evaluator_inputs[SGC::evaluator_input_idx];
      SGC::evaluator_input_idx++;
    }
    // std::cout << "[DEBUG] Garbled Label \n: "  << wire << std::endl;
  }
  isConstant = 0;
}

void Bit<Role::Garbler>::Reveal() {
  // output wire idx takes two spaces (so 128/2)
  unsigned int space_per_bitset = 128 / 2;
  unsigned int table_idx = SGC::output_wire_idx / space_per_bitset;
  unsigned int bitset_idx = 2 * (SGC::output_wire_idx % space_per_bitset);
  if (SGC::output_wire_idx % space_per_bitset == 0) {
    SGC::output_decoding_table.push_back(std::bitset<128>());
  }
  // Add entries into output decoding table
  if (static_cast<bool>(wire[0]) == false) {
    SGC::output_decoding_table[table_idx][bitset_idx] =
        SGC::HashHalfGates(wire, SGC::output_wire_idx)[0] ^ false;
    SGC::output_decoding_table[table_idx][bitset_idx + 1] =
        SGC::HashHalfGates(wire ^ SGC::delta, SGC::output_wire_idx)[0] ^ true;
  } else {
    SGC::output_decoding_table[table_idx][bitset_idx] =
        SGC::HashHalfGates(wire ^ SGC::delta, SGC::output_wire_idx)[0] ^ true;
    SGC::output_decoding_table[table_idx][bitset_idx + 1] =
        SGC::HashHalfGates(wire, SGC::output_wire_idx)[0] ^ false;
  }
  SGC::output_wire_idx++;
}

void Bit<Role::Evaluator>::Reveal() {
  // output wire idx takes two spaces (so 128/2)
  unsigned int space_per_bitset = 128 / 2;
  unsigned int table_idx = SGC::output_wire_idx / space_per_bitset;
  unsigned int bitset_idx = 2 * (SGC::output_wire_idx % space_per_bitset);
  // Retrieve entry from the output decoding table
  if (static_cast<bool>(wire[0]) == false) {
    SGC::result_bits.push_back(SGC::output_decoding_table[table_idx][bitset_idx] ^ SGC::HashHalfGates(wire, SGC::output_wire_idx)[0]);
  } else {
    SGC::result_bits.push_back(SGC::output_decoding_table[table_idx][bitset_idx + 1] ^ SGC::HashHalfGates(wire, SGC::output_wire_idx)[0]);
  }
  SGC::output_wire_idx++;
}

Bit<Role::Evaluator>::Bit() {
    isConstant = 0;
}

Bit<Role::Evaluator>::Bit(std::bitset<128> _wire, bool _isConstant) {
    wire = _wire;
    isConstant = _isConstant;
}

Bit<Role::Speculate> Bit<Role::Speculate>::constant(bool b) noexcept {
  return Bit(SGC::INTERMEDIATE); // make sure it does not increase input count
}


Bit<Role::Evaluator> Bit<Role::Evaluator>::constant(bool b) noexcept {
    return b ? Bit(1, 1) : Bit(0, 1);
}

Bit<Role::Cleartext> Bit<Role::Cleartext>::operator &(const Bit<Role::Cleartext> &rhs) const {
    Bit<Role::Cleartext> res;
    res.val = val & rhs.val;
    return res;
}

// AND-GATE: Garbler
Bit<Role::Garbler> Bit<Role::Garbler>::operator &(const Bit<Role::Garbler> &rhs) const {
    // std::cout << "Executing AND Garbler... " << std::endl;

    if (isConstant && rhs.isConstant) return Bit<Role::Garbler>(wire & rhs.wire, 1);
    if (isConstant) return wire == 0 ? Bit<Role::Garbler>::constant(0) : rhs;
    if (rhs.isConstant) return rhs.wire == 0 ? Bit<Role::Garbler>::constant(0) : *this;

    Bit<Role::Garbler> res;

    // Half gates Technique
    // v_a & v_b = (v_a ^ r ^ r) & v_b
    // v_a & v_b = [v_a & (r ^ v_b)] ^ [v_a & r]
    // Algorithm: page 9 of half-gates paper

    //const auto nonce0 = std::bitset<128> { SGC::and_gate_id };
    //const auto nonce1 = std::bitset<128> { SGC::and_gate_id + 1 };
    const auto nonce0 = std::bitset<128> { SGC::material_id };
    const auto nonce1 = std::bitset<128> { SGC::material_id + 1 };

    // Generate 2 Garbled Rows

    const auto ha0 = SGC::HashHalfGates(SGC::Find_Wire(wire, false), nonce0);
    const auto ha1 = SGC::HashHalfGates(SGC::Find_Wire(wire, true), nonce0);

    const auto hb0 = SGC::HashHalfGates(SGC::Find_Wire(rhs.wire, false), nonce1);
    const auto hb1 = SGC::HashHalfGates(SGC::Find_Wire(rhs.wire, true), nonce1);

    //SGC::material.push_back(rhs.wire[0] ? (ha0 ^ ha1 ^ SGC::delta) : (ha0 ^ ha1));
    //SGC::material.push_back(hb0 ^ hb1 ^ wire);
    SGC::material_buffer.push_back(rhs.wire[0] ? (ha0 ^ ha1 ^ SGC::delta) : (ha0 ^ ha1));
    SGC::material_buffer.push_back(hb0 ^ hb1 ^ wire);

#ifdef GARDEBUG
    std::cout << SGC::material[SGC::and_gate_id] << std::endl;
    std::cout << SGC::material[SGC::and_gate_id+1] << std::endl;
#endif

    //const auto g = wire[0] ? (ha0 ^ SGC::material[SGC::and_gate_id]) : ha0;
    //const auto e = rhs.wire[0] ? (hb0 ^ SGC::material[SGC::and_gate_id + 1] ^ wire) : hb0;
    const auto g = wire[0] ? (ha0 ^ SGC::material_buffer[SGC::material_id]) : ha0;
    const auto e = rhs.wire[0] ? (hb0 ^ SGC::material_buffer[SGC::material_id + 1] ^ wire) : hb0;

    res.wire = g ^ e; // whether res.wire = f(in1.wire, in2.wire, delta) // RO?

    //SGC::and_gate_id += 2;
    SGC::material_id += 2;
    SGC::buffer_idx += 2;



    // Unoptimized (standard GC)
    // Generate 4 Garbled Rows
    // size_t number_rows = 4;

    /*
    // Change the type accordingly
    std::bitset<128> garbled_table[number_rows];
    for (int va = 0; va < 2; va++) {
      for (int vb = 0; vb < 2; vb++) {
        // wire[0] are colors for permute-and-point
        // pos is the position in the garbled circuit
        int pos = 2 * (wire[0] ^ va) + (rhs.wire[0] ^ vb);
        // Label on the output wire (C in Mike Rosulek's video)
        std::bitset<128> output_wire = SGC::Find_Wire(res.wire, va & vb);
        // Garbled table row
        garbled_table[pos] = SGC::HashStandard(SGC::Find_Wire(wire, va),
                                               SGC::Find_Wire(rhs.wire, vb),
                                               SGC::and_gate_id) ^ output_wire;
      }
    }
    */

    return res;
}

Bit<Role::Garbler> Bit<Role::Garbler>::AND(const Bit<Role::Garbler>& rhs,
    std::vector<KappaBitString> &gc_material) const {
  if (isConstant && rhs.isConstant) return Bit<Role::Garbler>(wire & rhs.wire, 1);
  if (isConstant) return wire == 0 ? Bit<Role::Garbler>::constant(0) : rhs;
  if (rhs.isConstant) return rhs.wire == 0 ? Bit<Role::Garbler>::constant(0) : *this;

  Bit<Role::Garbler> res;

  // Half gates Technique
  // v_a & v_b = (v_a ^ r ^ r) & v_b
  // v_a & v_b = [v_a & (r ^ v_b)] ^ [v_a & r]
  // Algorithm: page 9 of half-gates paper

  //const auto nonce0 = std::bitset<128> { SGC::and_gate_id };
  //const auto nonce1 = std::bitset<128> { SGC::and_gate_id + 1 };
  const auto nonce0 = std::bitset<128> { SGC::material_id };
  const auto nonce1 = std::bitset<128> { SGC::material_id + 1 };

  // Generate 2 Garbled Rows

  const auto ha0 = SGC::HashHalfGates(SGC::Find_Wire(wire, false), nonce0);
  const auto ha1 = SGC::HashHalfGates(SGC::Find_Wire(wire, true), nonce0);

  const auto hb0 = SGC::HashHalfGates(SGC::Find_Wire(rhs.wire, false), nonce1);
  const auto hb1 = SGC::HashHalfGates(SGC::Find_Wire(rhs.wire, true), nonce1);

  gc_material.push_back(rhs.wire[0] ? (ha0 ^ ha1 ^ SGC::delta) : (ha0 ^ ha1));
  gc_material.push_back(hb0 ^ hb1 ^ wire);

#ifdef GARDEBUG
    std::cout << SGC::material[SGC::and_gate_id] << std::endl;
    std::cout << SGC::material[SGC::and_gate_id+1] << std::endl;
#endif

  const auto g = wire[0] ? (ha0 ^ gc_material[SGC::temp_buffer_idx]) : ha0;
  const auto e = rhs.wire[0] ? (hb0 ^ gc_material[SGC::temp_buffer_idx + 1] ^ wire) : hb0;

  res.wire = g ^ e; // whether res.wire = f(in1.wire, in2.wire, delta) // RO?

  //SGC::and_gate_id += 2;
  SGC::material_id += 2;
  SGC::temp_buffer_idx += 2;

  return res;
}

// AND-GATE: Evaluator
Bit<Role::Evaluator> Bit<Role::Evaluator>::operator &(const Bit<Role::Evaluator> &rhs) const {
    // std::cout << "Executing AND Evaluator... " << std::endl;

    if (isConstant && rhs.isConstant) return Bit<Role::Evaluator>(wire&rhs.wire, 1);
    if (isConstant) return wire == 0 ? Bit<Role::Evaluator>::constant(0) : rhs;
    if (rhs.isConstant) return rhs.wire == 0 ? Bit<Role::Evaluator>::constant(0) : *this;

    Bit<Role::Evaluator> res;

#ifdef GARDEBUG
    std::bitset<128> garbled_table;    
    std::cin >> garbled_table; SGC::material.push_back(garbled_table);
    std::cin >> garbled_table; SGC::material.push_back(garbled_table);
#endif

    // Half gates Technique
    // v_a & v_b = (v_a ^ r ^ r) & v_b
    // v_a & v_b = [v_a & (r ^ v_b)] ^ [v_a & r]

    // Algorithm: page 9 of half-gates paper
    //const auto nonce0 = std::bitset<128> { SGC::and_gate_id };
    //const auto nonce1 = std::bitset<128> { SGC::and_gate_id + 1 };
    const auto nonce0 = KappaBitString { SGC::material_id };
    const auto nonce1 = KappaBitString { SGC::material_id + 1 };

    const auto ha = SGC::HashHalfGates(wire, nonce0);
    const auto hb = SGC::HashHalfGates(rhs.wire, nonce1);

    //const auto g = wire[0] ? (ha ^ SGC::material[SGC::and_gate_id]) : ha;
    //const auto e = rhs.wire[0] ? (hb ^ SGC::material[SGC::and_gate_id + 1] ^ wire) : hb;
    const auto g = wire[0] ? (ha ^ SGC::material_buffer[SGC::material_id]) : ha;
    const auto e = rhs.wire[0] ? (hb ^ SGC::material_buffer[SGC::material_id + 1] ^ wire) : hb;

    //SGC::and_gate_id += 2;
    SGC::material_id += 2;
    SGC::buffer_idx += 2;

    res.wire = g ^ e;

    // Standard Unoptimized 4 row GC

    // int pos = 2 * wire[0] + rhs.wire[0];
    // std::bitset<128> output_wire = SGC::HashStandard(wire, rhs.wire, SGC::and_gate_id) ^ garble_table[pos];
    // res.wire = output_wire;
    // SGC::and_gate_id++;

    return res;
}

Bit<Role::Evaluator> Bit<Role::Evaluator>::AND(const Bit<Role::Evaluator>& rhs,
                                           std::vector<KappaBitString> &gc_material) const {
  if (isConstant && rhs.isConstant) return Bit<Role::Evaluator>(wire&rhs.wire, 1);
  if (isConstant) return wire == 0 ? Bit<Role::Evaluator>::constant(0) : rhs;
  if (rhs.isConstant) return rhs.wire == 0 ? Bit<Role::Evaluator>::constant(0) : *this;

  Bit<Role::Evaluator> res;

#ifdef GARDEBUG
    std::bitset<128> garbled_table;
    std::cin >> garbled_table; SGC::material.push_back(garbled_table);
    std::cin >> garbled_table; SGC::material.push_back(garbled_table);
#endif

  // Half gates Technique
  // v_a & v_b = (v_a ^ r ^ r) & v_b
  // v_a & v_b = [v_a & (r ^ v_b)] ^ [v_a & r]

  // Algorithm: page 9 of half-gates paper
  //const auto nonce0 = std::bitset<128> { SGC::and_gate_id };
  //const auto nonce1 = std::bitset<128> { SGC::and_gate_id + 1 };
  const auto nonce0 = KappaBitString { SGC::material_id };
  const auto nonce1 = KappaBitString { SGC::material_id + 1 };

  const auto ha = SGC::HashHalfGates(wire, nonce0);
  const auto hb = SGC::HashHalfGates(rhs.wire, nonce1);

  //const auto g = wire[0] ? (ha ^ SGC::material[SGC::and_gate_id]) : ha;
  //const auto e = rhs.wire[0] ? (hb ^ SGC::material[SGC::and_gate_id + 1] ^ wire) : hb;
  const auto g = wire[0] ? (ha ^ gc_material[SGC::temp_buffer_idx]) : ha;
  const auto e = rhs.wire[0] ? (hb ^ gc_material[SGC::temp_buffer_idx + 1] ^ wire) : hb;

  //SGC::and_gate_id += 2;
  SGC::material_id += 2;
  SGC::temp_buffer_idx += 2;

  res.wire = g ^ e;

  // Standard Unoptimized 4 row GC

  // int pos = 2 * wire[0] + rhs.wire[0];
  // std::bitset<128> output_wire = SGC::HashStandard(wire, rhs.wire, SGC::and_gate_id) ^ garble_table[pos];
  // res.wire = output_wire;
  // SGC::and_gate_id++;

  return res;
}

Bit<Role::Cleartext> Bit<Role::Cleartext>::operator ^(const Bit<Role::Cleartext>& rhs) const {
    Bit<Role::Cleartext> res;
    res.val = val ^ rhs.val;
    return res;
}

Bit<Role::Garbler> Bit<Role::Garbler>::operator ^(const Bit<Role::Garbler>& rhs) const {
    Bit<Role::Garbler> res;
    res.wire = wire ^ rhs.wire;
    res.isConstant = isConstant & rhs.isConstant;
    // res.point = point ^ rhs.point;
    return res;
}

Bit<Role::Evaluator> Bit<Role::Evaluator>::operator ^(const Bit<Role::Evaluator>& rhs) const {
    // TODO: make it better?
    if (isConstant && rhs.isConstant) return Bit<Role::Evaluator>(wire ^ rhs.wire, 1);
    if (isConstant) return rhs;
    if (rhs.isConstant) return *this;
    return Bit<Role::Evaluator>(wire ^ rhs.wire, 0);
}

Bit<Role::Cleartext> Bit<Role::Cleartext>::operator ~() const {
  Bit<Role::Cleartext> res;
  res.val = val ^ 1;
  return res;
}

Bit<Role::Garbler> Bit<Role::Garbler>::operator ~() const {
  Bit<Role::Garbler> res;
  res.wire = wire ^ SGC::delta;
  res.isConstant = isConstant;
  return res;
}

Bit<Role::Evaluator> Bit<Role::Evaluator>::operator ~() const {
    if (isConstant) return Bit<Role::Evaluator>::constant(wire[0] ^ 1);
    return *this;
}


// swappable

Bit<Role::Garbler> operator &(const Bit<Role::Garbler> &lhs, const bool &rhs) {
    return rhs ? lhs : Bit<Role::Garbler>::constant(0);
}

Bit<Role::Garbler> operator &(const bool &lhs, const Bit<Role::Garbler> &rhs) {
    return lhs ? rhs : Bit<Role::Garbler>::constant(0);
}

Bit<Role::Garbler> operator ^(const Bit<Role::Garbler> &lhs, const bool &rhs) {
    return rhs ? ~lhs : lhs;
}

Bit<Role::Garbler> operator ^(const bool &lhs, const Bit<Role::Garbler> &rhs) {
    return lhs ? ~rhs : rhs;
}

Bit<Role::Garbler> operator==(const Bit<Role::Garbler> &l, const bool &r) {
  Bit<Role::Garbler> res;
  res.wire = r ? l.wire : (l.wire ^ SGC::delta);
  return res;
}

Bit<Role::Evaluator> operator==(const Bit<Role::Evaluator> &l, const bool &r) {
  return l;
}


}; // namespace SGC



/* // TODO: make this ths static?
template<Role role>
Bit<role> operator &(const Bit<Role::Garbler> &lhs, const Bit<Role::Cleartext> &rhs) {
    if (rhs.val)
        return lhs;
    else
        return Bit<Role::Cleartext>(0);
}

template<Role role>
Bit<role> operator &(const Bit<Role::Cleartext> &lhs, const Bit<Role::Garbler> &rhs) {
    if (lhs.val)
        return rhs;
    else
        return Bit<Role::Cleartext>(0);
}
 */

/* 
Bit<Role::Garbler> operator ^(const Bit<Role::Garbler> &lhs, const Bit<Role::Cleartext> &rhs) {
    return rhs.val ? ~lhs : lhs;
}

Bit<Role::Garbler> operator ^(const Bit<Role::Cleartext> &lhs, const Bit<Role::Garbler> &rhs) {
    return lhs.val ? ~rhs : rhs;
} 
*/
