#include "SGCUtils.h"
#include <map>
#include <iostream>
#include <queue>
#include <cstdio>
#include <cmath>
#include <algorithm>
//#include <pair>

namespace SGC {

    // with new instructions, you may have to change the following four functions

    // Gb for a fragment
    void GbFragment(SGC::PRG &current_prg, std::vector<SGC::Bit32<Role::Garbler>> &current_reg, 
                    const SGC::Fragment &fragment, std::vector<KappaBitString> &gbmaterials, 
                    std::vector<SGC::Bit32<Role::Garbler>> &idx_array,
                    std::vector<SGC::Bit32<Role::Garbler>> &val_array) {
        SGC::Backup();
        current_prg(); current_prg(); // discard two seeds for children
        SGC::delta = current_prg();
        SGC::delta[0] = 1;
        SGC::temp_buffer_idx = 0;
        for (int i = 0; i < SGC::reg_cnt; i++) current_reg.push_back(SGC::Bit32<Role::Garbler>(current_prg));
        SGC::Bit32<Role::Garbler> zero_reg;
        // Iterate over each instruction in the fragment and construct material
        for (Inst inst : fragment) {
            switch (inst.op) {
                case OPCODE::ADD: // ADD
                    current_reg[inst.dst] = current_reg[inst.src1].Add(current_reg[inst.src2], gbmaterials);
                    break;
                case OPCODE::ADDI:
                    current_reg[inst.dst] = current_reg[inst.src1].Add(GbGetImm(inst.imm), gbmaterials);
                    break;
                case OPCODE::SUB: // SUB
                    current_reg[inst.dst] = current_reg[inst.src1].Sub(current_reg[inst.src2], gbmaterials);
                    break;                    
                case OPCODE::SUBI: 
                    current_reg[inst.dst] = current_reg[inst.src1].Sub(GbGetImm(inst.imm), gbmaterials);
                    break;                    
                case OPCODE::MUL: // MUL
                    current_reg[inst.dst] = current_reg[inst.src1].Multiply(current_reg[inst.src2], gbmaterials);
                    break;
                case OPCODE::LOAD: // LOAD
                    idx_array.push_back(current_reg[inst.src1]);
                    val_array.push_back(zero_reg);
                    for (int i = 0; i < BIT32::SIZE; i++) current_reg[inst.dst].bits[i].wire = current_prg();
                    break;
                case OPCODE::STORE: // STORE
                    idx_array.push_back(current_reg[inst.dst]);
                    val_array.push_back(current_reg[inst.src1]);
                    for (int i = 0; i < BIT32::SIZE; i++) current_prg();
                    break;
                case OPCODE::IMM: // IMM
                    current_reg[inst.dst] = GbGetImm(inst.imm);
                    break;
                case OPCODE::SWAP:
                    std::swap(current_reg[inst.src1].bits, current_reg[inst.src2].bits);
                    break;
                case OPCODE::COPY:
                    for (int i = 0; i < BIT32::SIZE; i++)
                        current_reg[inst.dst].bits[i] = current_reg[inst.src1].bits[i];
                    break;
                case OPCODE::CCOPY:
                    {
                        auto le = current_reg[inst.src2].And_Bit(current_reg[inst.src1], gbmaterials, 0, 0);
                        auto ri = current_reg[inst.dst].And_Bit(current_reg[inst.src1], gbmaterials, 0, 1);
                        current_reg[inst.dst] = le ^ ri;
                    }
                    break;
                case OPCODE::CMP:
                    current_reg[inst.dst] = current_reg[inst.src1].Cmp(current_reg[inst.src2], gbmaterials);
                    break;
                case OPCODE::EQ:
                    current_reg[inst.dst] = current_reg[inst.src1].Eq(current_reg[inst.src2], gbmaterials);
                    break;
                case OPCODE::EQI:
                    current_reg[inst.dst] = current_reg[inst.src1].Eq(GbGetImm(inst.imm), gbmaterials);
                    break;
                case OPCODE::XOR:
                    current_reg[inst.dst] = current_reg[inst.src1] ^ current_reg[inst.src2];
                    break;
                case OPCODE::AND0:
                    current_reg[inst.dst] = current_reg[inst.src1].And_Bit(current_reg[inst.src2], gbmaterials, 0, 0);
                    break;
                case OPCODE::AND1:
                    current_reg[inst.dst] = current_reg[inst.src1].And_Bit(current_reg[inst.src2], gbmaterials, 1, 0);
                    break;
                case OPCODE::ANDN0:
                    current_reg[inst.dst] = current_reg[inst.src1].And_Bit(current_reg[inst.src2], gbmaterials, 0, 1);
                    break;
                case OPCODE::ANDN1:
                    current_reg[inst.dst] = current_reg[inst.src1].And_Bit(current_reg[inst.src2], gbmaterials, 1, 1);
                    break;
                case OPCODE::RS1:
                    for (int i = 0; i < BIT32::SIZE - 1; i++) current_reg[inst.dst].bits[i].wire = current_reg[inst.dst].bits[i+1].wire;
                    current_reg[inst.dst].bits[BIT32::SIZE - 1].wire = 0;
                    break;
                case OPCODE::NOOP:
                    break;
                default:
                    std::cerr << "Invalid OPCODE 2" << std::endl;
                    exit(255);
            }
        }
        SGC::RestoreBackup();
    }

    // ev singleinst!!!
    void single_inst(Inst &inst, std::vector<SGC::Bit32<Role::Evaluator>> &reg, std::vector<KappaBitString> &evmaterials) {
        switch (inst.op) {
            case OPCODE::ADD: // ADD
                reg[inst.dst] = reg[inst.src1].Add(reg[inst.src2], evmaterials);
                break;
            case OPCODE::ADDI: // ADD
                reg[inst.dst] = reg[inst.src1].Add(SGC::Bit32<Role::Evaluator>(), evmaterials);
                break;
            case OPCODE::SUB: // SUB
                reg[inst.dst] = reg[inst.src1].Sub(reg[inst.src2], evmaterials);
                break;                
            case OPCODE::SUBI: // SUB
                reg[inst.dst] = reg[inst.src1].Sub(SGC::Bit32<Role::Evaluator>(), evmaterials);
                break;                
            case OPCODE::MUL: // MUL
                reg[inst.dst] = reg[inst.src1].Multiply(reg[inst.src2], evmaterials);
                break; 
            case OPCODE::IMM: // IMM
                for (int i = 0; i < BIT32::SIZE; i++)
                    reg[inst.dst].bits[i].wire = 0;
                break;
            case OPCODE::SWAP:
                std::swap(reg[inst.src1].bits, reg[inst.src2].bits);
                break;
            case OPCODE::COPY:
                for (int i = 0; i < BIT32::SIZE; i++)
                    reg[inst.dst].bits[i] = reg[inst.src1].bits[i];
                break;
            case OPCODE::CCOPY:
                {
                    auto le = reg[inst.src2].And_Bit(reg[inst.src1], evmaterials, 0, 0);
                    auto ri = reg[inst.dst].And_Bit(reg[inst.src1], evmaterials, 0, 1);
                    reg[inst.dst] = le ^ ri;
                }
                break;
            case OPCODE::LOAD:
                std::cerr << "LOAD" << std::endl;
                break;
            case OPCODE::STORE:
                std::cerr << "STORE" << std::endl;
                break;
            case OPCODE::CMP:
                reg[inst.dst] = reg[inst.src1].Cmp(reg[inst.src2], evmaterials);
                break;
            case OPCODE::EQ:
                reg[inst.dst] = reg[inst.src1].Eq(reg[inst.src2], evmaterials);
                break;
            case OPCODE::EQI:
                reg[inst.dst] = reg[inst.src1].Eq(SGC::Bit32<Role::Evaluator>(), evmaterials);
                break;
            case OPCODE::XOR:
                reg[inst.dst] = reg[inst.src1] ^ reg[inst.src2];
                break;
            case OPCODE::AND0:
                reg[inst.dst] = reg[inst.src1].And_Bit(reg[inst.src2], evmaterials, 0, 0);
                break;
            case OPCODE::AND1:
                reg[inst.dst] = reg[inst.src1].And_Bit(reg[inst.src2], evmaterials, 1, 0);
                break;
            case OPCODE::ANDN0:
                reg[inst.dst] = reg[inst.src1].And_Bit(reg[inst.src2], evmaterials, 0, 1);
                break;
            case OPCODE::ANDN1:
                reg[inst.dst] = reg[inst.src1].And_Bit(reg[inst.src2], evmaterials, 1, 1);
                break;
            case OPCODE::RS1:            
                for (int i = 0; i < BIT32::SIZE - 1; i++) reg[inst.dst].bits[i].wire = reg[inst.dst].bits[i+1].wire;
                reg[inst.dst].bits[BIT32::SIZE - 1].wire = 0;
                break;
            case OPCODE::NOOP:
                break;
            default:
                std::cerr << "Invalid OPCODE 1: " << inst.op << std::endl;
                exit(255);
        }
    }


    void GbExecuteSingleBBWithoutStacking(std::vector<SGC::Bit32<Role::Garbler>> &reg, EpiGRAM<Mode::G> &mem, const SGC::Fragment &fragment) {
        for (Inst inst : fragment) {
            switch (inst.op) {
                case OPCODE::ADD: // ADD
                    reg[inst.dst] = reg[inst.src1] + reg[inst.src2];
                    break;
                case OPCODE::ADDI: // ADD
                    reg[inst.dst] = reg[inst.src1] + GbGetImm(inst.imm);
                    break;
                case OPCODE::SUB: // SUB
                    reg[inst.dst] = reg[inst.src1].Sub(reg[inst.src2]);                
                    break;                    
                case OPCODE::SUBI: // SUB
                    reg[inst.dst] = reg[inst.src1].Sub(GbGetImm(inst.imm));                
                    break;                    
                case OPCODE::MUL: // MUL
                    reg[inst.dst] = reg[inst.src2] * reg[inst.src2];
                    break;
                case OPCODE::LOAD: // LOAD
                    {
                        SGC::Bit<Role::Garbler> rw_bit;
                        SGC::Bit32<Role::Garbler> zero_val;
                        reg[inst.dst] = GbACC(mem, SGC::mem_w, SGC::mem_n, reg[inst.src1], zero_val, rw_bit);
                    }
                    break;
                case OPCODE::STORE: // STORE
                    std::cerr << "no stacking store!" << std::endl;
                    {
                        SGC::Bit<Role::Garbler> rw_bit;
                        rw_bit.wire = SGC::delta;
                        GbACC(mem, SGC::mem_w, SGC::mem_n, reg[inst.dst], reg[inst.src1], rw_bit);
                    }
                    break;
                case OPCODE::IMM: // IMM
                    for (int i = 0; i < BIT32::SIZE; i++)
                        if ((inst.imm >> i) & 1)
                            reg[inst.dst].bits[i].wire = SGC::delta;
                        else
                            reg[inst.dst].bits[i].wire = 0;
                    break;
                case OPCODE::SWAP:
                    std::swap(reg[inst.src1].bits, reg[inst.src2].bits);
                    break;
                case OPCODE::COPY:
                    for (int i = 0; i < BIT32::SIZE; i++)
                        reg[inst.dst].bits[i] = reg[inst.src1].bits[i];
                    break;
                case OPCODE::CCOPY:
                    {
                        auto le = reg[inst.src2].And_Bit(reg[inst.src1], 0, 0);
                        auto ri = reg[inst.dst].And_Bit(reg[inst.src1], 0, 1);
                        reg[inst.dst] = le ^ ri;
                    }
                    break;                    
                case OPCODE::CMP:
                    reg[inst.dst] = reg[inst.src1].Cmp(reg[inst.src2]);
                    break;
                case OPCODE::EQ:
                    reg[inst.dst] = reg[inst.src1].Eq(reg[inst.src2]);
                    break;
                case OPCODE::EQI:
                    reg[inst.dst] = reg[inst.src1].Eq(GbGetImm(inst.imm));
                    break;
                case OPCODE::XOR:
                    reg[inst.dst] = reg[inst.src1] ^ reg[inst.src2];
                    break;
                case OPCODE::AND0:
                    reg[inst.dst] = reg[inst.src1].And_Bit(reg[inst.src2], 0, 0);
                    break;
                case OPCODE::AND1:
                    reg[inst.dst] = reg[inst.src1].And_Bit(reg[inst.src2], 1, 0);
                    break;
                case OPCODE::ANDN0:
                    reg[inst.dst] = reg[inst.src1].And_Bit(reg[inst.src2], 0, 1);
                    break;
                case OPCODE::ANDN1:
                    reg[inst.dst] = reg[inst.src1].And_Bit(reg[inst.src2], 1, 1);
                    break;
                case OPCODE::RS1:
                    for (int i = 0; i < BIT32::SIZE - 1; i++) reg[inst.dst].bits[i].wire = reg[inst.dst].bits[i+1].wire;
                    reg[inst.dst].bits[BIT32::SIZE - 1].wire = 0;
                    break;
                case OPCODE::NOOP:
                    break;
                default:
                    std::cerr << "Invalid OPCODE 2" << std::endl;
                    exit(255);
            }
        }        
    }


    void EvExecuteSingleBBWithoutStacking(std::vector<SGC::Bit32<Role::Evaluator>> &reg, EpiGRAM<Mode::E> &mem, const SGC::Fragment &fragment) {
        for (Inst inst : fragment) {
            switch (inst.op) {
                case OPCODE::ADD: // ADD
                    reg[inst.dst] = reg[inst.src1] + reg[inst.src2];
                    break;
                case OPCODE::ADDI: // ADD
                    reg[inst.dst] = reg[inst.src1] + SGC::Bit32<Role::Evaluator>();
                    break;
                case OPCODE::SUB: // SUB
                    reg[inst.dst] = reg[inst.src1].Sub(reg[inst.src2]);                
                    break;                    
                case OPCODE::SUBI: // SUB
                    reg[inst.dst] = reg[inst.src1].Sub(SGC::Bit32<Role::Evaluator>());                
                    break;                    
                case OPCODE::MUL: // MUL
                    reg[inst.dst] = reg[inst.src2] * reg[inst.src2];
                    break;
                case OPCODE::LOAD: // LOAD
                    {
                        SGC::Bit<Role::Evaluator> rw_bit;
                        SGC::Bit32<Role::Evaluator> zero_val;
                        reg[inst.dst] = EvACC(mem, SGC::mem_w, SGC::mem_n, reg[inst.src1], zero_val, rw_bit);
                    }
                    break;
                case OPCODE::STORE: // STORE
                    std::cerr << "no stacking store!" << std::endl;
                    {
                        SGC::Bit<Role::Evaluator> rw_bit;
                        rw_bit.wire = SGC::delta;
                        EvACC(mem, SGC::mem_w, SGC::mem_n, reg[inst.dst], reg[inst.src1], rw_bit);
                    }
                    break;
                case OPCODE::IMM: // IMM
                    for (int i = 0; i < BIT32::SIZE; i++)
                        if ((inst.imm >> i) & 1)
                            reg[inst.dst].bits[i].wire = SGC::delta;
                        else
                            reg[inst.dst].bits[i].wire = 0;
                    break;
                case OPCODE::SWAP:
                    std::swap(reg[inst.src1].bits, reg[inst.src2].bits);
                    break;
                case OPCODE::COPY:
                    for (int i = 0; i < BIT32::SIZE; i++)
                        reg[inst.dst].bits[i] = reg[inst.src1].bits[i];
                    break;
                case OPCODE::CCOPY:
                    {
                        auto le = reg[inst.src2].And_Bit(reg[inst.src1], 0, 0);
                        auto ri = reg[inst.dst].And_Bit(reg[inst.src1], 0, 1);
                        reg[inst.dst] = le ^ ri;
                    }
                    break;                    
                case OPCODE::CMP:
                    reg[inst.dst] = reg[inst.src1].Cmp(reg[inst.src2]);
                    break;
                case OPCODE::EQ:
                    reg[inst.dst] = reg[inst.src1].Eq(reg[inst.src2]);
                    break;
                case OPCODE::EQI:
                    reg[inst.dst] = reg[inst.src1].Eq(SGC::Bit32<Role::Evaluator>());
                    break;
                case OPCODE::XOR:
                    reg[inst.dst] = reg[inst.src1] ^ reg[inst.src2];
                    break;
                case OPCODE::AND0:
                    reg[inst.dst] = reg[inst.src1].And_Bit(reg[inst.src2], 0, 0);
                    break;
                case OPCODE::AND1:
                    reg[inst.dst] = reg[inst.src1].And_Bit(reg[inst.src2], 1, 0);
                    break;
                case OPCODE::ANDN0:
                    reg[inst.dst] = reg[inst.src1].And_Bit(reg[inst.src2], 0, 1);
                    break;
                case OPCODE::ANDN1:
                    reg[inst.dst] = reg[inst.src1].And_Bit(reg[inst.src2], 1, 1);
                    break;
                case OPCODE::RS1:
                    for (int i = 0; i < BIT32::SIZE - 1; i++) reg[inst.dst].bits[i].wire = reg[inst.dst].bits[i+1].wire;
                    reg[inst.dst].bits[BIT32::SIZE - 1].wire = 0;
                    break;
                case OPCODE::NOOP:
                    break;
                default:
                    std::cerr << "Invalid OPCODE 2" << std::endl;
                    exit(255);
            }
        }        
    }

    /*
    // Generator
    void GenGCMaterial(Fragment &fragment, KappaBitString &seed, uint32_t &nop, GCMaterial &res, uint32_t &SGT_length, size_t branch_idx) {
        //SGC::and_gate_id = 0;
        //SGC::material.clear();
        SGC::temp_buffer_idx = 0;
        SGC::PRG tmpprg(seed);
        res.hash_key = tmpprg();
        SGC::prf = PRF(res.hash_key);
        SGC::delta = res.gc_delta = tmpprg();
        SGC::delta[0] = res.gc_delta[0] = 1;
        int start_material_id = SGC::material_id;
        std::cerr << "start" << start_material_id << std::endl;
        assert(buffer_idx == SGC::material_buffer.size());
        // update material id based on the branch id
        int offset_material_id = start_material_id + branch_idx * SGT_length; // sgt length is 0 during first iteration
        SGC::material_id = offset_material_id;

        SGC::Bit32<Role::Garbler> reg[var_cnt];
        for (uint32_t i = 0; i < SGC::reg_cnt; i++) {
            reg[i] = SGC::Bit32<Role::Garbler>(tmpprg);
            reg[i].copy_wires(res.input);
        }

        // Iterate over each instruction in the fragment and construct material
        for (Inst inst : fragment) {
            switch (inst.op) {
                case 0: // ADD
                    //reg[inst.dst] = reg[inst.src1] + reg[inst.src2];
                    reg[inst.dst] = reg[inst.src1].Add(reg[inst.src2], res.gc_material);
                    break;
                case 1: // MUL
                    //reg[inst.dst] = reg[inst.src1] * reg[inst.src2];
                    reg[inst.dst] = reg[inst.src1].Multiply(reg[inst.src2], res.gc_material);
                    break;
                default:
                    std::cerr << "Invalid OPCODE" << std::endl;
                    exit(255);
            }
        }

        // TODO: is this needed?
        // TODO: if needed, which random source?
        for (int i = 0; i < nop; i++) {
            // i think below is not right - leaves spaces (eg 4,5, and next iter 8,9)
            //output[offset + (i << 1)] = tmpprg();
            //output[offset + ((i << 1) ^ 1)] = tmpprg();
            res.gc_material.push_back(tmpprg());
            res.gc_material.push_back(tmpprg());
        }

        for (uint32_t i = 0; i < SGC::reg_cnt; i++) {
            reg[i].copy_wires(res.output);
        }

        SGC::material_id = start_material_id;
    }

    // Evaluator
    void GenGCMaterial(Fragment &fragment, KappaBitString &seed, uint32_t &nop, std::vector<KappaBitString> &gc_material, uint32_t &SGT_length, size_t branch_idx) {
        //SGC::and_gate_id = 0;
        //SGC::material.clear();
        SGC::temp_buffer_idx = 0;
        SGC::PRG tmpprg(seed);
        KappaBitString tmptmp = tmpprg();
        SGC::prf = PRF(tmptmp);
        SGC::delta = tmpprg();
        SGC::delta[0] = 1;
        int start_material_id = SGC::material_id;
        // update material id based on the branch id
        int offset_material_id = start_material_id + branch_idx * SGT_length; // sgt length is 0 during first iteration
        SGC::material_id = offset_material_id;

        assert(SGC::buffer_idx < SGC::material_buffer.size());

        SGC::Bit32<Role::Garbler> reg[var_cnt];
        for (uint32_t i = 0; i < SGC::reg_cnt; i++) {
            reg[i] = SGC::Bit32<Role::Garbler>(tmpprg);
        }

        // Iterate over each instruction in the fragment and construct material
        for (Inst inst : fragment) {
            switch (inst.op) {
                case 0: // ADD
                  reg[inst.dst] = reg[inst.src1].Add(reg[inst.src2], gc_material);
                   // reg[inst.dst] = reg[inst.src1] + reg[inst.src2];
                    break;
                case 1: // MUL
                  reg[inst.dst] = reg[inst.src1].Multiply(reg[inst.src2], gc_material);
                   // reg[inst.dst] = reg[inst.src1] * reg[inst.src2];
                    break;
                default:
                    std::cerr << "Invalid OPCODE" << std::endl;
                    exit(255);
            }
        }

        // TODO: optimize?
        std::cerr << "material id " << SGC::material_id << std::endl;
        std::cerr << "start material id " << start_material_id << std::endl;


        // TODO: is this needed?
        // TODO: if needed, which random source?
        for (int i = 0; i < nop; i++) {
            // i think below is not right - leaves spaces (eg 4,5, and next iter 8,9)
            //output[offset + (i << 1)] = tmpprg();
            //output[offset + ((i << 1) ^ 1)] = tmpprg();
            gc_material.push_back(tmpprg());
            gc_material.push_back(tmpprg());
        }

        SGC::material_id = start_material_id;
    }

    /*
    void GenSGCMaterial(Fragment &fragment, KappaBitString &seed,
        std::vector<KappaBitString> &input, std::vector<KappaBitString> &output, std::vector<KappaBitString> &material, KappaBitString & uint32_t nop) {
        //std::map<std::string, SGC::Bit32<Garbler>> reg;
        // Set the SGC state
        // Don't forget to initialize and_id to 0 !!!!!!!!! (correct place?)
        SGC::and_gate_id = 0;
        SGC::material.clear();
        SGC::PRG tmpprg(seed);
        SGC::prf = SGC::prf(tmpprg());
        SGC::delta = tmpprg();
        SGC::delta[0] = 1;

        // Initialize the registry and save the input
        SGC::Bit32<Role::Garbler> reg[var_cnt];        
        for (uint32_t i = 0; i < SGC::reg_cnt; i++) { 
            reg[i] = SGC::Bit32<Role::Garbler>(tmpprg);
            reg[i].copy_wires(input);
        }

        // Iterate over each instruction in the fragment and construct material
        for (Inst inst : fragment) {
            switch (inst.op) {
                case 0: // ADD
                    reg[inst.dst] = reg[inst.src1] + reg[inst.src2];
                    break;
                case 1: // MUL
                    reg[inst.dst] = reg[inst.src1] * reg[inst.src2];
                    break;
                default:
                    std::cerr << "Invalid OPCODE" << std::endl;
                    exit(255);                    
            }
        }

        // Construct the output (padded material) from SGC::material and padding
        // TODO: do it efficiently
        int offset = SGC::material.size();        
        output.reserve(offset + nop * 2);
        for (int i = 0; i < offset; i++) output[i] = SGC::material[i];
        // TODO: is this needed?
        // TODO: if needed, which random source?
        for (int i = 0; i < nop; i++) {
            // i think below is not right - leaves spaces (eg 4,5, and next iter 8,9)
            //output[offset + (i << 1)] = tmpprg();
            //output[offset + ((i << 1) ^ 1)] = tmpprg();
            output[offset + 2 * i] = tmpprg();
            output[offset + 2 * i  + 1] = tmpprg();
        }
    }
    */

    /*
    void GetGenTable(std::vector<KappaBitString> &true_seed, 
                     std::vector<KappaBitString> &fake_seed, 
                     std::vector<Fragment> &fragments,
                     std::vector<std::vector<KappaBitString>> &true_materials,
                     std::vector<std::vector<std::vector<KappaBitString>>> &fake_materials) {
        // expanding true_seed into a vector (the lowest layer) of true seeds
        // call GenSGCMaterial accordingly
        uint32_t branch_cnt = fragments.size();
        true_materials.reserve(branch_cnt);
        // TODO: maybe an argument
        std::vector< std::vector<KappaBitString> > inputs;
        inputs.reserve(branch_cnt);
        uint32_t zero = 0;
        // TODO: figure out padding stuffs
        for (uint32_t i = 0; i < branch_cnt; i++) GenSGCMaterial(fragments[i], true_seed[i], inputs[i], true_materials[i], zero);

        fake_materials.reserve(branch_cnt);

        uint32_t tree_size = fake_seed.size(); // root is not useful
        uint32_t k = 1;
        // TODO: translate this into math formulation
        while ( (1 << (k + 1)) - 1 < tree_size ) k++;
        k = (1 << (k + 1)) - 2;
        for (uint32_t i = 1; i < tree_size; i++) {
            std::queue< std::pair<uint32_t, KappaBitString> > q;
            q.push( std::make_pair(i, fake_seed[i]) );
            while (!q.empty())
            {
                std::pair<uint32_t, KappaBitString> now = q.front();
                q.pop();
                if (now.first * 2 + 1 >= tree_size) { // this is a leaf
                    uint32_t idx = now.first > k ? (now.first - k - 1) : (branch_cnt - 1 - k + now.first);
                    std::vector<KappaBitString> tmpinput; // TODO: this is a return value too
                    std::vector<KappaBitString> tmpmaterial; // TODO: optimize this to get rid of copying complexity
                    GenSGCMaterial(fragments[idx], now.second, tmpinput, tmpmaterial, zero);
                    fake_materials[idx].push_back(tmpmaterial);
                } else {
                    SGC::PRG _prg(now.second);
                    q.push( std::make_pair(i*2+1, _prg()) );
                    q.push( std::make_pair(i*2+2, _prg()) );
                }
            }
        }
    }
    */

    /*
   // garbled OT
    void GOT(SGC::Bit<Role::Garbler> &b, KappaBitString &K0, KappaBitString &K1) {
        KappaBitString r0 = b.wire[0] ? (HashSGC(b.wire ^ SGC::delta) ^ K1) : (HashSGC(b.wire) ^ K0);
        KappaBitString r1 = b.wire[0] ? (HashSGC(b.wire) ^ K0) : (HashSGC(b.wire ^ SGC::delta) ^ K1);
        SGC::material_buffer.push_back(r0);
        SGC::material_buffer.push_back(r1);
    }
    KappaBitString GOT(SGC::Bit<Role::Evaluator> &b) {
        KappaBitString r0 = SGC::get_next_buffer();
        KappaBitString r1 = SGC::get_next_buffer();
        return HashSGC(b.wire) ^ (b.wire[0] ? r1 : r0);
    }
    */

    /*

    // Generator and Evaluator
    void EvalGC(std::vector<KappaBitString> &material_, Fragment &fragment,
                std::vector<KappaBitString> &init_wire, std::vector<KappaBitString> &output_wire, KappaBitString &hash_key,
                int material_id_before_gen, size_t SGT_length, int bid) {

        //SGC::and_gate_id = 0;
        //SGC::material.clear();
        SGC::prf = PRF(hash_key);

        // Initialize regs with correct garbling labels from input
        SGC::Bit32<Role::Evaluator> reg[var_cnt];
        // TODO: think about a better way
        for (uint32_t it = 0, i = 0; i < reg_cnt; i++) {
            for (uint32_t j = 0; j < BIT32::SIZE; j++, it++) {
                reg[i].bits[j].wire = init_wire[it];
            }
        }
        // TODO: avoid copying material again and again? Pointer to material?
        // Set SGC::material from input material_
        //SGC::and_gate_id = 0;
        SGC::temp_buffer_idx = 0;
        int start_material_id = SGC::material_id;
        SGC::material_id = material_id_before_gen + SGT_length * bid;
        //SGC::material_id = offset;
        //SGC::material.reserve(offset);
        //for (int i = 0; i < offset; i++) SGC::material.push_back(material_[i]);

        //for (int i = 0; i < offset; i++) SGC::material_buffer.push_back(material_[i]);
        // Evaluate the instructions in the fragment
        for (Inst inst : fragment) {
            switch (inst.op) {
                case 0: // ADD
                    reg[inst.dst] = reg[inst.src1].Add(reg[inst.src2], material_);
                    break;
                case 1: // MUL
                    reg[inst.dst] = reg[inst.src1].Multiply(reg[inst.src2], material_);
                    break;
                default:
                    std::cerr << "Invalid OPCODE" << std::endl;
                    exit(255);
            }
        }
        // TODO do something better
        //for (int i = 0; i < offset; i++) SGC::material_buffer.pop_back();
        SGC::material_id = start_material_id;
        // Move updated registry to output
        for (uint32_t i = 0; i < SGC::reg_cnt; i++) reg[i].copy_wires(output_wire);
    }

    void Share_Known_G(Bit<Role::Garbler> &cond, KappaBitString &val, KappaBitString &share) {
        // send val ^ H(cond.wire) ^ H(cond.wire ^ Delta)
        // SGC::material_buffer.push_back(2);
        SGC::material_buffer.push_back(val ^ HashSGC(cond.wire) ^ HashSGC(cond.wire ^ SGC::delta));
        SGC::material_id++;
        SGC::buffer_idx++;
        // std::cout << "[DEBUG1] " << cond.wire << std::endl;
        if (cond.wire[0]) share = HashSGC(cond.wire ^ SGC::delta) ^ val;
        else share = HashSGC(cond.wire);
        // if cond.wire[0] == 0, set share = H(cond.wire)
        // if cond.wire[0] == 1, set share = H(cond.wire ^ delta) ^ val
    }

    void Share_Known_G(Bit<Role::Evaluator> &cond, KappaBitString &share) {
        // get material
        KappaBitString res = SGC::get_next_buffer();
        // std::cout << "[DEBUG1] " << cond.wire << std::endl;
        if (cond.wire[0]) share = res ^ HashSGC(cond.wire);
        else share = HashSGC(cond.wire);
    }

    void Share_Seed(Bit<Role::Garbler> &cond, KappaBitString &s0, KappaBitString &s1) {
        // send s0 xor s1 xor share
        KappaBitString share;
        Share_Known_G(cond, s1, share);
        // std::cout << "[DEBUG] " << share << std::endl;
        SGC::material_buffer.push_back(s0 ^ s1 ^ share);
        SGC::material_id++;
        SGC::buffer_idx++;
    }

    void Share_Seed(Bit<Role::Evaluator> &cond, KappaBitString &res) {
        KappaBitString share;
        Share_Known_G(cond, share);
        // std::cout << "[DEBUG] " << share << std::endl;
        // get material
        KappaBitString G_share = SGC::get_next_buffer();
        res = G_share ^ share;
    }

    void Input_Translator(Bit<Role::Garbler> &mask, Bit<Role::Garbler> &input, KappaBitString &output0, KappaBitString output1, KappaBitString &junk) {
        KappaBitString truth_table[4];
        uint32_t zero = 0; // TODO: id? What is correct Hash function?

        // if mask = 0, input = 0 return junk
        truth_table[ (mask.wire[0] << 1) + input.wire[0] ] = HashStandard(mask.wire, input.wire, zero) ^ junk;
        // if mask = 0, input = 1 return junk
        truth_table[ (mask.wire[0] << 1) + (input.wire[0] ^ 1) ] = HashStandard(mask.wire, input.wire ^ SGC::delta, zero) ^ junk; 
        // if mask = 1, input = 0 return output0
        truth_table[ ((mask.wire[0] ^ 1) << 1) + input.wire[0] ] = HashStandard(mask.wire ^ SGC::delta, input.wire, zero) ^ output0;
        // if mask = 1, input = 1 return output1
        truth_table[ ((mask.wire[0] ^ 1) << 1) + (input.wire[0] ^ 1) ] = HashStandard(mask.wire ^ SGC::delta, input.wire ^ SGC::delta, zero) ^ output1; 

        for (int i = 0; i < 4; i++) SGC::material_buffer.push_back(truth_table[i]);
        SGC::material_id += 4;
        SGC::buffer_idx += 4;
    }

    KappaBitString Input_Translator(Bit<Role::Evaluator> &mask, Bit<Role::Evaluator> &input) {
        uint32_t zero = 0;

        KappaBitString truth_table[4];
        for (int i = 0; i < 4; i++) truth_table[i] = SGC::get_next_buffer();

        return truth_table[ (mask.wire[0] << 1) + input.wire[0] ] ^ HashStandard(mask.wire, input.wire, zero);
    }

    inline void From_Deltai_To_Delta(KappaBitString &garble_label, KappaBitString &mask) {
        if (garble_label[0]) garble_label ^= mask;
    }

    */

    /*

    void Demux_G(std::vector<Fragment> &fragments, std::vector<Bit32<Role::Garbler>> &reg) {
      std::cerr << "material id at the start of demux " << SGC::material_id << std::endl;
        // Backup();
        KappaBitString root_seed = prg();
        uint32_t branch_cnt = fragments.size();

        // optimize it?
        uint32_t tree_depth = 0;
        while ( (1 << (tree_depth + 1)) < branch_cnt ) tree_depth++;
        // Number of nodes if tree was full
        uint32_t fully_tree_cnt = (1 << (tree_depth + 1)) - 1;
        // Number of nodes in the actual not full tree
        uint32_t tree_cnt = fully_tree_cnt + 2 * ( branch_cnt - (1 << tree_depth) );

        // Compute true seed (at the leaves) and true_tree
        // (seeds at each node of the tree, need to be expanded properly from root seed)
        std::vector<KappaBitString> true_seed(branch_cnt);
        std::vector<KappaBitString> true_tree(tree_cnt);
        std::vector<uint32_t> bid_to_tid(branch_cnt); // mapping from branch id to tree id
        true_tree[0] = root_seed;
        for(int i = 0; i < tree_cnt; i++) {
            if (i * 2 + 1 >= tree_cnt) { // this is a leaf (of the incomplete tree)
                 // TODO if seems unnecessary, i should be at most fully_tree_cnt-1
                if (i >= fully_tree_cnt) {
                    true_seed[i - fully_tree_cnt] = true_tree[i];
                    bid_to_tid[i - fully_tree_cnt] = i;
                }
                // TODO this seems not right, can be negative
                //else true_seed[ branch_cnt - (fully_tree_cnt - i) ] = true_tree[i];
                // TODO did you mean:
                else { 
                    true_seed[ branch_cnt - (fully_tree_cnt - i) ] = true_tree[i];
                    bid_to_tid[ branch_cnt - (fully_tree_cnt - i) ] = i;
                }
            } else {
                SGC::PRG _prg(true_tree[i]);                
                true_tree[i * 2 + 1] = _prg();
                true_tree[i * 2 + 2] = _prg();
            }
        }

        // Create fake seeds (independently randomly generated)
        std::vector<KappaBitString> fake_tree(tree_cnt);
        for (int i = 0; i < tree_cnt; i++) fake_tree[i] = prg();

        // Create fake hash_key
        // TODO: we need this?
        std::vector<KappaBitString> fake_hash_key(branch_cnt);
        for (int i = 0; i < branch_cnt; i++) fake_hash_key[i] = prg();

        // Create fake delta
        std::vector<KappaBitString> fake_delta(branch_cnt);
        for (int i = 0; i < branch_cnt; i++) fake_delta[i] = prg();

        // one hot encoding of condition bits
        std::vector<Bit<Role::Garbler>> mask(branch_cnt);
        for (uint32_t i = 0; i < branch_cnt; i++) mask[i] = (reg[0] == i);

        //for (int i = 0; i < branch_cnt; i++) mask[i].Reveal();

        // for (int i = 0; i < branch_cnt; i++) std::cout << i << " : " << bid_to_tid[i] << std::endl;

        std::vector<Bit<Role::Garbler>> mask_path(tree_cnt);
        std::vector<Bit<Role::Garbler>> mask_tree(tree_cnt);
        // TODO: may be optimized?
        std::vector<std::pair<int, int>> range (tree_cnt);
        for (uint32_t i = 0; i < branch_cnt; i++) {
            mask_path[ bid_to_tid[i] ] = mask[i];
            range[ bid_to_tid[i] ] = std::make_pair(i, i);
        }
        // for (uint32_t i = 0; i < branch_cnt; i++) mask_tree[ bid_to_tid[i] ] = mask[i];
        // TODO: check the bound
        int last_node_with_child = (tree_cnt - 2) >> 1;
        for (int i = last_node_with_child; i >= 0; i--) {
            mask_path[i] = mask_path[i*2 + 1] ^ mask_path[i*2 + 2];
            range[i] = std::make_pair( range[i*2+1].first, range[i*2+2].second );
        }
        for (int i = tree_cnt - 1; i > 0; i--) mask_tree[i] = mask_path[i] ^ mask_path[(i-1) >> 1];

        //for (int i = 1; i < tree_cnt; i++) mask_tree[i].Reveal();

        // SGC::move_material_to_buffer();

        // Start to generate GC material
        Backup();
        std::vector<GCMaterial> true_gcm(branch_cnt);
        // TODO: don't forget the padding of NOP        
        uint32_t zero = 0;
        int material_id_before_gen = SGC::material_id;
        // TODO Set it to max of the true_gcms
        uint32_t SGT_length = 0;// = true_gcm[0].gc_material.size();
        for (int i = 0; i < branch_cnt; i++) {
          GenGCMaterial(fragments[i], true_seed[i], zero, true_gcm[i], SGT_length, i);
          SGT_length = (true_gcm[i].gc_material.size() > SGT_length) ? true_gcm[i].gc_material.size() : SGT_length;
        }

      //std::cerr << true_gcm[0].gc_material.size() << std::endl;
      //std::cerr << true_gcm[1].gc_material.size() << std::endl;
      //std::cerr << true_gcm[2].gc_material.size() << std::endl;
      //std::cerr << true_gcm[3].gc_material.size() << std::endl;
        //for (size_t idx = 1; idx < branch_cnt; idx++) {
        //  uint32_t curr_length = true_gcm[idx].gc_material.size();
        //  uint32_t SGT_length = (curr_length > SGT_length) ? curr_length : SGT_length;
        //}

        // TODO: rearrange
        // expand material stores the material at node i as if all seeds were expanded from this seed
        std::vector<std::vector<KappaBitString>> expand_material(tree_cnt);
        for (int i = 1; i < tree_cnt; i++) {
            int len = range[i].second - range[i].first + 1; // number of nodes below this tree node
            std::queue<KappaBitString> q;
            q.push(true_tree[i] ^ fake_tree[i]);
            // compute seeds of children from the seed at this node
            for (int j = 1; j < len; j++) {
                SGC::PRG _prg(q.front());
                q.pop();
                // Expand children seed from its parent
                q.push(_prg());
                q.push(_prg());
            }
            uint32_t zero = 0;
            // expand material for the last level (last children of some node i)
            GenGCMaterial(fragments[range[i].first], q.front(), zero, expand_material[i], SGT_length, range[i].first); q.pop();
            std::vector<KappaBitString> gc_material;
            for (int j = 1; j < len; j++) {
                gc_material.clear();
                GenGCMaterial(fragments[range[i].first + j], q.front(), zero, gc_material, SGT_length, range[i].first + j); q.pop();
                for (int k = 0 ; k < SGT_length; k++) {
                  expand_material[i][k] ^= gc_material[k];
                  //std::cerr << k << " " << expand_material[i].size() << " " << gc_material.size() << std::endl;
                }
            }
        }
        RestoreBackup();

        // increase by the ids reserved for the branches
        SGC::material_id += branch_cnt * SGT_length;
        // buffer id does not change as everything popped

        // share seed, hash_key and delta (one-hot)
        for (int i = 1; i < tree_cnt; i++) Share_Seed(mask_tree[i], true_tree[i], fake_tree[i]);
        for (int i = 0; i < branch_cnt; i++) Share_Seed(mask[i], true_gcm[i].hash_key, fake_hash_key[i]);
        for (int i = 0; i < branch_cnt; i++) {
            KappaBitString delta_xor = true_gcm[i].gc_delta ^ SGC::delta;
            Share_Seed(mask[i], delta_xor, fake_delta[i]);
            fake_delta[i] ^= delta_xor; // construct true fake
        }
        for (int i = 1; i < tree_cnt; i++) fake_tree[i] ^= true_tree[i];
        for (int i = 0; i < branch_cnt; i++) fake_hash_key[i] ^= true_gcm[i].hash_key;        



        // translate reg[0..3] -> reg[0..3] in language(reg[0])
        // reg[0]' = xor (mask inner_product each reg[0])
        // newlanguage reg[1][0] 0 = xor (mask inner_product each reg[1][0] 0)
        // newlanguage reg[1][0] 1 = xor (mask inner_product each reg[1][0] 1)
        // if reg[1][0] == 0 ? newlanguage reg[1][0] 0 : newlanguage reg[1][0] 1;

        //Bit<Role::Garbler> xxx;
        //Bit32<Role::Garbler> tmp = reg[0] & xxx;

        // reg[0] == 0


        // junk input from prg
        std::vector<std::vector<KappaBitString>> junk_input(branch_cnt, std::vector<KappaBitString>(reg_cnt * BIT32::SIZE));
        for (int b = 0; b < branch_cnt; b++) { // for each branch
            for (int i = 0; i < reg_cnt; i++) { // for each register
                for (int j = 0; j < BIT32::SIZE; j++) { // for each wire/bit
                    int idx = i * BIT32::SIZE + j;
                    junk_input[b][idx] = prg();
                    Input_Translator(mask[b], reg[i].bits[j], true_gcm[b].input[idx], true_gcm[b].input[idx] ^ true_gcm[b].gc_delta, junk_input[b][idx]);
                }
            }
        }


        std::vector<std::vector<KappaBitString>> SGT(tree_cnt, std::vector<KappaBitString>(SGT_length * 128));        

        for (int i = 0; i < branch_cnt; i++)
            for (int j = 0; j < SGT_length; j++) SGT[ bid_to_tid[i] ][j] = true_gcm[i].gc_material[j];
        for (int i = last_node_with_child; i >= 0; i--) 
            for (int j = 0; j < SGT_length; j++) SGT[i][j] = SGT[i*2 + 1][j] ^ SGT[i*2 + 2][j];
        // transform the SGT
        // std::vector<KappaBitString> SGT = true_gcm[0].gc_material;
        // uint32_t SGT_length = SGT[0].size();

        //SGC::material_buffer.push_back(SGT_length);
        for (uint32_t i = 0; i < SGT_length; i++) SGC::material_buffer.push_back(SGT[0][i]);
        SGC::buffer_idx += SGT_length;
        SGC::material_id += SGT_length;
        SGC::material_buffer.push_back(KappaBitString(0)); // all zero identifier to help parse on the evaluator
        SGC::buffer_idx++;

        // prepare the junk output
        Backup();
        std::vector<std::vector<std::vector<KappaBitString>>> junk_output(branch_cnt); // first for branch id, second for junkid, third for output
        for (int bid = 0; bid < branch_cnt; bid++) {
            int now = bid_to_tid[bid];
            // int brother_now = ((now + 1) ^ 1) - 1;
            std::vector<KappaBitString> acc_fake(SGT_length, 0);
            std::vector<KappaBitString> acc_true = SGT[0];
            for (int i = 0; i < SGT_length; i++) acc_true[i] ^= SGT[now][i];
            while (now) {
                std::vector<KappaBitString> fake_material = SGT[0];
                // update
                for (int i = 0; i < SGT_length; i++) acc_true[i] ^= SGT[ ((now + 1) ^ 1) - 1 ][i];
                for (int i = 0; i < SGT_length; i++) acc_fake[i] ^= expand_material[ ((now + 1) ^ 1) - 1 ][i];
                for (int i = 0; i < SGT_length; i++) fake_material[i] ^= acc_fake[i];
                for (int i = 0; i < SGT_length; i++) fake_material[i] ^= acc_true[i];
                now = (now - 1) >> 1; //  level up parent
                // fake_material is a possible fake one now   
                int lid = junk_output[bid].size();
                junk_output[bid].emplace_back();
                std::cerr << "MAT ID BEFORE GEN" << material_id_before_gen << std::endl;

                EvalGC(fake_material, fragments[bid], junk_input[bid], junk_output[bid][lid], fake_hash_key[bid],
                       material_id_before_gen, SGT_length, bid);
                for (auto &elem : junk_output[bid][lid]) From_Deltai_To_Delta(elem, fake_delta[bid]);
            }
        }

        RestoreBackup();

        // permute true input accordingly
        for (int i = 0; i < branch_cnt; i++)
            for (int j = 0; j < reg_cnt; j++)
                for (int k = 0; k < BIT32::SIZE; k++) {
                    int idx = j*BIT32::SIZE + k;
                    if (true_gcm[i].output[idx][0]) true_gcm[i].output[idx] ^= true_gcm[i].gc_delta ^ SGC::delta;
                }

        std::vector<int> depth(tree_cnt);
        depth[0] = 1;
        for (int i = 1; i < tree_cnt; i++) depth[i] = depth[(i-1)>>1] + 1;
        std::vector<std::vector<int>> calc_junk_order(tree_cnt); // represent the recursively dfs order, TODO: make it optimal
        // TODO: make the bound tight
        for (int i = last_node_with_child; i >= 0; i--) {
            for (auto x : calc_junk_order[i*2+1]) calc_junk_order[i].push_back(x);
            for (auto x : calc_junk_order[i*2+2]) calc_junk_order[i].push_back(x);
            calc_junk_order[i].push_back(i);
        }
        
        // MUX of G

        std::vector<KappaBitString> mux_tree(tree_cnt); // to save the garble label
        std::vector<KappaBitString> junk_tree(tree_cnt); // to simulate and save the junk label
        std::vector<KappaBitString> mux_pad(tree_cnt); // to save the mux table (from G to E), G needs to use it to generate junk

        for (int rid = 0; rid < reg_cnt; rid++) // for every register
            for (int wid = 0; wid < BIT32::SIZE; wid++) // for every bit/wire, mux it
            {
                int idx = rid * BIT32::SIZE + wid;
                for (int i = 0; i < branch_cnt; i++) mux_tree[ bid_to_tid[i] ] = true_gcm[i].output[idx];
                for (int i = last_node_with_child; i >= 0; i--) { 
                    for (int j = range[i].first; j <= range[i].second; j++) junk_tree[ bid_to_tid[j] ] = junk_output[j][ junk_output[j].size() - depth[i] ][idx];
                    // resursively(not by recursive) find fixed junk
                    for (auto x : calc_junk_order[i*2 + 1]) {
                        junk_tree[x] = junk_tree[x*2 + 1] ^ junk_tree[x*2 + 2];
                        if (static_cast<bool>(mask_path[x*2 + 1].wire[0]) == 0) junk_tree[x] ^= SGC::HashSGC(mask_path[x*2 + 1].wire);
                        else junk_tree[x] ^= SGC::HashSGC(mask_path[x*2 + 1].wire) ^ mux_pad[x];
                    }
                    for (auto x : calc_junk_order[i*2+2]) {
                        junk_tree[x] = junk_tree[x*2 + 1] ^ junk_tree[x*2 + 2];
                        if (static_cast<bool>(mask_path[x*2 + 1].wire[0]) == 0) junk_tree[x] ^= SGC::HashSGC(mask_path[x*2 + 1].wire);
                        else junk_tree[x] ^= SGC::HashSGC(mask_path[x*2 + 1].wire) ^ mux_pad[x];
                    }
                    mux_pad[i] = SGC::HashSGC(mask_path[i*2 + 1].wire) ^ SGC::HashSGC(mask_path[i*2 + 1].wire ^ SGC::delta) ^ \
                                 junk_tree[i*2 + 1] ^ junk_tree[i*2 + 2] ^ \
                                 mux_tree[i*2 + 1] ^ mux_tree[i*2 + 2];
                    if (static_cast<bool>(mask_path[i*2 + 1].wire[0]) == 0) mux_tree[i] = junk_tree[i*2 + 1] ^ mux_tree[i*2 + 2] ^ SGC::HashSGC(mask_path[i*2 + 1].wire);
                    else mux_tree[i] = junk_tree[i*2 + 2] ^ mux_tree[i*2 + 1] ^ SGC::HashSGC(mask_path[i*2 + 1].wire ^ SGC::delta);
                    SGC::material_buffer.push_back(mux_pad[i]);
                    SGC::material_id++;
                    SGC::buffer_idx++;
                }
                reg[rid].bits[wid].wire = mux_tree[0];
            }

    }

    */

    /*

    void Demux_E(std::vector<Fragment> &fragments, std::vector<Bit32<Role::Evaluator>> &reg) {
      std::cerr << "material id at the start of demux " << SGC::material_id << std::endl;
        // Backup();
        uint32_t branch_cnt = fragments.size();

        // optimize it?
        uint32_t tree_depth = 0;
        while ( (1 << (tree_depth + 1)) < branch_cnt ) tree_depth++;
        // Number of nodes if tree was full
        uint32_t fully_tree_cnt = (1 << (tree_depth + 1)) - 1;
        // Number of nodes in the actual not full tree
        uint32_t tree_cnt = fully_tree_cnt + 2 * ( branch_cnt - (1 << tree_depth) );

        // Compute true seed (at the leaves) and true_tree
        // (seeds at each node of the tree, need to be expanded properly from root seed)
        std::vector<KappaBitString> seed_tree(tree_cnt);
        std::vector<KappaBitString> hash_key(branch_cnt);
        std::vector<KappaBitString> delta_xor(branch_cnt);
        std::vector<uint32_t> bid_to_tid(branch_cnt); // mapping from branch id to tree id (ie leaves)
        for(int i = 0; i < tree_cnt; i++) {
            if (i * 2 + 1 >= tree_cnt) { 
                if (i >= fully_tree_cnt) {
                    bid_to_tid[i - fully_tree_cnt] = i;
                } else { 
                    bid_to_tid[ branch_cnt - (fully_tree_cnt - i) ] = i;
                }
            } 
        }

        // RestoreBackup(); // TODO: dislike this way

        // for (int i = 0; i < branch_cnt; i++) std::cout << i << " : " << bid_to_tid[i] << std::endl;

        // std::cout << branch_cnt << ' ' << tree_cnt << ' ' << fully_tree_cnt << ' ' << tree_depth << std::endl;

        std::cerr << "sgc mat id 1 " << SGC::material_id << std::endl;
        // one hot encoding of condition bits
        std::vector<Bit<Role::Evaluator>> mask(branch_cnt);
        for (uint32_t i = 0; i < branch_cnt; i++) mask[i] = (reg[0] == i);

        std::cerr << "sgc mat id 2 " << SGC::material_id << std::endl;

        //for (int i = 0; i < branch_cnt; i++) mask[i].Reveal();

        // mask_path: 1 on the path, else 0
        std::vector<Bit<Role::Evaluator>> mask_path(tree_cnt);
        // mask_tree: 1 right off the path, else 0
        std::vector<Bit<Role::Evaluator>> mask_tree(tree_cnt);
        // range of indices in child nodes
        std::vector<std::pair<int, int>> range(tree_cnt);
        for (uint32_t i = 0; i < branch_cnt; i++) {
            mask_path[ bid_to_tid[i] ] = mask[i];
            range[ bid_to_tid[i] ] = std::make_pair(i, i);
        }
        // TODO: check the bound
        for (int i = ((tree_cnt - 2) >> 1); i >= 0; i--) {
            mask_path[i] = mask_path[i*2 + 1] ^ mask_path[i*2 + 2];
            range[i] = std::make_pair( range[i*2+1].first, range[i*2+2].second );
        }
        for (int i = tree_cnt - 1; i > 0; i--) mask_tree[i] = mask_path[i] ^ mask_path[(i-1) >> 1];
    
        // for (int i = 1; i < tree_cnt; i++) mask_tree[i].Reveal();

        // receiving one-hot stuff based on mask
        for (int i = 1; i < tree_cnt; i++) Share_Seed(mask_tree[i], seed_tree[i]);   
        for (int i = 0; i < branch_cnt; i++) Share_Seed(mask[i], hash_key[i]);
        for (int i = 0; i < branch_cnt; i++) Share_Seed(mask[i], delta_xor[i]);

        std::vector<std::vector<KappaBitString>> input(branch_cnt, std::vector<KappaBitString>(reg_cnt * BIT32::SIZE));
        std::vector<std::vector<KappaBitString>> output(branch_cnt);

        for (int b = 0; b < branch_cnt; b++) { // for each branch
            for (int i = 0; i < reg_cnt; i++) { // for each register
                for (int j = 0; j < BIT32::SIZE; j++) { // for each wire/bit
                    int idx = i * BIT32::SIZE + j;
                    input[b][idx] = Input_Translator(mask[b], reg[i].bits[j]);
                }
            }
        }

        // for (int i = 1; i < tree_cnt; i++)    
        //     std::cerr << seed_tree[i] << std::endl;

        // transform the SGT
        //uint32_t SGT_length = SGC::get_next_buffer().to_ulong();
        //std::vector<KappaBitString> SGT(SGT_length);
        //for (uint32_t i = 0; i < SGT_length; i++) SGT[i] = SGC::get_next_buffer();
        std::vector<KappaBitString> SGT;
        uint32_t SGT_length;
        SGC::retrieve_material(SGT, SGT_length);

        std::cerr << "SGT length: " << SGT_length << std::endl;
        std::cerr << "SGc mat id: " << SGC::material_id << std::endl;
        std::cerr << "SGc mat length: " << SGC::material_buffer.size() << std::endl;

        // Before calculating material, backup everything for future to recover
        Backup();

        std::vector<std::vector<KappaBitString>> expand_material(tree_cnt);

      int material_id_before_gen = SGC::material_id;

        for (int i = 1; i < tree_cnt; i++) {
          std::cerr << "SGT len 1 " << SGT_length << std::endl;
            int len = range[i].second - range[i].first + 1;
            //int log_len = log2(len);
            //std::vector<KappaBitString> expand_seed(len);
            std::queue<KappaBitString> q;
            q.push(seed_tree[i]);
            for (int j = 1; j < len; j++) {
                SGC::PRG _prg(q.front()); q.pop();
                q.push(_prg());
                q.push(_prg());
            }
            uint32_t zero = 0;
            //std::cerr << i << ' ' << seed << std::endl;
            GenGCMaterial(fragments[range[i].first], q.front(), zero, expand_material[i], SGT_length,
                range[i].first); q.pop();
            std::vector<KappaBitString> gc_material;
            for (int j = 1; j < len; j++) {
                gc_material.clear();
                GenGCMaterial(fragments[range[i].first + j], q.front(), zero, gc_material, SGT_length,
                              range[i].first + j); q.pop();
                for (int k = 0 ; k < SGT_length; k++) {
                  //std::cerr << "SGT len " << SGT_length << std::endl;
                  //std::cerr << "k" << k << std::endl;
                  //std::cerr << "gc material size" << gc_material.size() << std::endl;
                  expand_material[i][k] ^= gc_material[k];
                }
            }
            //std::cerr << "[DEBUG] " << i << std::endl;
            //for (int k = 0 ; k < SGT_length; k++) std::cerr << expand_material[i][k] << std::endl;
        }
        SGC::material_id += SGT_length * branch_cnt;
        std::cerr << "SGT len 2 " << SGT_length << std::endl;

        int wire_size = reg_cnt * BIT32::SIZE;
        for (int i = 0; i < branch_cnt; i++) {
            std::vector<KappaBitString> active_material = SGT;
            int now = bid_to_tid[i];
            while (now) {
                int brother = ((now + 1) ^ 1) - 1;
                for (int j = 0; j < SGT_length; j++) active_material[j] ^= expand_material[brother][j];
                now = (now - 1) >> 1;
            }
          std::cerr << "MAT ID BEFORE GEN" << material_id_before_gen << std::endl;
            EvalGC(active_material, fragments[i], input[i], output[i], hash_key[i],
                   material_id_before_gen, SGT_length, i);
            //RestoreBackup(); // TODO: check this            
            for (int j = 0; j < wire_size; j++) From_Deltai_To_Delta(output[i][j], delta_xor[i]);
            
        }



        RestoreBackup();

        // mux for E
        for (int rid = 0; rid < reg_cnt; rid++) // for every register
            for (int wid = 0; wid < BIT32::SIZE; wid++) // for every bit/wire, mux it
            {
                int idx = rid * BIT32::SIZE + wid;
                // TODO: optimize, save more memory?
                std::vector<KappaBitString> mux_tree(tree_cnt);
                for (int i = 0; i < branch_cnt; i++) mux_tree[ bid_to_tid[i] ] = output[i][idx];

                for (int i = ((tree_cnt - 2) >> 1); i >= 0; i--) {
                    mux_tree[i] = mux_tree[i*2 + 1] ^ mux_tree[i*2 + 2];
                    KappaBitString mux_pad = SGC::get_next_buffer();
                    if (static_cast<bool>(mask_path[i*2 + 1].wire[0]) == 0) mux_tree[i] ^= SGC::HashSGC(mask_path[i*2 + 1].wire);
                    else mux_tree[i] ^= SGC::HashSGC(mask_path[i*2 + 1].wire) ^ mux_pad;
                }     
                reg[rid].bits[wid].wire = mux_tree[0];
            }
        
    }
    */

    // garbling and junk
    std::pair<KappaBitString, KappaBitString> GbCondAdd(KappaBitString &x, KappaBitString &b, KappaBitString &r) {
        std::pair<KappaBitString, KappaBitString> res;
        r = HashSGC(x) ^ HashSGC(x ^ SGC::delta) ^ (b[0] ? SGC::delta : 0);
        SGC::material_buffer.push_back(r);
        res.first = (x[0] ? r : 0) ^ HashSGC(x) ^ (b[0] ? 0 : x);
        res.second = res.first ^ x;
        return res;
    }

    // garbling or junk
    std::pair<KappaBitString, KappaBitString> EvCondAdd(KappaBitString &x, KappaBitString &b) {
        std::pair<KappaBitString, KappaBitString> res;
        KappaBitString r = SGC::get_next_buffer();
        KappaBitString T = (x[0] ? r : 0) ^ HashSGC(x);
        res.first = T ^ (b[0] ? 0 : x);
        res.second = T ^ (b[0] ? x : 0);
        return res;
    }

    // ev in-gb-head
    std::pair<KappaBitString, KappaBitString> EvCondAdd(KappaBitString &x, KappaBitString &b, KappaBitString &r) {
        std::pair<KappaBitString, KappaBitString> res;
        KappaBitString T = (x[0] ? r : 0) ^ HashSGC(x);
        res.first = T ^ (b[0] ? 0 : x);
        res.second = T ^ (b[0] ? x : 0);        
        return res;
    }

    void EvBranchingTree(std::vector<KappaBitString> &b, std::vector<std::vector<KappaBitString>> &res) {
        int k = b.size(); // we have 2^k branch
        // initialize res to expectation size
        res.resize(k);
        for (int i = 0; i < k; i++) res[i].resize((1 << (i+1)) - 1);
        // initialize first row in the res's forest
        for (int i = 0; i < k; i++) res[i][0] = b[i];
        // evaluate node by node
        // cond/depth 0 -> 3
        // wid depth+1 -> 3
        // get material for b_{wid}|b_{cond} then expand res forests
        for (int cond = 0; cond < k; cond++)
            for (int wid = cond+1; wid < k; wid++) {
                KappaBitString r = SGC::get_next_buffer(); // take materials for wid | cond
                // go every possible wid, cond and expand the wid's tree
                // (1 << cond) different cases
                int case_cnt = 1 << cond;
                int nodeid = case_cnt - 1;
                for (int i = 0; i < case_cnt; i++, nodeid++) {
                    // branch wire = res[cond][nodeid]
                    // x wire = res[wid][nodeid]
                    KappaBitString T = (res[wid][nodeid][0] ? r : 0) ^ HashSGC(res[wid][nodeid]);
                    res[wid][nodeid*2 + 1] = T ^ (res[cond][nodeid][0] ? 0 : res[wid][nodeid]);
                    res[wid][nodeid*2 + 2] = T ^ (res[cond][nodeid][0] ? res[wid][nodeid] : 0);
                }                
            }
    }

    // x is the input wire
    void EvBitCondAdd(std::vector<std::vector<KappaBitString>> &passing_tree, KappaBitString &x, std::vector<KappaBitString> &res) {
        int k = passing_tree.size(); // we ahve 2^k branch
        // tranlate from cond_0 to cond_{k-1}
        // initial the res_tree for x
        res.resize((1 << (k+1)) - 1);
        // initial the root
        res[0] = x;
        for (int cond = 0; cond < k; cond++) {
            KappaBitString r = SGC::get_next_buffer(); // take matereials for x | cond
            int case_cnt = 1 << cond;
            int nodeid = case_cnt - 1;
            for (int i = 0; i < case_cnt; i++, nodeid++) {
                // branch wire = passing_tree[cond][nodeid]
                // x wire = res[nodeid]
                KappaBitString T = (res[nodeid][0] ? r : 0) ^ HashSGC(passing_tree[cond][nodeid]);
                res[nodeid*2 + 1] = T ^ (passing_tree[cond][nodeid][0] ? 0 : res[nodeid]);
                res[nodeid*2 + 2] = T ^ (passing_tree[cond][nodeid][0] ? res[nodeid] : 0);
            }
        }
    }

    // For Garbler to Generate M for EvBranchingTree
    // Key Idea: Garbler can directly assume branch 0 is active
    // The entire tree would be just an path + several tree
    // with rearrangement but value is the same 
    void GbBranchingTree(std::vector<KappaBitString> &b, std::vector<std::vector<KappaBitString>> &res) {
        int k = b.size(); // we have 2^k branch
        // initialize res to expectation size
        res.resize(k);
        for (int i = 0; i < k; i++) res[i].resize((1 << (i+1)) - 1);
        // initialize first row in the res's forest
        for (int i = 0; i < k; i++) res[i][0] = b[i];
        // garble node by node
        // cond/depth 0 -> 3
        // wid depth+1 -> 3
        // send material for b_{wid}|b_{cond} then expand res (junk) forests
        for (int cond = 0; cond < k; cond++)
            for (int wid = cond+1; wid < k; wid++) {
                KappaBitString r;
                // (1 << cond) different cases
                int case_cnt = 1 << cond;
                // handle special case when i = 0, because G assumes active branch is 0
                int nodeid = case_cnt - 1;
                // generate and send material for wid | cond        
                // tmppair store one true one junk                
                auto tmppair = GbCondAdd(res[wid][nodeid], res[cond][nodeid], r);
                // put true on left child, junk on right child
                res[wid][nodeid*2 + 1] = tmppair.first;
                res[wid][nodeid*2 + 2] = tmppair.second;
                // go every possible wid, cond and expand the wid's tree (i.e., junk expanding)
                nodeid++;
                for (int i = 1; i < case_cnt; i++, nodeid++) {
                    // branch wire = res[cond][nodeid]
                    // x wire = res[wid][nodeid]
                    KappaBitString T = (res[wid][nodeid][0] ? r : 0) ^ HashSGC(res[wid][nodeid]);
                    res[wid][nodeid*2 + 1] = T ^ (res[cond][nodeid][0] ? 0 : res[wid][nodeid]);
                    res[wid][nodeid*2 + 2] = T ^ (res[cond][nodeid][0] ? res[wid][nodeid] : 0);
                }                
            }                
    }

    // Similar idea as above
    void GbBitCondAdd(std::vector<std::vector<KappaBitString>> &passing_tree, KappaBitString &x, std::vector<KappaBitString> &res) {
        int k = passing_tree.size(); // we ahve 2^k branch
        // tranlate from cond_0 to cond_{k-1}
        // initial the res_tree for x
        res.resize((1 << (k+1)) - 1);
        // initial the root
        res[0] = x;
        for (int cond = 0; cond < k; cond++) {
            KappaBitString r;
            int case_cnt = 1 << cond;
            int nodeid = case_cnt - 1;
            // generate materials for x | cond
            auto tmppair = GbCondAdd(res[nodeid], passing_tree[cond][nodeid], r);
            res[nodeid*2 + 1] = tmppair.first;
            res[nodeid*2 + 2] = tmppair.second;
            nodeid++;
            // junk expanding
            for (int i = 1; i < case_cnt; i++, nodeid++) {
                // branch wire = passing_tree[cond][nodeid]
                // x wire = res[nodeid]
                KappaBitString T = (res[nodeid][0] ? r : 0) ^ HashSGC(passing_tree[cond][nodeid]);
                res[nodeid*2 + 1] = T ^ (passing_tree[cond][nodeid][0] ? 0 : res[nodeid]);
                res[nodeid*2 + 2] = T ^ (passing_tree[cond][nodeid][0] ? res[nodeid] : 0);
            }
        }        
    }

    void GbSeedTree(std::vector<Bit<Role::Garbler>> &point_func, std::vector<KappaBitString> &bit_encode, 
                    std::vector<KappaBitString> &good_seed, std::vector<KappaBitString> &bad_seed) {
        int branch_cnt = point_func.size();   
        int depth = 0;
        while ((1 << depth) < branch_cnt) depth++;
        int tree_size = (1 << (depth+1)) - 1;
        int tree_size_wo_leaf = (1 << depth) - 1;
        good_seed.resize(tree_size);
        bad_seed.resize(tree_size);
        // assign seed
        good_seed[0] = prg();
        // good seed is correlated
        for (int i = 0; i < tree_size_wo_leaf; i++) {
            SGC::PRG _prg(good_seed[i]);                
            good_seed[i * 2 + 1] = _prg();
            good_seed[i * 2 + 2] = _prg();
        }

        // accumulate the point_func
        std::vector<KappaBitString> accumulate_tree;
        accumulate_tree.resize(tree_size);
        for (int i = 0; i < branch_cnt; i++) accumulate_tree[tree_size_wo_leaf + i] = point_func[i].wire;
        for (int i = tree_size_wo_leaf - 1; i >= 0; i--) accumulate_tree[i] = accumulate_tree[i * 2 + 1] ^ accumulate_tree[i * 2 + 2];

        // transfer the sortinghat
        // bad seed is not correlated and determine by GRR
        for (int i = 1; i < tree_size_wo_leaf + branch_cnt; i++) {
            int father_id = (i - 1) >> 1;
            KappaBitString encrypt_key = accumulate_tree[i] ^ accumulate_tree[father_id];
            KappaBitString r = HashSGC(encrypt_key ^ SGC::delta) ^ good_seed[i];
            //SGC::material_buffer.push_back(r);    
            SGC::AddToBuffer(r);
            bad_seed[i] = HashSGC(encrypt_key) ^ r;                    
        }

        // calculating bit_encode, which will be used in split/join
        bit_encode.resize(depth);
        for (int i = 0; i < depth; i++) {
            bit_encode[i] = 0;
            int right_bound = (1 << (i + 2));
            for (int j = (1 << (i + 1)); j < right_bound; j += 2) bit_encode[i] ^= accumulate_tree[j];
        }
    }   

    void EvSeedTree(std::vector<Bit<Role::Evaluator>> &point_func, std::vector<KappaBitString> &bit_encode, std::vector<KappaBitString> &seed_tree) {
        int branch_cnt = point_func.size();   
        int depth = 0;
        while ((1 << depth) < branch_cnt) depth++;
        int tree_size = (1 << (depth+1)) - 1;
        int tree_size_wo_leaf = (1 << depth) - 1;
        seed_tree.resize(tree_size);
        seed_tree[0] = 0;

        // accumulate the point_func
        std::vector<KappaBitString> accumulate_tree;
        accumulate_tree.resize(tree_size);
        for (int i = 0; i < branch_cnt; i++) accumulate_tree[tree_size_wo_leaf + i] = point_func[i].wire;
        for (int i = tree_size_wo_leaf - 1; i >= 0; i--) accumulate_tree[i] = accumulate_tree[i * 2 + 1] ^ accumulate_tree[i * 2 + 2];

        // get and decrypt the sortinghat
        for (int i = 1; i < tree_size_wo_leaf + branch_cnt; i++) {
            int father_id = (i - 1) >> 1;
            KappaBitString decrypt_key = accumulate_tree[i] ^ accumulate_tree[father_id];
            KappaBitString r = SGC::get_next_buffer();
            seed_tree[i] = HashSGC(decrypt_key) ^ r;                  
        }

        // calculating bit_encode, which will be used in split/join
        bit_encode.resize(depth);
        for (int i = 0; i < depth; i++) {
            bit_encode[i] = 0;
            int right_bound = (1 << (i + 2));
            for (int j = (1 << (i + 1)); j < right_bound; j += 2) bit_encode[i] ^= accumulate_tree[j];
        }
    }

    std::pair<KappaBitString, KappaBitString> GbCondAdd(KappaBitString &x, KappaBitString &b, KappaBitString &offset, KappaBitString &r) {
        std::pair<KappaBitString, KappaBitString> res;
        KappaBitString Hash_X0 = HashSGC(x);
        KappaBitString Hash_X1 = HashSGC(x ^ offset);
        r = Hash_X0 ^ Hash_X1 ^ (b[0] ? offset : 0);
        res.first = (x[0] ? r : 0) ^ Hash_X0 ^ (b[0] ? 0 : x);
        res.second = res.first ^ x;
        return res;
    }

    // global delta
    // K0 := Delta ^ Delta_1
    // K1 := Delta ^ Delta_0
    inline void GbTransDelta(KappaBitString &b, KappaBitString &offset, KappaBitString K0, KappaBitString K1, KappaBitString &r0, KappaBitString &r1) {
        KappaBitString Hash_B0 = HashSGC(b);
        KappaBitString Hash_B1 = HashSGC(b ^ offset);
        r0 = b[0] ? (Hash_B1 ^ K1) : (Hash_B0 ^ K0);
        r1 = b[0] ? (Hash_B0 ^ K0) : (Hash_B1 ^ K1);
    }

    void GbSplitMux(KappaBitString left_seed, KappaBitString right_seed, KappaBitString current_delta, std::vector<KappaBitString> &b, std::vector<KappaBitString> &x, std::vector<KappaBitString> &gbmaterials) {
        SGC::PRG left_prg(left_seed);
        KappaBitString left_left_seed = left_prg();
        KappaBitString left_right_seed = left_prg();
        KappaBitString left_delta = left_prg();
        left_delta[0] = 1;
        SGC::PRG right_prg(right_seed);
        KappaBitString right_left_seed = right_prg();
        KappaBitString right_right_seed = right_prg();
        KappaBitString right_delta = right_prg(); 
        right_delta[0] = 1;

        // data for next layer
        std::vector<KappaBitString> b_left;
        std::vector<KappaBitString> b_right;
        std::vector<KappaBitString> x_left;
        std::vector<KappaBitString> x_right;


        // b[0] is conditon, acting on b[1] -> end, and also x
        // first, garbler transfer (delta ^ delta_{b[0]}) to E
        KappaBitString Hash_B0 = HashSGC(b[0]);
        KappaBitString Hash_B1 = HashSGC(b[0] ^ current_delta);
        KappaBitString delta_delta0 = current_delta ^ left_delta;
        KappaBitString delta_delta1 = current_delta ^ right_delta;

        KappaBitString transdelta_r0 = b[0][0] ? (Hash_B1 ^ delta_delta1) : (Hash_B0 ^ delta_delta0);
        KappaBitString transdelta_r1 = b[0][0] ? (Hash_B0 ^ delta_delta0) : (Hash_B1 ^ delta_delta1);
        gbmaterials.push_back(transdelta_r0); gbmaterials.push_back(transdelta_r1);

        // first, finished

        // second, for every wire translated, translate
        for (int i = 1; i < b.size(); i++) {
            KappaBitString mat;
            auto condadd0 = GbCondAdd(b[i], b[0], current_delta, mat); // first is label, second is junk
            auto condadd1 = condadd0;
            condadd0.first = condadd0.first ^ (condadd0.first[0] ? delta_delta0 : 0);
            //condadd0.second = condadd0.second ^ (condadd0.second[0] ? delta_delta1 : 0);
            condadd1.first = condadd1.first ^ (condadd1.first[0] ? delta_delta1 : 0);
            //condadd1.second = condadd1.second ^ (condadd1.second[0] ? delta_delta0 : 0);

            gbmaterials.push_back(mat);

            KappaBitString x0 = left_prg();
            KappaBitString x1 = right_prg();
            b_left.push_back(x0); b_right.push_back(x1);

            KappaBitString transwire_r0 = condadd0.first ^ x0 ^ Hash_B0;
            KappaBitString transwire_r1 = condadd1.first ^ x1 ^ Hash_B1;

            gbmaterials.push_back(transwire_r0); gbmaterials.push_back(transwire_r1);
            //condadd0.second ^= transwire_r0 ^ Hash_B1;
            //condadd1.second ^= transwire_r1 ^ Hash_B0;

        }

        for (int i = 0; i < x.size(); i++) {
            KappaBitString mat;
            auto condadd0 = GbCondAdd(x[i], b[0], current_delta, mat); // first is label, second is junk
            auto condadd1 = condadd0;
            condadd0.first = condadd0.first ^ (condadd0.first[0] ? delta_delta0 : 0);
            //condadd0.second = condadd0.second ^ (condadd0.second[0] ? delta_delta1 : 0);
            condadd1.first = condadd1.first ^ (condadd1.first[0] ? delta_delta1 : 0);
            //condadd1.second = condadd1.second ^ (condadd1.second[0] ? delta_delta0 : 0);

            gbmaterials.push_back(mat);

            KappaBitString x0 = left_prg();
            KappaBitString x1 = right_prg();
            x_left.push_back(x0); x_right.push_back(x1);

            KappaBitString transwire_r0 = condadd0.first ^ x0 ^ Hash_B0;
            KappaBitString transwire_r1 = condadd1.first ^ x1 ^ Hash_B1;

            gbmaterials.push_back(transwire_r0); gbmaterials.push_back(transwire_r1);
            //condadd0.second ^= transwire_r0 ^ Hash_B1;
            //condadd1.second ^= transwire_r1 ^ Hash_B0;            
        }

        // if no b is translated, i.e., b.size() == 1, this means that we move to the leaf layer
        if (b.size() == 1) return;

        // otherwise, we need to recursively call and stacking them into this layer
        std::vector<KappaBitString> gbmaterials_left;
        std::vector<KappaBitString> gbmaterials_right;

        GbSplitMux(left_left_seed, left_right_seed, left_delta, b_left, x_left, gbmaterials_left);
        GbSplitMux(right_left_seed, right_right_seed, right_delta, b_right, x_right, gbmaterials_right); 

        for (int i = 0; i < gbmaterials_left.size(); i++) gbmaterials.push_back(gbmaterials_left[i] ^ gbmaterials_right[i]); // stack together          
    }

    // this version with SplitReply record
    void GbSplitMux(int node_id, KappaBitString left_seed, KappaBitString right_seed, KappaBitString current_delta, 
                    std::vector<KappaBitString> &b, std::vector<KappaBitString> &x, 
                    std::vector<KappaBitString> &gbmaterials,
                    SplitReply &record) {

        if (node_id >= record.b.size()) record.b.resize(node_id + 1);
        record.b[node_id] = b[0];
        
        SGC::PRG left_prg(left_seed);
        KappaBitString left_left_seed = left_prg();
        KappaBitString left_right_seed = left_prg();
        KappaBitString left_delta = left_prg();
        left_delta[0] = 1;
        SGC::PRG right_prg(right_seed);
        KappaBitString right_left_seed = right_prg();
        KappaBitString right_right_seed = right_prg();
        KappaBitString right_delta = right_prg(); 
        right_delta[0] = 1;

        if (node_id * 2 + 2 >= record.delta.size()) record.delta.resize(node_id * 2 + 3);
        record.delta[node_id * 2 + 1] = left_delta;
        record.delta[node_id * 2 + 2] = right_delta;

        // data for next layer
        std::vector<KappaBitString> b_left;
        std::vector<KappaBitString> b_right;
        std::vector<KappaBitString> x_left;
        std::vector<KappaBitString> x_right;


        // b[0] is conditon, acting on b[1] -> end, and also x
        // first, garbler transfer (delta ^ delta_{b[0]}) to E
        KappaBitString Hash_B0 = HashSGC(b[0]);
        KappaBitString Hash_B1 = HashSGC(b[0] ^ current_delta);
        KappaBitString delta_delta0 = current_delta ^ left_delta;
        KappaBitString delta_delta1 = current_delta ^ right_delta;

        KappaBitString transdelta_r0 = b[0][0] ? (Hash_B1 ^ delta_delta1) : (Hash_B0 ^ delta_delta0);
        KappaBitString transdelta_r1 = b[0][0] ? (Hash_B0 ^ delta_delta0) : (Hash_B1 ^ delta_delta1);
        gbmaterials.push_back(transdelta_r0); gbmaterials.push_back(transdelta_r1);

        // first, finished

        // second, for every wire translated, translate
        for (int i = 1; i < b.size(); i++) {
            KappaBitString mat;
            auto condadd0 = GbCondAdd(b[i], b[0], current_delta, mat); // first is label, second is junk
            auto condadd1 = condadd0;
            condadd0.first = condadd0.first ^ (condadd0.first[0] ? delta_delta0 : 0);
            //condadd0.second = condadd0.second ^ (condadd0.second[0] ? delta_delta1 : 0);
            condadd1.first = condadd1.first ^ (condadd1.first[0] ? delta_delta1 : 0);
            //condadd1.second = condadd1.second ^ (condadd1.second[0] ? delta_delta0 : 0);

            gbmaterials.push_back(mat);

            KappaBitString x0 = left_prg();
            KappaBitString x1 = right_prg();
            b_left.push_back(x0); b_right.push_back(x1);

            KappaBitString transwire_r0 = condadd0.first ^ x0 ^ Hash_B0;
            KappaBitString transwire_r1 = condadd1.first ^ x1 ^ Hash_B1;

            gbmaterials.push_back(transwire_r0); gbmaterials.push_back(transwire_r1);
            //condadd0.second ^= transwire_r0 ^ Hash_B1;
            //condadd1.second ^= transwire_r1 ^ Hash_B0;

        }

        for (int i = 0; i < x.size(); i++) {
            KappaBitString mat;
            auto condadd0 = GbCondAdd(x[i], b[0], current_delta, mat); // first is label, second is junk
            auto condadd1 = condadd0;
            condadd0.first = condadd0.first ^ (condadd0.first[0] ? delta_delta0 : 0);
            //condadd0.second = condadd0.second ^ (condadd0.second[0] ? delta_delta1 : 0);
            condadd1.first = condadd1.first ^ (condadd1.first[0] ? delta_delta1 : 0);
            //condadd1.second = condadd1.second ^ (condadd1.second[0] ? delta_delta0 : 0);

            gbmaterials.push_back(mat);

            KappaBitString x0 = left_prg();
            KappaBitString x1 = right_prg();
            x_left.push_back(x0); x_right.push_back(x1);

            KappaBitString transwire_r0 = condadd0.first ^ x0 ^ Hash_B0;
            KappaBitString transwire_r1 = condadd1.first ^ x1 ^ Hash_B1;

            gbmaterials.push_back(transwire_r0); gbmaterials.push_back(transwire_r1);
            //condadd0.second ^= transwire_r0 ^ Hash_B1;
            //condadd1.second ^= transwire_r1 ^ Hash_B0;            
        }

        if (node_id * 2 + 2 >= record.prg.size()) record.prg.resize(node_id * 2 + 3);
        record.prg[node_id * 2 + 1] = left_prg;
        record.prg[node_id * 2 + 2] = right_prg;

        // if no b is translated, i.e., b.size() == 1, this means that we move to the leaf layer
        if (b.size() == 1) return;

        // otherwise, we need to recursively call and stacking them into this layer
        std::vector<KappaBitString> gbmaterials_left;
        std::vector<KappaBitString> gbmaterials_right;

        GbSplitMux(node_id * 2 + 1, left_left_seed, left_right_seed, left_delta, b_left, x_left, gbmaterials_left, record);
        GbSplitMux(node_id * 2 + 2, right_left_seed, right_right_seed, right_delta, b_right, x_right, gbmaterials_right, record); 

        for (int i = 0; i < gbmaterials_left.size(); i++) gbmaterials.push_back(gbmaterials_left[i] ^ gbmaterials_right[i]); // stack together          
    }

    void GbSplitMuxWithJunkProcess(int node_id, std::vector<std::vector<KappaBitString>> &junk_mat, 
                                   KappaBitString left_seed, KappaBitString right_seed, KappaBitString current_delta, 
                                   std::vector<KappaBitString> &b, std::vector<KappaBitString> &x, std::vector<KappaBitString> &gbmaterials,
                                   std::vector<std::vector<std::vector<KappaBitString>>> &junkres,
                                   std::vector<KappaBitString> &delta_tree, std::vector<KappaBitString> &cond_tree,
                                   std::vector<std::vector<KappaBitString>> &junk_b, std::vector<std::vector<KappaBitString>> &junk_delta,
                                   std::vector<SGC::PRG> &prg_tree) {

        cond_tree[node_id] = b[0];

        SGC::PRG left_prg(left_seed);
        KappaBitString left_left_seed = left_prg();
        KappaBitString left_right_seed = left_prg();
        KappaBitString left_delta = left_prg();
        left_delta[0] = 1;
        delta_tree[node_id * 2 + 1] = left_delta;
        SGC::PRG right_prg(right_seed);
        KappaBitString right_left_seed = right_prg();
        KappaBitString right_right_seed = right_prg();
        KappaBitString right_delta = right_prg(); 
        right_delta[0] = 1;
        delta_tree[node_id * 2 + 2] = right_delta;

        // data for next layer
        std::vector<KappaBitString> b_left;
        std::vector<KappaBitString> b_right;
        std::vector<KappaBitString> x_left;
        std::vector<KappaBitString> x_right;

        std::vector<KappaBitString> junk_b_left;
        std::vector<KappaBitString> junk_b_right;
        std::vector<KappaBitString> junk_x_left;
        std::vector<KappaBitString> junk_x_right;


        // b[0] is conditon, acting on b[1] -> end, and also x
        // first, garbler transfer (delta ^ delta_{b[0]}) to E
        KappaBitString Hash_B0 = HashSGC(b[0]);
        KappaBitString Hash_B1 = HashSGC(b[0] ^ current_delta);
        KappaBitString delta_delta0 = current_delta ^ left_delta;
        KappaBitString delta_delta1 = current_delta ^ right_delta;

        KappaBitString transdelta_r0 = b[0][0] ? (Hash_B1 ^ delta_delta1) : (Hash_B0 ^ delta_delta0);
        KappaBitString transdelta_r1 = b[0][0] ? (Hash_B0 ^ delta_delta0) : (Hash_B1 ^ delta_delta1);
        gbmaterials.push_back(transdelta_r0); gbmaterials.push_back(transdelta_r1);

        // first, finished

        // second, for every wire translated, translate
        for (int i = 1; i < b.size(); i++) {
            KappaBitString mat;
            auto condadd0 = GbCondAdd(b[i], b[0], current_delta, mat); // first is label, second is junk
            auto condadd1 = condadd0;
            condadd0.first = condadd0.first ^ (condadd0.first[0] ? delta_delta0 : 0);
            condadd0.second = condadd0.second ^ (condadd0.second[0] ? delta_delta1 : 0);
            condadd1.first = condadd1.first ^ (condadd1.first[0] ? delta_delta1 : 0);
            condadd1.second = condadd1.second ^ (condadd1.second[0] ? delta_delta0 : 0);

            gbmaterials.push_back(mat);

            KappaBitString x0 = left_prg();
            KappaBitString x1 = right_prg();
            b_left.push_back(x0); b_right.push_back(x1);

            KappaBitString transwire_r0 = condadd0.first ^ x0 ^ Hash_B0;
            KappaBitString transwire_r1 = condadd1.first ^ x1 ^ Hash_B1;

            gbmaterials.push_back(transwire_r0); gbmaterials.push_back(transwire_r1);
            condadd0.second ^= transwire_r0 ^ Hash_B1;
            condadd1.second ^= transwire_r1 ^ Hash_B0;
            junk_b_left.push_back(condadd0.second); junk_b_right.push_back(condadd1.second);
        }

        for (int i = 0; i < x.size(); i++) {
            KappaBitString mat;
            auto condadd0 = GbCondAdd(x[i], b[0], current_delta, mat); // first is label, second is junk
            auto condadd1 = condadd0;
            condadd0.first = condadd0.first ^ (condadd0.first[0] ? delta_delta0 : 0);
            condadd0.second = condadd0.second ^ (condadd0.second[0] ? delta_delta1 : 0);
            condadd1.first = condadd1.first ^ (condadd1.first[0] ? delta_delta1 : 0);
            condadd1.second = condadd1.second ^ (condadd1.second[0] ? delta_delta0 : 0);

            gbmaterials.push_back(mat);

            KappaBitString x0 = left_prg();
            KappaBitString x1 = right_prg();
            x_left.push_back(x0); x_right.push_back(x1);

            KappaBitString transwire_r0 = condadd0.first ^ x0 ^ Hash_B0;
            KappaBitString transwire_r1 = condadd1.first ^ x1 ^ Hash_B1;

            gbmaterials.push_back(transwire_r0); gbmaterials.push_back(transwire_r1);
            condadd0.second ^= transwire_r0 ^ Hash_B1;
            condadd1.second ^= transwire_r1 ^ Hash_B0;         
            junk_x_left.push_back(condadd0.second); junk_x_right.push_back(condadd1.second);   
        }

        if (node_id * 2 + 2 >= prg_tree.size()) prg_tree.resize(node_id * 2 + 3);
        prg_tree[node_id * 2 + 1] = left_prg;
        prg_tree[node_id * 2 + 2] = right_prg;

        // if no b is translated, i.e., b.size() == 1, this means that we move to the leaf layer
        if (b.size() == 1) {
            junkres[node_id * 2 + 1].push_back(junk_x_left);
            junkres[node_id * 2 + 2].push_back(junk_x_right);
            return;
        }

        // otherwise, we need to recursively call and stacking them into this layer
        std::vector<KappaBitString> gbmaterials_left;
        std::vector<KappaBitString> gbmaterials_right;

        GbSplitMuxWithJunkProcess(node_id * 2 + 1, junk_mat, left_left_seed, left_right_seed, left_delta, b_left, x_left, gbmaterials_left, junkres, delta_tree, cond_tree, junk_b, junk_delta, prg_tree);
        GbSplitMuxWithJunkProcess(node_id * 2 + 2, junk_mat, right_left_seed, right_right_seed, right_delta, b_right, x_right, gbmaterials_right, junkres, delta_tree, cond_tree, junk_b, junk_delta, prg_tree);

        // here is the place to collect junk  
        std::vector<KappaBitString> junkmaterials_left;
        std::vector<KappaBitString> junkmaterials_right;

        for (int i = 0; i < gbmaterials_left.size(); i++) {
            KappaBitString stack_mat = gbmaterials_left[i] ^ gbmaterials_right[i];
            gbmaterials.push_back(stack_mat); // stack together    
            junkmaterials_left.push_back(stack_mat ^ junk_mat[node_id * 2 + 2][i]);
            junkmaterials_right.push_back(stack_mat ^ junk_mat[node_id * 2 + 1][i]);
        }

        // execute E-in-head
        EvSplitMux(node_id * 2 + 1, junk_mat, junk_b_left, junk_x_left, junkmaterials_left, junkres[node_id * 2 + 1], junk_b[node_id * 2 + 1], junk_delta[node_id * 2 + 1]);
        EvSplitMux(node_id * 2 + 2, junk_mat, junk_b_right, junk_x_right, junkmaterials_right, junkres[node_id * 2 + 2], junk_b[node_id * 2 + 2], junk_delta[node_id * 2 + 2]);
    }

    void EvSplitMux(int node_id, std::vector<KappaBitString> &seed_tree, std::vector<KappaBitString> &b, std::vector<KappaBitString> &x, std::vector<KappaBitString> &evmaterials) {
        // data for next layer
        std::vector<KappaBitString> b_left;
        std::vector<KappaBitString> b_right;
        std::vector<KappaBitString> x_left;
        std::vector<KappaBitString> x_right;

        // b[0] is conditon, acting on b[1] -> end, and also x
        // first, evaluator get (delta ^ delta_{b[0]})
        KappaBitString Hash_B = HashSGC(b[0]);
        KappaBitString delta_deltab = Hash_B ^ (b[0][0] ? evmaterials[1] : evmaterials[0]);
        // first, finished

        int mid = 2;

        // second, for every wire translated, translate
        for (int i = 1; i < b.size(); i++) {
            // E's condadd
            KappaBitString condaddd_r = evmaterials[mid++];
            KappaBitString Hash_X = HashSGC(b[i]);
            KappaBitString T = (b[i][0] ? condaddd_r : 0) ^ Hash_X;
            std::pair<KappaBitString, KappaBitString> condadd;
            condadd.first = T ^ (b[0][0] ? 0 : b[i]);            
            condadd.second = T ^ (b[0][0] ? b[i] : 0);

            // E's translate delta
            condadd.first ^= condadd.first[0] ? delta_deltab : 0;
            condadd.second ^= condadd.second[0] ? delta_deltab : 0;

            // E's translate wire
            condadd.first ^= evmaterials[mid++] ^ Hash_B;
            condadd.second ^= evmaterials[mid++] ^ Hash_B;            

            // store to next layer
            b_left.push_back(condadd.first); b_right.push_back(condadd.second);           
        }

        for (int i = 0; i < x.size(); i++) {
            // E's condadd
            KappaBitString condaddd_r = evmaterials[mid++];
            KappaBitString Hash_X = HashSGC(x[i]);
            KappaBitString T = (x[i][0] ? condaddd_r : 0) ^ Hash_X;
            std::pair<KappaBitString, KappaBitString> condadd;
            condadd.first = T ^ (b[0][0] ? 0 : x[i]);            
            condadd.second = T ^ (b[0][0] ? x[i] : 0);

            // E's translate delta
            condadd.first ^= condadd.first[0] ? delta_deltab : 0;
            condadd.second ^= condadd.second[0] ? delta_deltab : 0;

            // E's translate wire
            condadd.first ^= evmaterials[mid++] ^ Hash_B;
            condadd.second ^= evmaterials[mid++] ^ Hash_B;            

            // store to next layer
            x_left.push_back(condadd.first); x_right.push_back(condadd.second);            
        }

        // if no b is translated, i.e., b.size() == 1, this means that we move to the leaf layer
        if (b.size() == 1) {
            std::cout << "EV " << node_id << ' ' << x_left[0] << ' ' << x_right[0] << std::endl;
            return;
        }
        
        // recursively call
        // evaluator garbles left/right branch using corresponding seed
        // a. left
        SGC::PRG left_prg(seed_tree[node_id * 2 + 1]);
        KappaBitString left_left_seed = left_prg();
        KappaBitString left_right_seed = left_prg();
        KappaBitString left_delta = left_prg(); left_delta[0] = 1;
        std::vector<KappaBitString> gb_b_left;
        for (int i = 1; i < b.size(); i++) gb_b_left.push_back(left_prg());
        std::vector<KappaBitString> gb_x_left;
        for (int i = 0; i < x.size(); i++) gb_x_left.push_back(left_prg());
        std::vector<KappaBitString> gbmaterials_left;
        GbSplitMux(left_left_seed, left_right_seed, left_delta, gb_b_left, gb_x_left, gbmaterials_left);
        // b. right
        SGC::PRG right_prg(seed_tree[node_id * 2 + 2]);
        KappaBitString right_left_seed = right_prg();
        KappaBitString right_right_seed = right_prg();
        KappaBitString right_delta = right_prg(); right_delta[0] = 1;
        std::vector<KappaBitString> gb_b_right;
        for (int i = 1; i < b.size(); i++) gb_b_right.push_back(right_prg());
        std::vector<KappaBitString> gb_x_right;
        for (int i = 0; i < x.size(); i++) gb_x_right.push_back(right_prg());
        std::vector<KappaBitString> gbmaterials_right;
        GbSplitMux(right_left_seed, right_right_seed, right_delta, gb_b_right, gb_x_right, gbmaterials_right);  

        // xor-out (unstack) to next layer ev
        std::vector<KappaBitString> evmaterials_left;
        std::vector<KappaBitString> evmaterials_right;
        for (int i = 0; i < gbmaterials_left.size(); i++) {
            evmaterials_left.push_back(evmaterials[mid] ^ gbmaterials_right[i]);
            evmaterials_right.push_back(evmaterials[mid++] ^ gbmaterials_left[i]);
        }

        // recursively ev
        EvSplitMux(node_id * 2 + 1, seed_tree, b_left, x_left, evmaterials_left);
        EvSplitMux(node_id * 2 + 2, seed_tree, b_right, x_right, evmaterials_right);
    }

    void EvSplitMux(int node_id, std::vector<std::vector<KappaBitString>> &mat_tree, 
                    std::vector<KappaBitString> &b, std::vector<KappaBitString> &x, std::vector<KappaBitString> &evmaterials, 
                    std::vector<std::vector<KappaBitString>> &evres,
                    std::vector<KappaBitString> &evb, std::vector<KappaBitString> &evdelta) {
        // save evb
        evb.push_back(b[0]);

        // data for next layer
        std::vector<KappaBitString> b_left;
        std::vector<KappaBitString> b_right;
        std::vector<KappaBitString> x_left;
        std::vector<KappaBitString> x_right;

        // b[0] is conditon, acting on b[1] -> end, and also x
        // first, evaluator get (delta ^ delta_{b[0]})
        KappaBitString Hash_B = HashSGC(b[0]);
        KappaBitString delta_deltab = Hash_B ^ (b[0][0] ? evmaterials[1] : evmaterials[0]);
        // first, finished

        // save ebdelta
        evdelta.push_back(delta_deltab);

        int mid = 2;

        // second, for every wire translated, translate
        for (int i = 1; i < b.size(); i++) {
            // E's condadd
            KappaBitString condaddd_r = evmaterials[mid++];
            KappaBitString Hash_X = HashSGC(b[i]);
            KappaBitString T = (b[i][0] ? condaddd_r : 0) ^ Hash_X;
            std::pair<KappaBitString, KappaBitString> condadd;
            condadd.first = T ^ (b[0][0] ? 0 : b[i]);            
            condadd.second = T ^ (b[0][0] ? b[i] : 0);

            // E's translate delta
            condadd.first ^= condadd.first[0] ? delta_deltab : 0;
            condadd.second ^= condadd.second[0] ? delta_deltab : 0;

            // E's translate wire
            condadd.first ^= evmaterials[mid++] ^ Hash_B;
            condadd.second ^= evmaterials[mid++] ^ Hash_B;            

            // store to next layer
            b_left.push_back(condadd.first); b_right.push_back(condadd.second);           
        }

        for (int i = 0; i < x.size(); i++) {
            // E's condadd
            KappaBitString condaddd_r = evmaterials[mid++];

            KappaBitString Hash_X = HashSGC(x[i]);
            KappaBitString T = (x[i][0] ? condaddd_r : 0) ^ Hash_X;
            std::pair<KappaBitString, KappaBitString> condadd;
            condadd.first = T ^ (b[0][0] ? 0 : x[i]);            
            condadd.second = T ^ (b[0][0] ? x[i] : 0);

            // E's translate delta
            condadd.first ^= condadd.first[0] ? delta_deltab : 0;
            condadd.second ^= condadd.second[0] ? delta_deltab : 0;

            // E's translate wire
            condadd.first ^= evmaterials[mid++] ^ Hash_B;
            condadd.second ^= evmaterials[mid++] ^ Hash_B;            

            // store to next layer
            x_left.push_back(condadd.first); x_right.push_back(condadd.second);            
        }

        // if no b is translated, i.e., b.size() == 1, this means that we move to the leaf layer
        if (b.size() == 1) {
            evres.push_back(x_left);
            evres.push_back(x_right);
/*             int depth = 0;
            while ((1 << depth) - 1 <= node_id) depth++;
            int base = (1 << depth) - 1;
            for (int i = 0; i < x_left.size(); i++) evres[node_id * 2 + 1 - base].push_back(x_left[i]); 
            for (int i = 0; i < x_right.size(); i++) evres[node_id * 2 + 2 - base].push_back(x_right[i]);
 */            
            return;
        }
        
        // xor-out (unstack) to next layer ev
        std::vector<KappaBitString> evmaterials_left;
        std::vector<KappaBitString> evmaterials_right;
        for (int i = 0; i < mat_tree[node_id * 2 + 1].size(); i++) {
            evmaterials_left.push_back(evmaterials[mid] ^ mat_tree[node_id * 2 + 2][i]);
            evmaterials_right.push_back(evmaterials[mid++] ^ mat_tree[node_id * 2 + 1][i]);
        }

        // recursively ev
        std::vector<KappaBitString> evb_left;
        std::vector<KappaBitString> evb_right;
        std::vector<KappaBitString> evdelta_left;
        std::vector<KappaBitString> evdelta_right;
        EvSplitMux(node_id * 2 + 1, mat_tree, b_left, x_left, evmaterials_left, evres, evb_left, evdelta_left);
        EvSplitMux(node_id * 2 + 2, mat_tree, b_right, x_right, evmaterials_right, evres, evb_right, evdelta_right);
        // merge into evb/evdelta !!!!! in correct order !!!!!
        int pl = 0, pr = 0;
        for (int i = 0; ; i++) {
            for (int j = 0; j < (1 << i); j++)
                if (pl < evb_left.size()) evb.push_back(evb_left[pl++]);
            for (int j = 0; j < (1 << i); j++)
                if (pr < evb_right.size()) evb.push_back(evb_right[pr++]);
            if (pl >= evb_left.size() && pr >= evb_right.size()) break;
        }
        pl = 0, pr = 0;
        for (int i = 0; ; i++) {
            for (int j = 0; j < (1 << i); j++)
                if (pl < evdelta_left.size()) evdelta.push_back(evdelta_left[pl++]);
            for (int j = 0; j < (1 << i); j++)
                if (pr < evdelta_right.size()) evdelta.push_back(evdelta_right[pr++]);
            if (pl >= evdelta_left.size() && pr >= evdelta_right.size()) break;
        }        
    }

    // Gb's Flash Split
    void GbFlashSplit(int node_id, SplitReply &record, std::vector<KappaBitString> &x, std::vector<KappaBitString> &gbmaterials) {

        // data for next layer
        std::vector<KappaBitString> x_left;
        std::vector<KappaBitString> x_right;


        // b[0] is conditon, acting on b[1] -> end, and also x
        // first, garbler transfer (delta ^ delta_{b[0]}) to E
        KappaBitString Hash_B0 = HashSGC(record.b[node_id]);
        KappaBitString Hash_B1 = HashSGC(record.b[node_id] ^ record.delta[node_id]);
        KappaBitString delta_delta0 = record.delta[node_id] ^ record.delta[node_id * 2 + 1];
        KappaBitString delta_delta1 = record.delta[node_id] ^ record.delta[node_id * 2 + 2];

        KappaBitString transdelta_r0 = record.b[node_id][0] ? (Hash_B1 ^ delta_delta1) : (Hash_B0 ^ delta_delta0);
        KappaBitString transdelta_r1 = record.b[node_id][0] ? (Hash_B0 ^ delta_delta0) : (Hash_B1 ^ delta_delta1);
/*         gbmaterials.push_back(transdelta_r0); gbmaterials.push_back(transdelta_r1);
 */
        // first, finished

        for (int i = 0; i < x.size(); i++) {
            KappaBitString mat;
            auto condadd0 = GbCondAdd(x[i], record.b[node_id], record.delta[node_id], mat); // first is label, second is junk
            auto condadd1 = condadd0;
            condadd0.first = condadd0.first ^ (condadd0.first[0] ? delta_delta0 : 0);
            condadd1.first = condadd1.first ^ (condadd1.first[0] ? delta_delta1 : 0);
 
            gbmaterials.push_back(mat);

            KappaBitString x0 = record.prg[node_id * 2 + 1]();
            KappaBitString x1 = record.prg[node_id * 2 + 2]();
            x_left.push_back(x0); x_right.push_back(x1);

            KappaBitString transwire_r0 = condadd0.first ^ x0 ^ Hash_B0;
            KappaBitString transwire_r1 = condadd1.first ^ x1 ^ Hash_B1;

            gbmaterials.push_back(transwire_r0); gbmaterials.push_back(transwire_r1);
            //condadd0.second ^= transwire_r0 ^ Hash_B1;
            //condadd1.second ^= transwire_r1 ^ Hash_B0;            
        }

        // if no b is translated, i.e., b.size() == 1, this means that we move to the leaf layer
        if (node_id * 2 + 1 >= record.b.size()) return;

        // otherwise, we need to recursively call and stacking them into this layer
        std::vector<KappaBitString> gbmaterials_left;
        std::vector<KappaBitString> gbmaterials_right;

        GbFlashSplit(node_id * 2 + 1, record, x_left, gbmaterials_left);
        GbFlashSplit(node_id * 2 + 2, record, x_right, gbmaterials_right); 

        for (int i = 0; i < gbmaterials_left.size(); i++) gbmaterials.push_back(gbmaterials_left[i] ^ gbmaterials_right[i]); // stack together          
    }

    void GenerateSplitMatTree(std::vector<KappaBitString> &seed_tree, int depth, int x_cnt, 
                              std::vector<std::vector<KappaBitString>> &mat_tree, std::vector<SplitReply> &split_record) {
        int mat_tree_size = (1 << depth) - 1;
        mat_tree.resize(mat_tree_size);
        split_record.resize(mat_tree_size);
        for (int i = 1; i < depth; i++) {
            for (int j = 0; j < (1 << i); j++) {
                int node_id = (1 << i) - 1 + j;
                SGC::PRG tmp_prg(seed_tree[node_id]);
                KappaBitString left_seed = tmp_prg();
                KappaBitString right_seed = tmp_prg();
                KappaBitString tmp_delta = tmp_prg(); tmp_delta[0] = 1;
                std::vector<KappaBitString> b;
                for (int k = i; k < depth; k++) b.push_back(tmp_prg());
                std::vector<KappaBitString> x;
                for (int k = 0; k < x_cnt; k++) x.push_back(tmp_prg());
                split_record[node_id].prg.resize(1);
                split_record[node_id].prg[0] = tmp_prg;
                split_record[node_id].delta.resize(1);
                split_record[node_id].delta[0] = tmp_delta;
                GbSplitMux(0, left_seed, right_seed, tmp_delta, b, x, mat_tree[node_id], split_record[node_id]);
            }
        }
    }

    void GenerateSplitMatTree(std::vector<KappaBitString> &seed_tree, int depth, int x_cnt, std::vector<std::vector<KappaBitString>> &mat_tree) {
        int mat_tree_size = (1 << depth) - 1;
        mat_tree.resize(mat_tree_size);
        for (int i = 1; i < depth; i++) {
            for (int j = 0; j < (1 << i); j++) {
                int node_id = (1 << i) - 1 + j;
                SGC::PRG tmp_prg(seed_tree[node_id]);
                KappaBitString left_seed = tmp_prg();
                KappaBitString right_seed = tmp_prg();
                KappaBitString tmp_delta = tmp_prg(); tmp_delta[0] = 1;
                std::vector<KappaBitString> b;
                for (int k = i; k < depth; k++) b.push_back(tmp_prg());
                std::vector<KappaBitString> x;
                for (int k = 0; k < x_cnt; k++) x.push_back(tmp_prg());
                GbSplitMux(left_seed, right_seed, tmp_delta, b, x, mat_tree[node_id]);
            }
        }
    }

    // JoinMux
    // Ev
    KappaBitString EvJoinMux(std::vector<KappaBitString> &mat_tree, std::vector<KappaBitString> &b_tree, std::vector<KappaBitString> &delta_tree, std::vector<KappaBitString> &x) {
        std::vector<KappaBitString> eval_tree(mat_tree.size() + x.size());
        for (int i = 0; i < x.size(); i++) {
            eval_tree[b_tree.size() + i] = x[i];
        }
        for (int node_id = b_tree.size() - 1; node_id >= 0; node_id--) {
            if (eval_tree[node_id * 2 + 1][0]) eval_tree[node_id * 2 + 1] ^= delta_tree[node_id];
            if (eval_tree[node_id * 2 + 2][0]) eval_tree[node_id * 2 + 2] ^= delta_tree[node_id];
            eval_tree[node_id] = HashSGC(b_tree[node_id]) ^ eval_tree[node_id * 2 + 1] ^ eval_tree[node_id * 2 + 2] ^ (b_tree[node_id][0] ? mat_tree[node_id] : 0);
        }
        // std::cout << "EVJUNK " << eval_tree[3] << ' ' << eval_tree[4] << std::endl;
        return eval_tree[0];
    }

    // Gb
    KappaBitString GbJoinMux(int node_id, std::vector<KappaBitString> &x_array, 
                             std::vector<KappaBitString> &delta_tree, std::vector<KappaBitString> &cond_tree,
                             std::vector<std::vector<KappaBitString>> &junk_b,
                             std::vector<std::vector<KappaBitString>> &junk_delta,
                             std::vector<std::vector<KappaBitString>> &junk_input,
                             std::vector<KappaBitString> &gbmaterials) {
        
        if (node_id + 1 >= x_array.size()) {
            return x_array[node_id - x_array.size() + 1];
        }

        std::vector<KappaBitString> gbmaterials_left;
        std::vector<KappaBitString> gbmaterials_right;

        KappaBitString garble_left = GbJoinMux(node_id * 2 + 1, x_array, delta_tree, cond_tree, junk_b, junk_delta, junk_input, gbmaterials_left);
        KappaBitString garble_right = GbJoinMux(node_id * 2 + 2, x_array, delta_tree, cond_tree, junk_b, junk_delta, junk_input, gbmaterials_right);

        KappaBitString junk_left = EvJoinMux(gbmaterials_left, junk_b[node_id * 2 + 1], junk_delta[node_id * 2 + 1], junk_input[node_id * 2 + 1]);
        KappaBitString junk_right = EvJoinMux(gbmaterials_right, junk_b[node_id * 2 + 2], junk_delta[node_id * 2 + 2], junk_input[node_id * 2 + 2]);

        KappaBitString delta_delta0 = delta_tree[node_id] ^ delta_tree[node_id * 2 + 1];
        KappaBitString delta_delta1 = delta_tree[node_id] ^ delta_tree[node_id * 2 + 2];   

        if (garble_left[0]) garble_left ^= delta_delta0;
        if (junk_left[0]) junk_left ^= delta_delta1;
        if (garble_right[0]) garble_right ^= delta_delta1;
        if (junk_right[0]) junk_right ^= delta_delta0;

        KappaBitString key0 = HashSGC(cond_tree[node_id]);
        KappaBitString key1 = HashSGC(cond_tree[node_id] ^ delta_tree[node_id]);

        KappaBitString rr = key0 ^ key1 ^ garble_left ^ garble_right ^ junk_left ^ junk_right;
        KappaBitString outx = key0 ^ garble_left ^ junk_right ^ (cond_tree[node_id][0] ? rr : 0);

/*         if (node_id == 1)
            std::cout << "JUNK " << junk_left << ' ' << junk_right << std::endl;
 */
        gbmaterials.push_back(rr);

        // combine gbm_left + gbm_right + rr -> gbm  
        int pl = 0, pr = 0;
        for (int i = 0; ; i++) {
            for (int j = 0; j < (1 << i); j++) 
                if (pl < gbmaterials_left.size()) gbmaterials.push_back(gbmaterials_left[pl++]);
            for (int j = 0; j < (1 << i); j++)
                if (pr < gbmaterials_right.size()) gbmaterials.push_back(gbmaterials_right[pr++]);
            if (pl >= gbmaterials_left.size() && pr >= gbmaterials_right.size()) break;
        }

        return outx;


    }

    /*
    // Ev for a fragment
    void EvFragment(std::vector<SGC::Bit32<Role::Evaluator>> &current_reg, SGC::Fragment &fragment, std::vector<KappaBitString> &evmaterials) {
        std::cout << "@@@@@@@@@@@@@" << std::endl;
        SGC::temp_buffer_idx = 0;
        for (Inst inst : fragment) {
            switch (inst.op) {
                case OPCODE::ADD: // ADD
                    current_reg[inst.dst] = current_reg[inst.src1].Add(current_reg[inst.src2], evmaterials);
                    break;
                case OPCODE::STORE: // MUL
                    current_reg[inst.dst] = current_reg[inst.src1].Multiply(current_reg[inst.src2], evmaterials);
                    break;
                default:
                    std::cerr << "Invalid OPCODE" << std::endl;
                    exit(255);
            }
        }
    }
    */

    // Gb and Stacking Fragments
    /*
    void GbStackFragments(std::vector<Fragment> &fragments, std::vector<KappaBitString> &good_seed, 
                          std::vector<std::vector<SGC::Bit32<Role::Garbler>>> &final_reg, std::vector<KappaBitString> &gbmaterials) {
        int fg_cnt = fragments.size();
        std::vector<std::vector<KappaBitString>> branch_mat(fg_cnt);
        std::vector<SGC::PRG> branch_prg;
        int max_length = 0;
        int backup_mid = SGC::material_id;
        for (int i = 0; i < fg_cnt; i++) {
            branch_prg.push_back(SGC::PRG(good_seed[i]));
            branch_mat[i].clear();
            std::vector<SGC::Bit32<Role::Garbler>> acc_array;
            GbFragment(branch_prg[i], final_reg[i], fragments[i], branch_mat[i], acc_array);
            if (branch_mat[i].size() > max_length) max_length = branch_mat[i].size(); // calculate the padding length
            SGC::material_id = backup_mid;
        }
        // padding
        for (int i = 0; i < fg_cnt; i++) {
            int diff = max_length - branch_mat[i].size();
            while (diff--) branch_mat[i].push_back(branch_prg[i]());
        }
        // stacking
        gbmaterials.clear();
        for (int i = 0; i < max_length; i++) {
            KappaBitString acc = 0;
            for (int j = 0; j < fg_cnt; j++) acc ^= branch_mat[j][i];
            gbmaterials.push_back(acc);
        }
    }
    */

    // stackfragments with seeds
    void GbStackFragments(int node_id, KappaBitString current_seed,
                          const std::vector<Fragment> &fragments, const int &leave_bound, const int &max_length,
                          std::vector<KappaBitString> &gbmaterials) {
        if (node_id >= leave_bound) {
            int fid = node_id - leave_bound;
            gbmaterials.clear();
            if (fid >= fragments.size()) return;
            SGC::PRG tmp_prg(current_seed);
            std::vector<SGC::Bit32<Role::Garbler>> final_reg;
            int backup_mid = SGC::material_id;
            std::vector<SGC::Bit32<Role::Garbler>> idx_array;
            std::vector<SGC::Bit32<Role::Garbler>> val_array;
            GbFragment(tmp_prg, final_reg, fragments[fid], gbmaterials, idx_array, val_array);
            SGC::material_id = backup_mid;
            int diff = max_length - gbmaterials.size();
            while (diff--) gbmaterials.push_back(tmp_prg());
            return;
        }
        SGC::PRG tmp_prg(current_seed);
        std::vector<KappaBitString> gbmaterials_left;
        std::vector<KappaBitString> gbmaterials_right;

        GbStackFragments(node_id * 2 + 1, tmp_prg(), fragments, leave_bound, max_length, gbmaterials_left);
        GbStackFragments(node_id * 2 + 2, tmp_prg(), fragments, leave_bound, max_length, gbmaterials_right);

        gbmaterials.clear();
        if (gbmaterials_left.size() == 0 && gbmaterials_right.size() == 0) return;
        if (gbmaterials_right.size() == 0) {
            gbmaterials = gbmaterials_left;
            return;
        }
        for (int i = 0; i < gbmaterials_left.size(); i++) gbmaterials.push_back(gbmaterials_left[i] ^ gbmaterials_right[i]);
    }

    void GenerateCondMatTree(std::vector<KappaBitString> &seed_tree, const std::vector<Fragment> &fragments, const int &max_length,
                             std::vector<std::vector<KappaBitString>> &mat_tree) {
        int depth = 0;
        while ((1 << depth) < fragments.size()) depth++;
        int leave_bound = (1 << depth) - 1;
        mat_tree.resize(seed_tree.size());
        for (int i = 1; i < seed_tree.size(); i++) GbStackFragments(i, seed_tree[i], fragments, leave_bound, max_length, mat_tree[i]);
    }    
    
    /*

    void EvStackFragments(int node_id, std::vector<std::vector<KappaBitString>> &mat_tree, 
                          std::vector<Fragment> &fragments, const int &leave_bound,
                          std::vector<KappaBitString> &evmaterials,
                          std::vector<std::vector<KappaBitString>> &ev_input,
                          std::vector<std::vector<SGC::Bit32<Role::Evaluator>>> &ev_output) {
        if (node_id >= leave_bound) {
            int fid = node_id - leave_bound;
            if (fid >= fragments.size()) return;
            for (int i = 0; i < SGC::reg_cnt; i++) {
                auto GR = SGC::Bit32<Role::Evaluator>();
                for (int j = 0; j < BIT32::SIZE; j++) GR.bits[j].wire = ev_input[fid][BIT32::SIZE * i + j];
                ev_output[fid].push_back(GR);
            }
            auto backup_mid = SGC::material_id;
            EvFragment(ev_output[fid], fragments[fid], evmaterials);
            SGC::material_id = backup_mid;
            return;
        }

        std::vector<KappaBitString> evmaterials_left; evmaterials_left.clear();
        std::vector<KappaBitString> evmaterials_right; evmaterials_right.clear();

        for (int i = 0; i < evmaterials.size(); i++) {
            evmaterials_left.push_back(evmaterials[i] ^ mat_tree[node_id * 2 + 2][i]);
            evmaterials_right.push_back(evmaterials[i] ^ mat_tree[node_id * 2 + 1][i]);
        }

        EvStackFragments(node_id * 2 + 1, mat_tree, fragments, leave_bound, evmaterials_left, ev_input, ev_output);
        EvStackFragments(node_id * 2 + 2, mat_tree, fragments, leave_bound, evmaterials_right, ev_input, ev_output);

    }

    */

    // garbler generate junk output for each branching
    /*
    void GbProcessJunk(int node_id, std::vector<std::vector<KappaBitString>> &mat_tree,
                       std::vector<Fragment> &fragments, const int &leave_bound,
                       int lidx, int ridx,
                       std::vector<std::vector<std::vector<KappaBitString>>> &junk_in,
                       std::vector<std::vector<std::vector<SGC::Bit32<Role::Evaluator>>>> &junk_output,
                       std::vector<KappaBitString> &gbmaterials) {
        if (node_id >= leave_bound) {
            return;
        }
        std::vector<KappaBitString> gbmaterials_left, gbmaterials_right;
        gbmaterials_left.clear(); gbmaterials_right.clear();
        for (int i = 0; i < gbmaterials.size(); i++) {
            gbmaterials_left.push_back(gbmaterials[i] ^ mat_tree[node_id * 2 + 2][i]);
            gbmaterials_right.push_back(gbmaterials[i] ^ mat_tree[node_id * 2 + 1][i]);
        }

        int mid = (lidx + ridx) >> 1;
        std::vector<Fragment> fragments_left, fragments_right;
        fragments_left.clear(); fragments_right.clear();

        for (int i = lidx; i <= mid; i++) {
            int id = i - lidx;
            if (id < fragments.size()) fragments_left.push_back(fragments[id]);
        }
        
        for (int i = mid + 1; i <= ridx; i++) {
            int id = i - lidx;
            if (id < fragments.size()) fragments_right.push_back(fragments[id]);
        }

        if (fragments_left.size()) {
            junk_output[node_id * 2 + 1].resize(fragments_left.size());
            EvStackFragments(node_id * 2 + 1, mat_tree, fragments_left, leave_bound, gbmaterials_left, junk_in[node_id * 2 + 1], junk_output[node_id * 2 + 1]);
        }
        if (fragments_right.size()) {
            junk_output[node_id * 2 + 2].resize(fragments_right.size());
            EvStackFragments(node_id * 2 + 2, mat_tree, fragments_right, leave_bound+(mid-lidx+1), gbmaterials_right, junk_in[node_id * 2 + 2], junk_output[node_id * 2 + 2]);
        }
        if (fragments_left.size())
            GbProcessJunk(node_id * 2 + 1, mat_tree, fragments_left, leave_bound, lidx, mid, junk_in, junk_output, gbmaterials_left);
        if (fragments_right.size())
            GbProcessJunk(node_id * 2 + 2, mat_tree, fragments_right, leave_bound+(mid-lidx+1), mid+1, ridx, junk_in, junk_output, gbmaterials_right);

    }
    */

    void EvGenerateSession(int node_id, std::vector<std::vector<KappaBitString>> &mat_tree, 
                           const std::vector<Fragment> &fragments, const int &leave_bound,
                           std::vector<KappaBitString> &evmaterials,
                           std::vector<std::vector<KappaBitString>> &ev_input,
                           std::vector<SGC::EVSession> &ev_session) {
        if (node_id >= leave_bound) {
            int fid = node_id - leave_bound;
            if (fid >= fragments.size()) return;
            ev_session.push_back(SGC::EVSession());
            for (int i = 0; i < SGC::reg_cnt; i++) {
                auto GR = SGC::Bit32<Role::Evaluator>();
                for (int j = 0; j < BIT32::SIZE; j++) GR.bits[j].wire = ev_input[fid][BIT32::SIZE * i + j];
                ev_session[ev_session.size() - 1].reg.push_back(GR);
            }
            ev_session[ev_session.size() - 1].nonce_id = SGC::material_id;
            ev_session[ev_session.size() - 1].prog = fragments[fid];
            ev_session[ev_session.size() - 1].evmaterials = evmaterials;
            ev_session[ev_session.size() - 1].mat_id = 0;
            ev_session[ev_session.size() - 1].pc = 0;
            return;
        }

        std::vector<KappaBitString> evmaterials_left; evmaterials_left.clear();
        std::vector<KappaBitString> evmaterials_right; evmaterials_right.clear();

        for (int i = 0; i < evmaterials.size(); i++) {
            evmaterials_left.push_back(evmaterials[i] ^ mat_tree[node_id * 2 + 2][i]);
            evmaterials_right.push_back(evmaterials[i] ^ mat_tree[node_id * 2 + 1][i]);
        }

        EvGenerateSession(node_id * 2 + 1, mat_tree, fragments, leave_bound, evmaterials_left, ev_input, ev_session);
        EvGenerateSession(node_id * 2 + 2, mat_tree, fragments, leave_bound, evmaterials_right, ev_input, ev_session);

    }

    // stackfragments with seeds
    void GbStackFragmentsWithJunkProcess(int node_id, KappaBitString current_seed,
                                         const std::vector<Fragment> &fragments, const int &leave_bound, const int &max_length,
                                         std::vector<std::vector<SGC::Bit32<Role::Garbler>>> &final_reg,
                                         std::vector<KappaBitString> &gbmaterials,
                                         std::vector<std::vector<KappaBitString>> &mat_tree,
                                         int lidx, int ridx,
                                         std::vector<std::vector<std::vector<KappaBitString>>> &junk_in,
                                         std::vector<std::vector<SGC::EVSession>> &junk_session,
                                         std::vector<std::vector<SGC::Bit32<Role::Garbler>>> &idx_array,
                                         std::vector<std::vector<SGC::Bit32<Role::Garbler>>> &val_array) {

        // good seed gb           
        if (node_id >= leave_bound) {
            int fid = node_id - leave_bound;
            gbmaterials.clear();
            if (fid >= fragments.size()) return;
            SGC::PRG tmp_prg(current_seed);
            final_reg.push_back(std::vector<SGC::Bit32<Role::Garbler>>());
            idx_array.push_back(std::vector<SGC::Bit32<Role::Garbler>>());
            val_array.push_back(std::vector<SGC::Bit32<Role::Garbler>>());            
            int backup_mid = SGC::material_id;
            GbFragment(tmp_prg, final_reg[final_reg.size()-1], fragments[fid], gbmaterials, idx_array[idx_array.size()-1], val_array[val_array.size()-1]);
            SGC::material_id = backup_mid;
            int diff = max_length - gbmaterials.size();
            while (diff--) gbmaterials.push_back(tmp_prg());
            return;
        }

        SGC::PRG tmp_prg(current_seed);
        std::vector<KappaBitString> gbmaterials_left;
        std::vector<KappaBitString> gbmaterials_right;

        int mid = (lidx + ridx) >> 1;
        std::vector<Fragment> fragments_left, fragments_right;
        fragments_left.clear(); fragments_right.clear();

        for (int i = lidx; i <= mid; i++) {
            int id = i - lidx;
            if (id < fragments.size()) fragments_left.push_back(fragments[id]);
        }
        
        for (int i = mid + 1; i <= ridx; i++) {
            int id = i - lidx;
            if (id < fragments.size()) fragments_right.push_back(fragments[id]);
        }

        // gb left and right (if exists)
        if (fragments_left.size()) GbStackFragmentsWithJunkProcess(node_id * 2 + 1, tmp_prg(), fragments_left, leave_bound, max_length, final_reg, gbmaterials_left, mat_tree, lidx, mid, junk_in, junk_session, idx_array, val_array);
        if (fragments_right.size()) GbStackFragmentsWithJunkProcess(node_id * 2 + 2, tmp_prg(), fragments_right, leave_bound+(mid-lidx+1), max_length, final_reg, gbmaterials_right, mat_tree, mid+1, ridx, junk_in, junk_session, idx_array, val_array);

        // stacking them
        gbmaterials.clear();
        if (gbmaterials_right.size() == 0) gbmaterials = gbmaterials_left;
        else {
            for (int i = 0; i < gbmaterials_left.size(); i++) gbmaterials.push_back(gbmaterials_left[i] ^ gbmaterials_right[i]);
        }

        // process junk for two children
        gbmaterials_left.clear(); gbmaterials_right.clear();
        for (int i = 0; i < gbmaterials.size(); i++) {
            gbmaterials_left.push_back(gbmaterials[i] ^ mat_tree[node_id * 2 + 2][i]);
            gbmaterials_right.push_back(gbmaterials[i] ^ mat_tree[node_id * 2 + 1][i]);
        }

        if (fragments_left.size()) {
            EvGenerateSession(node_id * 2 + 1, mat_tree, fragments_left, leave_bound, gbmaterials_left, junk_in[node_id * 2 + 1], junk_session[node_id * 2 + 1]);
        }
        if (fragments_right.size()) {
            EvGenerateSession(node_id * 2 + 2, mat_tree, fragments_right, leave_bound+(mid-lidx+1), gbmaterials_right, junk_in[node_id * 2 + 2], junk_session[node_id * 2 + 2]);
        }
    }    


    // stackfragments with seeds
    /*
    void GbStackFragmentsWithJunkProcess(int node_id, KappaBitString current_seed,
                                         std::vector<Fragment> &fragments, const int &leave_bound, const int &max_length,
                                         std::vector<std::vector<SGC::Bit32<Role::Garbler>>> &final_reg,
                                         std::vector<KappaBitString> &gbmaterials,
                                         std::vector<std::vector<KappaBitString>> &mat_tree,
                                         int lidx, int ridx,
                                         std::vector<std::vector<std::vector<KappaBitString>>> &junk_in,
                                         std::vector<std::vector<std::vector<SGC::Bit32<Role::Evaluator>>>> &junk_output) {

        // good seed gb           
        if (node_id >= leave_bound) {
            int fid = node_id - leave_bound;
            gbmaterials.clear();
            if (fid >= fragments.size()) return;
            SGC::PRG tmp_prg(current_seed);
            final_reg.push_back(std::vector<SGC::Bit32<Role::Garbler>>());
            int backup_mid = SGC::material_id;
            std::vector<SGC::Bit32<Role::Garbler>> acc_array;
            GbFragment(tmp_prg, final_reg[final_reg.size()-1], fragments[fid], gbmaterials, acc_array);
            SGC::material_id = backup_mid;
            int diff = max_length - gbmaterials.size();
            while (diff--) gbmaterials.push_back(tmp_prg());
            return;
        }

        SGC::PRG tmp_prg(current_seed);
        std::vector<KappaBitString> gbmaterials_left;
        std::vector<KappaBitString> gbmaterials_right;

        int mid = (lidx + ridx) >> 1;
        std::vector<Fragment> fragments_left, fragments_right;
        fragments_left.clear(); fragments_right.clear();

        for (int i = lidx; i <= mid; i++) {
            int id = i - lidx;
            if (id < fragments.size()) fragments_left.push_back(fragments[id]);
        }
        
        for (int i = mid + 1; i <= ridx; i++) {
            int id = i - lidx;
            if (id < fragments.size()) fragments_right.push_back(fragments[id]);
        }

        // gb left and right (if exists)
        if (fragments_left.size()) GbStackFragmentsWithJunkProcess(node_id * 2 + 1, tmp_prg(), fragments_left, leave_bound, max_length, final_reg, gbmaterials_left, mat_tree, lidx, mid, junk_in, junk_output);
        if (fragments_right.size()) GbStackFragmentsWithJunkProcess(node_id * 2 + 2, tmp_prg(), fragments_right, leave_bound+(mid-lidx+1), max_length, final_reg, gbmaterials_right, mat_tree, mid+1, ridx, junk_in, junk_output);

        // stacking them
        gbmaterials.clear();
        if (gbmaterials_right.size() == 0) gbmaterials = gbmaterials_left;
        else {
            for (int i = 0; i < gbmaterials_left.size(); i++) gbmaterials.push_back(gbmaterials_left[i] ^ gbmaterials_right[i]);
        }

        // process junk for two children
        gbmaterials_left.clear(); gbmaterials_right.clear();
        for (int i = 0; i < gbmaterials.size(); i++) {
            gbmaterials_left.push_back(gbmaterials[i] ^ mat_tree[node_id * 2 + 2][i]);
            gbmaterials_right.push_back(gbmaterials[i] ^ mat_tree[node_id * 2 + 1][i]);
        }

        if (fragments_left.size()) {
            junk_output[node_id * 2 + 1].resize(fragments_left.size());
            EvStackFragments(node_id * 2 + 1, mat_tree, fragments_left, leave_bound, gbmaterials_left, junk_in[node_id * 2 + 1], junk_output[node_id * 2 + 1]);
        }
        if (fragments_right.size()) {
            junk_output[node_id * 2 + 2].resize(fragments_right.size());
            EvStackFragments(node_id * 2 + 2, mat_tree, fragments_right, leave_bound+(mid-lidx+1), gbmaterials_right, junk_in[node_id * 2 + 2], junk_output[node_id * 2 + 2]);
        }
    }    
    */

    // GB's Join at the level of Registry
    void GbJoinRegistry(std::vector<std::vector<SGC::Bit32<Role::Garbler>>> &final_reg, // for "active" wires
                        std::vector<std::vector<std::vector<SGC::Bit32<Role::Evaluator>>>> &junk_output, // for determistic "inactive" junk
                        std::vector<KappaBitString> &delta_tree, std::vector<KappaBitString> &cond_tree, 
                        std::vector<std::vector<KappaBitString>> &junk_b, std::vector<std::vector<KappaBitString>> &junk_delta,                        
                        std::vector<SGC::Bit32<Role::Garbler>> &join_reg) {
        auto branch_size = final_reg.size();
        for (int rid = 0; rid < SGC::reg_cnt; rid++) { // for every reg
            for (int wid = 0; wid < BIT32::SIZE; wid++) { // for every wire
                std::vector<KappaBitString> x_array; x_array.clear(); // for active input
                for (int i = 0; i < branch_size; i++) x_array.push_back(final_reg[i][rid].bits[wid].wire);
                std::vector<std::vector<KappaBitString>> junk_input(junk_output.size()); // for "inactive" junk
                for (int i = 1; i < junk_input.size(); i++) {
                    for (int j = 0; j < junk_output[i].size(); j++) {
                        junk_input[i].push_back(junk_output[i][j][rid].bits[wid].wire);
                    }
                }
                std::vector<KappaBitString> gbmaterials; gbmaterials.clear();
                join_reg[rid].bits[wid].wire = GbJoinMux(0, x_array, delta_tree, cond_tree, junk_b, junk_delta, junk_input, gbmaterials);
                for (int i = 0; i < gbmaterials.size(); i++) SGC::AddToBuffer(gbmaterials[i]);
            }
        }
    }

    // GB's Join at the level of Registry
    void GbJoinRegistry(std::vector<std::vector<SGC::Bit32<Role::Garbler>>> &final_reg, // for "active" wires
                        std::vector<std::vector<SGC::EVSession>> &junk_session, // for determistic "inactive" junk
                        std::vector<KappaBitString> &delta_tree, std::vector<KappaBitString> &cond_tree, 
                        std::vector<std::vector<KappaBitString>> &junk_b, std::vector<std::vector<KappaBitString>> &junk_delta,                        
                        std::vector<SGC::Bit32<Role::Garbler>> &join_reg) {
        auto branch_size = final_reg.size();
        for (int rid = 0; rid < SGC::reg_cnt; rid++) { // for every reg
            for (int wid = 0; wid < BIT32::SIZE; wid++) { // for every wire
                //std::cout << "HHHHHHH " << rid << ' ' << wid << "============" << std::endl;
                std::vector<KappaBitString> x_array; x_array.clear(); // for active input
                for (int i = 0; i < branch_size; i++) x_array.push_back(final_reg[i][rid].bits[wid].wire);
                std::vector<std::vector<KappaBitString>> junk_input(junk_session.size()); // for "inactive" junk
                for (int i = 1; i < junk_input.size(); i++) {
                    for (int j = 0; j < junk_session[i].size(); j++) {
                        junk_input[i].push_back(junk_session[i][j].reg[rid].bits[wid].wire);
                    }
                }
                std::vector<KappaBitString> gbmaterials; gbmaterials.clear();
                join_reg[rid].bits[wid].wire = GbJoinMux(0, x_array, delta_tree, cond_tree, junk_b, junk_delta, junk_input, gbmaterials);
                for (int i = 0; i < gbmaterials.size(); i++) SGC::AddToBuffer(gbmaterials[i]);
            }
        }
    }    

    // EV's Join at the level of Rigistry
    void EvJoinRegistry(std::vector<std::vector<SGC::Bit32<Role::Evaluator>>> &final_reg,
                        std::vector<KappaBitString> &evb, std::vector<KappaBitString> &evdelta,
                        std::vector<SGC::Bit32<Role::Evaluator>> &join_reg) {
        auto branch_size = final_reg.size();
        for (int rid = 0; rid < SGC::reg_cnt; rid++) { // for every reg
            for (int wid = 0; wid < BIT32::SIZE; wid++) { // for every wire
                std::vector<KappaBitString> x_array; x_array.clear(); // for active input
                for (int i = 0; i < branch_size; i++) x_array.push_back(final_reg[i][rid].bits[wid].wire);
                std::vector<KappaBitString> join_mat_tree; join_mat_tree.clear();
                for (int i = 0; i < branch_size - 1; i++) join_mat_tree.push_back(SGC::get_next_buffer());
                join_reg[rid].bits[wid].wire = EvJoinMux(join_mat_tree, evb, evdelta, x_array);
            }
        }
    }

    // EV's Join at the level of Rigistry
    void EvJoinRegistry(std::vector<SGC::EVSession> &ev_session,
                        std::vector<KappaBitString> &evb, std::vector<KappaBitString> &evdelta,
                        std::vector<SGC::Bit32<Role::Evaluator>> &join_reg) {
        auto branch_size = ev_session.size();
        for (int rid = 0; rid < SGC::reg_cnt; rid++) { // for every reg
            for (int wid = 0; wid < BIT32::SIZE; wid++) { // for every wire
                //std::cout << "CCCCCC " << rid << ' ' << wid << "==========" << std::endl;
                std::vector<KappaBitString> x_array; x_array.clear(); // for active input
                for (int i = 0; i < branch_size; i++) x_array.push_back(ev_session[i].reg[rid].bits[wid].wire);
                std::vector<KappaBitString> join_mat_tree; join_mat_tree.clear();
                for (int i = 0; i < branch_size - 1; i++) join_mat_tree.push_back(SGC::get_next_buffer());
                join_reg[rid].bits[wid].wire = EvJoinMux(join_mat_tree, evb, evdelta, x_array);
            }
        }
    }

    // Gb's process junk until ACC
    void GbStepProcessJunk(std::vector<std::vector<SGC::EVSession>> &junk_session) {
        for (int i = 0; i < junk_session.size(); i++)
            for (int j = 0; j < junk_session[i].size(); j++)
                junk_session[i][j].Step();
    }

    // Gb's Flash Join
    void GbFlashJoin(int acc_id, std::vector<std::vector<SGC::EVSession>> &junk_session, 
                     std::vector<std::vector<SGC::Bit32<Role::Garbler>>> &idx_array,
                     std::vector<std::vector<SGC::Bit32<Role::Garbler>>> &val_array,
                     std::vector<KappaBitString> &delta_tree, std::vector<KappaBitString> &cond_tree, 
                     std::vector<std::vector<KappaBitString>> &junk_b, std::vector<std::vector<KappaBitString>> &junk_delta,                        
                     std::vector<KappaBitString> &flashmaterials,
                     SGC::Bit32<Role::Garbler> &flash_idx,
                     SGC::Bit32<Role::Garbler> &flash_val) {
        auto branch_size = idx_array.size();
        for (auto wid = 0; wid < BIT32::SIZE; wid++) { // for every wire as idx
            std::vector<KappaBitString> x_array; x_array.clear();
            for (int i = 0; i < branch_size; i++) x_array.push_back(acc_id < idx_array[i].size() ? idx_array[i][acc_id].bits[wid].wire : 0);
            std::vector<std::vector<KappaBitString>> junk_input(junk_session.size()); // for "inactive" junk
            for (int i = 1; i < junk_input.size(); i++) {
                for (int j = 0; j < junk_session[i].size(); j++) {
                    junk_input[i].push_back(junk_session[i][j].idx_reg.bits[wid].wire);
                }
            }
            flash_idx.bits[wid].wire = GbJoinMux(0, x_array, delta_tree, cond_tree, junk_b, junk_delta, junk_input, flashmaterials);
        }
        for (auto wid = 0; wid < BIT32::SIZE; wid++) { // for every wire as val
            std::vector<KappaBitString> x_array; x_array.clear();
            for (int i = 0; i < branch_size; i++) x_array.push_back(acc_id < val_array[i].size() ? val_array[i][acc_id].bits[wid].wire : 0);
            std::vector<std::vector<KappaBitString>> junk_input(junk_session.size()); // for "inactive" junk
            for (int i = 1; i < junk_input.size(); i++) {
                for (int j = 0; j < junk_session[i].size(); j++) {
                    junk_input[i].push_back(junk_session[i][j].val_reg.bits[wid].wire);
                }
            }
            flash_val.bits[wid].wire = GbJoinMux(0, x_array, delta_tree, cond_tree, junk_b, junk_delta, junk_input, flashmaterials);
        }
    }

    // Ev's Flash Join
    void EvFlashJoin(int acc_id, std::vector<SGC::EVSession> &ev_session, 
                     std::vector<KappaBitString> &evb, std::vector<KappaBitString> &evdelta,
                     SGC::Bit32<Role::Evaluator> &flash_idx,
                     SGC::Bit32<Role::Evaluator> &flash_val) {
        auto branch_size = ev_session.size();
        for (int wid = 0; wid < BIT32::SIZE; wid++) { // for every wire
            std::vector<KappaBitString> x_array; x_array.clear(); // for active input
            for (int i = 0; i < branch_size; i++) x_array.push_back(ev_session[i].idx_reg.bits[wid].wire);
            std::vector<KappaBitString> join_mat_tree; join_mat_tree.clear();
            for (int i = 0; i < branch_size - 1; i++) join_mat_tree.push_back(SGC::get_next_buffer());
            flash_idx.bits[wid].wire = EvJoinMux(join_mat_tree, evb, evdelta, x_array);
        }        
        for (int wid = 0; wid < BIT32::SIZE; wid++) { // for every wire
            std::vector<KappaBitString> x_array; x_array.clear(); // for active input
            for (int i = 0; i < branch_size; i++) x_array.push_back(ev_session[i].val_reg.bits[wid].wire);
            std::vector<KappaBitString> join_mat_tree; join_mat_tree.clear();
            for (int i = 0; i < branch_size - 1; i++) join_mat_tree.push_back(SGC::get_next_buffer());
            flash_val.bits[wid].wire = EvJoinMux(join_mat_tree, evb, evdelta, x_array);
        }        
    }

    // Gb's ACC
    void GbACC(SGC::Bit32<Role::Garbler> &flash_src, SGC::Bit32<Role::Garbler> &flash_dst,
               std::vector<KappaBitString> &flashmaterials) {
        // now is just a simple not gate
        for (int i = 0; i < BIT32::SIZE; i++) flash_dst.bits[i].wire = flash_src.bits[i].wire ^ SGC::delta;
    }

    // Ev's ACC
    void EvACC(SGC::Bit32<Role::Evaluator> &flash_src, SGC::Bit32<Role::Evaluator> &flash_dst) {
        // now is juat a simple not gate
        for (int i = 0; i < BIT32::SIZE; i++) flash_dst.bits[i].wire = flash_src.bits[i].wire;
    }

    // Flash Split

    void GbFlashSplit(int node_id, 
                      std::vector<KappaBitString> &x, 
                      std::vector<KappaBitString> &gbmaterials,
                      SplitReply &record) {

        // data for next layer
        std::vector<KappaBitString> x_left;
        std::vector<KappaBitString> x_right;


        // b[0] is conditon, acting on b[1] -> end, and also x
        // first, garbler transfer (delta ^ delta_{b[0]}) to E
        KappaBitString Hash_B0 = HashSGC(record.b[node_id]);
        KappaBitString Hash_B1 = HashSGC(record.b[node_id] ^ record.delta[node_id]);
        KappaBitString delta_delta0 = record.delta[node_id] ^ record.delta[node_id * 2 + 1];
        KappaBitString delta_delta1 = record.delta[node_id] ^ record.delta[node_id * 2 + 2];

        for (int i = 0; i < x.size(); i++) {
            KappaBitString mat;
            auto condadd0 = GbCondAdd(x[i], record.b[node_id], record.delta[node_id], mat); // first is label, second is junk
            auto condadd1 = condadd0;
            condadd0.first = condadd0.first ^ (condadd0.first[0] ? delta_delta0 : 0);
            condadd1.first = condadd1.first ^ (condadd1.first[0] ? delta_delta1 : 0);

            gbmaterials.push_back(mat);

            KappaBitString x0 = record.prg[node_id * 2 + 1]();
            KappaBitString x1 = record.prg[node_id * 2 + 2]();
            x_left.push_back(x0); x_right.push_back(x1);

            KappaBitString transwire_r0 = condadd0.first ^ x0 ^ Hash_B0;
            KappaBitString transwire_r1 = condadd1.first ^ x1 ^ Hash_B1;

            gbmaterials.push_back(transwire_r0); gbmaterials.push_back(transwire_r1);
        }

        // last layer
        if (node_id * 2 + 1 >= record.b.size()) return;

        // otherwise, we need to recursively call and stacking them into this layer
        std::vector<KappaBitString> gbmaterials_left;
        std::vector<KappaBitString> gbmaterials_right;

        GbFlashSplit(node_id * 2 + 1, x_left, gbmaterials_left, record);
        GbFlashSplit(node_id * 2 + 2, x_right, gbmaterials_right, record); 

        for (int i = 0; i < gbmaterials_left.size(); i++) gbmaterials.push_back(gbmaterials_left[i] ^ gbmaterials_right[i]); // stack together          

    }

    void GenerateFlashSplitMatTree(int depth, int x_cnt, 
                                   std::vector<std::vector<KappaBitString>> &mat_tree, std::vector<SplitReply> &split_record) {
        int mat_tree_size = (1 << depth) - 1;
        mat_tree.resize(mat_tree_size);
        for (int i = 1; i < depth; i++) {
            for (int j = 0; j < (1 << i); j++) {
                int node_id = (1 << i) - 1 + j;
                std::vector<KappaBitString> x;
                for (int k = 0; k < x_cnt; k++) x.push_back(split_record[node_id].prg[0]());
                GbFlashSplit(0, x, mat_tree[node_id], split_record[node_id]);
            }
        }
    }

    void EvFlashSplit(int node_id, int mat_node, std::vector<std::vector<KappaBitString>> &mat_tree, 
                      std::vector<KappaBitString> &x, std::vector<KappaBitString> &evmaterials, 
                      std::vector<std::vector<KappaBitString>> &evres,
                      std::vector<KappaBitString> &evb, std::vector<KappaBitString> &evdelta) {
 
        // data for next layer
        std::vector<KappaBitString> x_left;
        std::vector<KappaBitString> x_right;

        // b[0] is conditon, acting on b[1] -> end, and also x
        // first, evaluator get (delta ^ delta_{b[0]})
        KappaBitString Hash_B = HashSGC(evb[node_id]);
        KappaBitString delta_deltab = evdelta[node_id];
        // first, finished

        int mid = 0;

        for (int i = 0; i < x.size(); i++) {
            // E's condadd

            KappaBitString condaddd_r = evmaterials[mid++];

            KappaBitString Hash_X = HashSGC(x[i]);
            KappaBitString T = (x[i][0] ? condaddd_r : 0) ^ Hash_X;
            std::pair<KappaBitString, KappaBitString> condadd;
            condadd.first = T ^ (evb[node_id][0] ? 0 : x[i]);            
            condadd.second = T ^ (evb[node_id][0] ? x[i] : 0);

            // E's translate delta
            condadd.first ^= condadd.first[0] ? delta_deltab : 0;
            condadd.second ^= condadd.second[0] ? delta_deltab : 0;

            // E's translate wire
            condadd.first ^= evmaterials[mid++] ^ Hash_B;
            condadd.second ^= evmaterials[mid++] ^ Hash_B;            

            // store to next layer
            x_left.push_back(condadd.first); x_right.push_back(condadd.second);            
        }

        // if no b is translated, i.e., b.size() == 1, this means that we move to the leaf layer
        if (node_id * 2 + 1 >= evb.size()) {
            evres.push_back(x_left);
            evres.push_back(x_right);
            return;
        }
        
        // xor-out (unstack) to next layer ev
        std::vector<KappaBitString> evmaterials_left;
        std::vector<KappaBitString> evmaterials_right;
        for (int i = 0; i < mat_tree[mat_node * 2 + 1].size(); i++) {
            evmaterials_left.push_back(evmaterials[mid] ^ mat_tree[mat_node * 2 + 2][i]);
            evmaterials_right.push_back(evmaterials[mid++] ^ mat_tree[mat_node * 2 + 1][i]);
        }

        // recursively ev
        EvFlashSplit(node_id * 2 + 1, mat_node * 2 + 1, mat_tree, x_left, evmaterials_left, evres, evb, evdelta);     
        EvFlashSplit(node_id * 2 + 2, mat_node * 2 + 2, mat_tree, x_right, evmaterials_right, evres, evb, evdelta);
    }

    void GbFlashSplitMuxWithJunkProcess(int node_id, std::vector<std::vector<KappaBitString>> &junk_mat, 
                                        std::vector<KappaBitString> &x, std::vector<KappaBitString> &gbmaterials,
                                        std::vector<std::vector<std::vector<KappaBitString>>> &junkres,
                                        std::vector<KappaBitString> &delta_tree, std::vector<KappaBitString> &cond_tree,
                                        std::vector<std::vector<KappaBitString>> &junk_b, std::vector<std::vector<KappaBitString>> &junk_delta,
                                        std::vector<SGC::PRG> &prg_tree) {

        // data for next layer
        std::vector<KappaBitString> x_left;
        std::vector<KappaBitString> x_right;

        std::vector<KappaBitString> junk_x_left;
        std::vector<KappaBitString> junk_x_right;


        // b[0] is conditon, acting on b[1] -> end, and also x
        // first, garbler transfer (delta ^ delta_{b[0]}) to E
        KappaBitString Hash_B0 = HashSGC(cond_tree[node_id]);
        KappaBitString Hash_B1 = HashSGC(cond_tree[node_id] ^ delta_tree[node_id]);
        KappaBitString delta_delta0 = delta_tree[node_id] ^ delta_tree[node_id * 2 + 1];
        KappaBitString delta_delta1 = delta_tree[node_id] ^ delta_tree[node_id * 2 + 2];

        // first, finished

        for (int i = 0; i < x.size(); i++) {
            KappaBitString mat;
            auto condadd0 = GbCondAdd(x[i], cond_tree[node_id], delta_tree[node_id], mat); // first is label, second is junk
            auto condadd1 = condadd0;
            condadd0.first = condadd0.first ^ (condadd0.first[0] ? delta_delta0 : 0);
            condadd0.second = condadd0.second ^ (condadd0.second[0] ? delta_delta1 : 0);
            condadd1.first = condadd1.first ^ (condadd1.first[0] ? delta_delta1 : 0);
            condadd1.second = condadd1.second ^ (condadd1.second[0] ? delta_delta0 : 0);

            gbmaterials.push_back(mat);

            KappaBitString x0 = prg_tree[node_id * 2 + 1]();
            KappaBitString x1 = prg_tree[node_id * 2 + 2]();
            x_left.push_back(x0); x_right.push_back(x1);

            KappaBitString transwire_r0 = condadd0.first ^ x0 ^ Hash_B0;
            KappaBitString transwire_r1 = condadd1.first ^ x1 ^ Hash_B1;

            gbmaterials.push_back(transwire_r0); gbmaterials.push_back(transwire_r1);
            condadd0.second ^= transwire_r0 ^ Hash_B1;
            condadd1.second ^= transwire_r1 ^ Hash_B0;         
            junk_x_left.push_back(condadd0.second); junk_x_right.push_back(condadd1.second);   
        }

        // if no b is translated, i.e., b.size() == 1, this means that we move to the leaf layer
        if (node_id * 2 + 1 >= cond_tree.size()) {
            junkres[node_id * 2 + 1].push_back(junk_x_left);
            junkres[node_id * 2 + 2].push_back(junk_x_right);
            return;
        }

        // otherwise, we need to recursively call and stacking them into this layer
        std::vector<KappaBitString> gbmaterials_left;
        std::vector<KappaBitString> gbmaterials_right;

        GbFlashSplitMuxWithJunkProcess(node_id * 2 + 1, junk_mat, x_left, gbmaterials_left, junkres, delta_tree, cond_tree, junk_b, junk_delta, prg_tree);
        GbFlashSplitMuxWithJunkProcess(node_id * 2 + 2, junk_mat, x_right, gbmaterials_right, junkres, delta_tree, cond_tree, junk_b, junk_delta, prg_tree);

        // here is the place to collect junk  
        std::vector<KappaBitString> junkmaterials_left;
        std::vector<KappaBitString> junkmaterials_right;

        for (int i = 0; i < gbmaterials_left.size(); i++) {
            KappaBitString stack_mat = gbmaterials_left[i] ^ gbmaterials_right[i];
            gbmaterials.push_back(stack_mat); // stack together    
            junkmaterials_left.push_back(stack_mat ^ junk_mat[node_id * 2 + 2][i]);
            junkmaterials_right.push_back(stack_mat ^ junk_mat[node_id * 2 + 1][i]);
        }

        // execute E-in-head
        EvFlashSplit(0, node_id * 2 + 1, junk_mat, junk_x_left, junkmaterials_left, junkres[node_id * 2 + 1], junk_b[node_id * 2 + 1], junk_delta[node_id * 2 + 1]);
        EvFlashSplit(0, node_id * 2 + 2, junk_mat, junk_x_right, junkmaterials_right, junkres[node_id * 2 + 2], junk_b[node_id * 2 + 2], junk_delta[node_id * 2 + 2]);
    }

    void ExtractAccessPattern(const std::vector<Fragment> &fragments, std::vector<std::vector<bool>> &access_pattern) {
        access_pattern.resize(fragments.size());        
        for (int i = 0; i < fragments.size(); i++) {
            for (auto &inst : fragments[i]) {
                switch (inst.op) {
                    case OPCODE::LOAD: // LOAD
                        access_pattern[i].push_back(false);
                        break;
                    case OPCODE::STORE: // STORE
                        access_pattern[i].push_back(true);
                        break;
                    default:      
                        break;              
                }
            }
        }
    }

    // Execution of Segments
    void GbExecuteOneSeg(std::vector<SGC::Bit32<Role::Garbler>> &reg, const std::vector<Fragment> &fragments, EpiGRAM<Mode::G> &mem) {

        // TODO: change to correct stuff
        // Now get active from input
        int active = 0;
        std::cin >> active;
        std::vector<SGC::Bit<Role::Garbler>> point_func;
        for (int i = 0; i < 4; i++) point_func.push_back(SGC::Bit<Role::Garbler>(true, SGC::GARBLER, i == active));

        std::vector<KappaBitString> bit_encode;
        std::vector<KappaBitString> good_seed;
        std::vector<KappaBitString> bad_seed;
        GbSeedTree(point_func, bit_encode, good_seed, bad_seed);
        //auto z = x & y;

        std::vector<KappaBitString> trans_x;
        std::vector<KappaBitString> gbmaterials;

        // set trans_x from every single bit in reg
        for (int i = 0; i < SGC::reg_cnt; i++)
            for (int j = 0; j < BIT32::SIZE; j++)
                trans_x.push_back(reg[i].bits[j].wire);

        std::vector<std::vector<KappaBitString>> junk_mat;
        std::vector<SplitReply> split_record;    
        GenerateSplitMatTree(bad_seed, bit_encode.size(), BIT32::SIZE * SGC::reg_cnt, junk_mat, split_record); 

        std::vector<std::vector<std::vector<KappaBitString>>> junkres(7);
        std::vector<std::vector<KappaBitString>> junk_b(7);
        std::vector<std::vector<KappaBitString>> junk_delta(7);

        std::vector<KappaBitString> delta_tree(7);
        std::vector<KappaBitString> cond_tree(3);
        std::vector<SGC::PRG> prg_tree;
        delta_tree[0] = SGC::delta;

        GbSplitMuxWithJunkProcess(0, junk_mat, good_seed[1], good_seed[2], SGC::delta, bit_encode, trans_x, gbmaterials, junkres, delta_tree, cond_tree, junk_b, junk_delta, prg_tree);    

        //std::cout << "Size := " << gbmaterials.size() << endl;
        for (int i = 0; i < gbmaterials.size(); i++) SGC::AddToBuffer(gbmaterials[i]); //SGC::material_buffer.push_back(gbmaterials[i]);

        // TODO: Add Util to Calculate 2048 From Fragments
        int max_length = GetMaxLength(fragments);
        //std::cout << "HIHI MAX LENGTH = " << max_length << std::endl;

        // note: be put before AddToBuffer to ensure correct nonce
        std::vector<std::vector<KappaBitString>> cond_mat_tree;
        GenerateCondMatTree(bad_seed, fragments, max_length, cond_mat_tree);

        gbmaterials.clear();
        std::vector<std::vector<SGC::Bit32<Role::Garbler>>> final_reg;
        std::vector<std::vector<std::vector<SGC::Bit32<Role::Evaluator>>>> junk_output(7);
        std::vector<std::vector<SGC::EVSession>> junk_session(7);
        std::vector<std::vector<SGC::Bit32<Role::Garbler>>> idx_array;
        std::vector<std::vector<SGC::Bit32<Role::Garbler>>> val_array;

        GbStackFragmentsWithJunkProcess(0, good_seed[0], fragments, 3, max_length, final_reg, gbmaterials, cond_mat_tree, 0, 3, junkres, junk_session, idx_array, val_array);

        std::vector<std::vector<bool>> access_pattern;
        ExtractAccessPattern(fragments, access_pattern);

        int max_acc_cnt = 0;
        for (int i = 0; i < idx_array.size(); i++) 
            max_acc_cnt = std::max(max_acc_cnt, (int)idx_array[i].size());
        // flashing!
        std::vector<KappaBitString> flashmaterials; flashmaterials.clear();
        for (int accid = 0; accid < max_acc_cnt; accid++) {
            GbStepProcessJunk(junk_session);
            SGC::Bit32<Role::Garbler> flash_idx;
            SGC::Bit32<Role::Garbler> flash_val;
            GbFlashJoin(accid, junk_session, idx_array, val_array, delta_tree, cond_tree, junk_b, junk_delta, flashmaterials, flash_idx, flash_val);
            SGC::Bit<Role::Garbler> rw_bit;
            GetRW(accid, access_pattern, point_func, rw_bit);
            SGC::Bit32<Role::Garbler> flash_dst = GbACC(mem, SGC::mem_w, SGC::mem_n, flash_idx, flash_val, rw_bit);
            std::vector<std::vector<KappaBitString>> flash_split_mat_tree;
            GenerateFlashSplitMatTree(bit_encode.size(), BIT32::SIZE, flash_split_mat_tree, split_record);
            std::vector<KappaBitString> x;
            for (int i = 0; i < BIT32::SIZE; i++) x.push_back(flash_dst.bits[i].wire);
            std::vector<std::vector<std::vector<KappaBitString>>> flash_junkres(7);
            GbFlashSplitMuxWithJunkProcess(0, flash_split_mat_tree, x, flashmaterials, flash_junkres, delta_tree, cond_tree, junk_b, junk_delta, prg_tree);
            for (int i = 0; i < junk_session.size(); i++) {
                for (int j = 0; j < junk_session[i].size(); j++) junk_session[i][j].Continue(flash_junkres[i][j]);
            }
        }
        GbStepProcessJunk(junk_session);  

        //std::cout << "Size := " << gbmaterials.size() << endl;
        for (int i = 0; i < gbmaterials.size(); i++) SGC::AddToBuffer(gbmaterials[i]);

        //std::cout << "Flash Size := " << flashmaterials.size() << endl;
        for (int i = 0; i < flashmaterials.size(); i++) SGC::AddToBuffer(flashmaterials[i]);

        GbJoinRegistry(final_reg, junk_session, delta_tree, cond_tree, junk_b, junk_delta, reg);

    }

    void EvExecuteOneSeg(std::vector<SGC::Bit32<Role::Evaluator>> &reg, const std::vector<Fragment> &fragments, EpiGRAM<Mode::E> &mem) {
        
        // For one Segments

        std::vector<SGC::Bit<Role::Evaluator>> point_func;
        for (int i = 0; i < 4; i++) point_func.push_back(SGC::Bit<Role::Evaluator>(true, SGC::GARBLER));

        std::vector<KappaBitString> bit_encode;
        std::vector<KappaBitString> seed_tree;
        EvSeedTree(point_func, bit_encode, seed_tree);      

        std::vector<KappaBitString> trans_x;
        std::vector<KappaBitString> gbmaterials;

        for (int i = 0; i < SGC::reg_cnt; i++)
            for (int j = 0; j < BIT32::SIZE; j++)
                trans_x.push_back(reg[i].bits[j].wire);

        std::vector<std::vector<KappaBitString>> mat_tree;
        std::vector<SplitReply> split_record;    
        GenerateSplitMatTree(seed_tree, bit_encode.size(), BIT32::SIZE * SGC::reg_cnt, mat_tree, split_record);

        int evsm_size = GetSplitMuxSize(bit_encode.size(), trans_x.size());
        for (int i = 0; i < evsm_size; i++) gbmaterials.push_back(SGC::get_next_buffer());

        std::vector<std::vector<KappaBitString>> evres;
        std::vector<KappaBitString> evb;
        std::vector<KappaBitString> evdelta;
        EvSplitMux(0, mat_tree, bit_encode, trans_x, gbmaterials, evres, evb, evdelta);


        int max_length = GetMaxLength(fragments);
        //std::cout << "HIHI MAX LENGTH = " << max_length << std::endl;

        std::vector<std::vector<KappaBitString>> cond_mat_tree;
        GenerateCondMatTree(seed_tree, fragments, max_length, cond_mat_tree);

        std::vector<std::vector<bool>> access_pattern;
        ExtractAccessPattern(fragments, access_pattern);    

        gbmaterials.clear();
        auto backup_mid = SGC::material_id;
        for (int i = 0; i < max_length; i++) gbmaterials.push_back(SGC::get_next_buffer());
        SGC::material_id = backup_mid;

        std::vector<SGC::EVSession> ev_session;

        // std::vector<std::vector<SGC::Bit32<Role::Evaluator>>> ev_output;
        EvGenerateSession(0, cond_mat_tree, fragments, 3, gbmaterials, evres, ev_session);
        SGC::material_id += max_length;

        for (int accid = 0; ; accid++) {
        bool check = true;
        for (int i = 0; i < ev_session.size(); i++) {
            if (ev_session[i].Step() == false) check = false;
        }
        if (check) break;
        // otherwise flash join acc and flash split
        SGC::Bit32<Role::Evaluator> flash_idx;
        SGC::Bit32<Role::Evaluator> flash_val;
        EvFlashJoin(accid, ev_session, evb, evdelta, flash_idx, flash_val);
        SGC::Bit<Role::Evaluator> rw_bit;
        GetRW(accid, access_pattern, point_func, rw_bit);
        SGC::Bit32<Role::Evaluator> flash_dst = EvACC(mem, SGC::mem_w, SGC::mem_n, flash_idx, flash_val, rw_bit);
        std::vector<std::vector<KappaBitString>> flash_split_mat_tree;
        GenerateFlashSplitMatTree(bit_encode.size(), BIT32::SIZE, flash_split_mat_tree, split_record);
        std::vector<KappaBitString> x;
        for (int i = 0; i < BIT32::SIZE; i++) x.push_back(flash_dst.bits[i].wire);
        std::vector<KappaBitString> flashsplit_mat;
        for (int i = 0; i < 192; i++) flashsplit_mat.push_back(SGC::get_next_buffer());
        std::vector<std::vector<KappaBitString>> flashsplit_res;
        EvFlashSplit(0, 0, flash_split_mat_tree, x, flashsplit_mat, flashsplit_res, evb, evdelta); 
        for (int i = 0; i < ev_session.size(); i++) ev_session[i].Continue(flashsplit_res[i]);
        }

        // Join Branches into One Registry
        EvJoinRegistry(ev_session, evb, evdelta, reg);

    }

    // Execution of singel BB
    void GbExecuteSingleBB(std::vector<BasicBlock> &CFG, std::vector<uint32_t> &active_bb, 
                           std::vector<SGC::Bit32<Role::Garbler>> &reg, EpiGRAM<Mode::G> &mem) {

        // no need stacking now
        if (active_bb.size() == 1) {
            GbExecuteSingleBBWithoutStacking(reg, mem, CFG[active_bb[0]].fragment);
            return;
        }

        std::vector<Fragment> fragments;
        std::vector<SGC::Bit<Role::Garbler>> point_func;
        for (auto id : active_bb) {
            fragments.push_back(CFG[id].fragment);
            point_func.push_back(reg[REG::PC] == CFG[id].start_addr);
        }

        uint32_t branch_cnt = ceil(log2(active_bb.size()));
        branch_cnt = 1 << branch_cnt;
        while (point_func.size() != branch_cnt) {
            fragments.push_back(Fragment());
            point_func.push_back(SGC::Bit<Role::Garbler>());
        }

        std::vector<KappaBitString> bit_encode;
        std::vector<KappaBitString> good_seed;
        std::vector<KappaBitString> bad_seed;
        GbSeedTree(point_func, bit_encode, good_seed, bad_seed);
        //auto z = x & y;

        std::vector<KappaBitString> trans_x;
        std::vector<KappaBitString> gbmaterials;

        // set trans_x from every single bit in reg
        for (int i = 0; i < SGC::reg_cnt; i++)
            for (int j = 0; j < BIT32::SIZE; j++)
                trans_x.push_back(reg[i].bits[j].wire);

        std::vector<std::vector<KappaBitString>> junk_mat;
        std::vector<SplitReply> split_record;    
        GenerateSplitMatTree(bad_seed, bit_encode.size(), BIT32::SIZE * SGC::reg_cnt, junk_mat, split_record); 

        std::vector<std::vector<std::vector<KappaBitString>>> junkres(2 * branch_cnt - 1);
        std::vector<std::vector<KappaBitString>> junk_b(2 * branch_cnt - 1);
        std::vector<std::vector<KappaBitString>> junk_delta(2 * branch_cnt - 1);

        std::vector<KappaBitString> delta_tree(2 * branch_cnt - 1);
        std::vector<KappaBitString> cond_tree(branch_cnt - 1);
        std::vector<SGC::PRG> prg_tree;
        delta_tree[0] = SGC::delta;

        GbSplitMuxWithJunkProcess(0, junk_mat, good_seed[1], good_seed[2], SGC::delta, bit_encode, trans_x, gbmaterials, junkres, delta_tree, cond_tree, junk_b, junk_delta, prg_tree);    

        //std::cout << "Size := " << gbmaterials.size() << endl;
        for (int i = 0; i < gbmaterials.size(); i++) 
        {
            SGC::AddToBuffer(gbmaterials[i]); //SGC::material_buffer.push_back(gbmaterials[i]);
        }

        // TODO: Add Util to Calculate 2048 From Fragments
        int max_length = GetMaxLength(fragments);
        //std::cout << "HIHI MAX LENGTH = " << max_length << std::endl;

        // note: be put before AddToBuffer to ensure correct nonce
        std::vector<std::vector<KappaBitString>> cond_mat_tree;
        GenerateCondMatTree(bad_seed, fragments, max_length, cond_mat_tree);

        gbmaterials.clear();
        std::vector<std::vector<SGC::Bit32<Role::Garbler>>> final_reg;
        std::vector<std::vector<std::vector<SGC::Bit32<Role::Evaluator>>>> junk_output(2 * branch_cnt - 1);
        std::vector<std::vector<SGC::EVSession>> junk_session(2 * branch_cnt - 1);
        std::vector<std::vector<SGC::Bit32<Role::Garbler>>> idx_array;
        std::vector<std::vector<SGC::Bit32<Role::Garbler>>> val_array;

        GbStackFragmentsWithJunkProcess(0, good_seed[0], fragments, branch_cnt - 1, max_length, final_reg, gbmaterials, cond_mat_tree, 0, branch_cnt - 1, junkres, junk_session, idx_array, val_array);

        std::vector<std::vector<bool>> access_pattern;
        ExtractAccessPattern(fragments, access_pattern);

        int max_acc_cnt = 0;
        for (int i = 0; i < idx_array.size(); i++) 
            max_acc_cnt = std::max(max_acc_cnt, (int)idx_array[i].size());
            // flashing!
        std::vector<KappaBitString> flashmaterials; flashmaterials.clear();

        //std::cout << "Size := " << gbmaterials.size() << endl;
        for (int i = 0; i < gbmaterials.size(); i++) SGC::AddToBuffer(gbmaterials[i]);

        for (int accid = 0; accid < max_acc_cnt; accid++) {
            GbStepProcessJunk(junk_session);
            SGC::Bit32<Role::Garbler> flash_idx;
            SGC::Bit32<Role::Garbler> flash_val;
            GbFlashJoin(accid, junk_session, idx_array, val_array, delta_tree, cond_tree, junk_b, junk_delta, flashmaterials, flash_idx, flash_val);
            SGC::Bit<Role::Garbler> rw_bit;
            GetRW(accid, access_pattern, point_func, rw_bit);
            SGC::Bit32<Role::Garbler> flash_dst = GbACC(mem, SGC::mem_w, SGC::mem_n, flash_idx, flash_val, rw_bit);
            std::vector<std::vector<KappaBitString>> flash_split_mat_tree;
            GenerateFlashSplitMatTree(bit_encode.size(), BIT32::SIZE, flash_split_mat_tree, split_record);
            std::vector<KappaBitString> x;
            for (int i = 0; i < BIT32::SIZE; i++) x.push_back(flash_dst.bits[i].wire);
            std::vector<std::vector<std::vector<KappaBitString>>> flash_junkres(2 * branch_cnt - 1);
            GbFlashSplitMuxWithJunkProcess(0, flash_split_mat_tree, x, flashmaterials, flash_junkres, delta_tree, cond_tree, junk_b, junk_delta, prg_tree);
            for (int i = 0; i < junk_session.size(); i++) {
                for (int j = 0; j < junk_session[i].size(); j++) junk_session[i][j].Continue(flash_junkres[i][j]);
            }
        }
        GbStepProcessJunk(junk_session);  

        //std::cout << "Flash Size := " << flashmaterials.size() << endl;
        for (int i = 0; i < flashmaterials.size(); i++) SGC::AddToBuffer(flashmaterials[i]);

        GbJoinRegistry(final_reg, junk_session, delta_tree, cond_tree, junk_b, junk_delta, reg);
    }

    void EvExecuteSingleBB(std::vector<BasicBlock> &CFG, std::vector<uint32_t> &active_bb, 
                           std::vector<SGC::Bit32<Role::Evaluator>> &reg, EpiGRAM<Mode::E> &mem) {
        
        // no need stacking now
        if (active_bb.size() == 1) {
            EvExecuteSingleBBWithoutStacking(reg, mem, CFG[active_bb[0]].fragment);            
            return;
        }

        std::vector<Fragment> fragments;
        std::vector<SGC::Bit<Role::Evaluator>> point_func;
        for (auto id : active_bb) {
            fragments.push_back(CFG[id].fragment);
            point_func.push_back(reg[REG::PC] == CFG[id].start_addr);
        }

        uint32_t branch_cnt = ceil(log2(active_bb.size()));
        branch_cnt = 1 << branch_cnt;
        while (point_func.size() != branch_cnt) {
            fragments.push_back(Fragment());
            point_func.push_back(SGC::Bit<Role::Evaluator>());
        }

        std::vector<KappaBitString> bit_encode;
        std::vector<KappaBitString> seed_tree;
        EvSeedTree(point_func, bit_encode, seed_tree);      

        std::vector<KappaBitString> trans_x;
        std::vector<KappaBitString> gbmaterials;

        for (int i = 0; i < SGC::reg_cnt; i++)
            for (int j = 0; j < BIT32::SIZE; j++)
                trans_x.push_back(reg[i].bits[j].wire);

        std::vector<std::vector<KappaBitString>> mat_tree;
        std::vector<SplitReply> split_record;    
        GenerateSplitMatTree(seed_tree, bit_encode.size(), BIT32::SIZE * SGC::reg_cnt, mat_tree, split_record);

        int evsm_size = GetSplitMuxSize(bit_encode.size(), trans_x.size());
        for (int i = 0; i < evsm_size; i++) {
            gbmaterials.push_back(SGC::get_next_buffer());
        }

        std::vector<std::vector<KappaBitString>> evres;
        std::vector<KappaBitString> evb;
        std::vector<KappaBitString> evdelta;
        EvSplitMux(0, mat_tree, bit_encode, trans_x, gbmaterials, evres, evb, evdelta);


        int max_length = GetMaxLength(fragments);
        //std::cout << "HIHI MAX LENGTH = " << max_length << std::endl;

        std::vector<std::vector<KappaBitString>> cond_mat_tree;
        GenerateCondMatTree(seed_tree, fragments, max_length, cond_mat_tree);

        std::vector<std::vector<bool>> access_pattern;
        ExtractAccessPattern(fragments, access_pattern);    

        gbmaterials.clear();
        auto backup_mid = SGC::material_id;
        for (int i = 0; i < max_length; i++) gbmaterials.push_back(SGC::get_next_buffer());
        SGC::material_id = backup_mid;

        std::vector<SGC::EVSession> ev_session;

        // std::vector<std::vector<SGC::Bit32<Role::Evaluator>>> ev_output;
        EvGenerateSession(0, cond_mat_tree, fragments, branch_cnt - 1, gbmaterials, evres, ev_session);
        SGC::material_id += max_length;

        for (int accid = 0; ; accid++) {
            bool check = true;
            for (int i = 0; i < ev_session.size(); i++) {
                if (ev_session[i].Step() == false) check = false;
            }
            if (check) break;
            // otherwise flash join acc and flash split
            SGC::Bit32<Role::Evaluator> flash_idx;
            SGC::Bit32<Role::Evaluator> flash_val;
            EvFlashJoin(accid, ev_session, evb, evdelta, flash_idx, flash_val);
            SGC::Bit<Role::Evaluator> rw_bit;
            GetRW(accid, access_pattern, point_func, rw_bit);
            SGC::Bit32<Role::Evaluator> flash_dst = EvACC(mem, SGC::mem_w, SGC::mem_n, flash_idx, flash_val, rw_bit);
            std::vector<std::vector<KappaBitString>> flash_split_mat_tree;
            GenerateFlashSplitMatTree(bit_encode.size(), BIT32::SIZE, flash_split_mat_tree, split_record);
            std::vector<KappaBitString> x;
            for (int i = 0; i < BIT32::SIZE; i++) x.push_back(flash_dst.bits[i].wire);
            std::vector<KappaBitString> flashsplit_mat;
            int flash_mat_cnt = BIT32::SIZE * 3 * bit_encode.size();
            for (int i = 0; i < flash_mat_cnt; i++) flashsplit_mat.push_back(SGC::get_next_buffer());
            std::vector<std::vector<KappaBitString>> flashsplit_res;
            EvFlashSplit(0, 0, flash_split_mat_tree, x, flashsplit_mat, flashsplit_res, evb, evdelta); 
            for (int i = 0; i < ev_session.size(); i++) ev_session[i].Continue(flashsplit_res[i]);
        }

        // Join Branches into One Registry
        EvJoinRegistry(ev_session, evb, evdelta, reg);
    } 


};
