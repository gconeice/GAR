#ifndef SGC_BASICBLOCK_H__
#define SGC_BASICBLOCK_H__

#include <vector>
#include <string>

namespace SGC {

    enum OPCODE : std::uint8_t {
        ADD,
        ADDI,
        SUB,
        SUBI,
        MUL,
        LOAD,
        STORE,
        IMM,
        SWAP,
        COPY,
        CCOPY,
        CMP,
        EQ,
        EQI,
        XOR,
        RS1,
        NOOP,
        // below are some special (internal) OPs
        AND0, // AND with reg.bits[0]
        AND1, // AND with reg.bits[1]
        ANDN0, // AND with ~reg.bits[0]
        ANDN1, // AND with ~reg.bits[1]
    };

    enum REG : std::uint8_t {
        GR0,
        GR1,
        GR2,
        GR3,
        GR4,
        GR5,
        GR6,
        GR7,
        GR8,
        PC,
        // below are some special (internal) REGs
        SP0,
        SP1,    
        SP2,    
    };

    struct Inst {
        OPCODE op;
        REG dst;
        REG src1, src2;   
        uint32_t imm;
    };    

    struct BasicBlock {
        uint32_t start_addr;
        uint32_t acc_cnt;
        std::vector<Inst> fragment;
        std::vector<uint32_t> next_BB;
        std::string branch_BB;
        void clear() {
            fragment.clear();
            next_BB.clear();
            start_addr = 0;
            branch_BB = "";
            acc_cnt = 0;
        }
    };

    using Fragment = std::vector<Inst>;

};

#endif