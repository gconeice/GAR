// Project Headers
#include "Bit.h"
#include "Bit32.h"
#include "Const.h"
#include "cryptographic_primitives/Prg.h"
#include "Role.h"
#include "AsParse.h"
#include "SGCUtils.h"

// EpiGRAM

#include <lazypermutation.h>
#include <permute.h>
#include <util.h>
#include <epigram.h>
#include <arithmetic.h>
#include <cpu.h>

// Standard Headers
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <fstream>
#include <vector>

// Submodule Headers
#include <emp-ot/emp-ot.h>
#include <emp-tool/emp-tool.h> // For NetIO

// time testing
#include <chrono>   

using namespace std;
using namespace SGC;
using namespace chrono;

std::chrono::time_point<std::chrono::system_clock> start1, start2, end1;

// TODO Set the following global variables before each run

// Based on network settings, compute bandwidth_delay_product
size_t bandwidth = 1073741824; // in bytes, e.g. 1GB = 1073741824
double latency = 0.1; // in seconds, e.g. 100ms = 0.1s
size_t adjusted_bandwidth_delay_product = bandwidth * latency - 10000; // Choose some small buffer as this is the theoretical maximum

template <Role R>
void and_gate() {

}

//template void transferLargeVector<int>;

int main(int argc, char ** argv) {
  std::cout << "bandwidth delay product " << adjusted_bandwidth_delay_product << std::endl;

  // Form as many connections as you can to maximize throughput
//  std::vector<size_t> num_connections (material_sizes.size());
//  for (size_t c = 0; c < material_sizes.size(); c++) {
//    num_connections[c] = ceil(material_sizes[c] / adjusted_bandwidth_delay_product);
//  }

  // std::cout << ceil(log2(64)) << std::endl;

  Config cfg { 256, 256, 64 };
  Label zero_label;
  EPCCRH key(zero_label);

  ifstream fin("prog.s");
  std::vector<std::string> program;
  OpenAs(fin, program);
  fin.close();
  std::vector<BasicBlock> CFG;
  GenerateCFG(program, CFG);

  std::vector<std::vector<uint32_t>> BBPath;
  uint32_t giant_step = stoi(argv[2]);
  mem_n = stoi(argv[3]);
  //cin >> giant_step;
  uint32_t max_acc_cnt = GenerateBBPath(giant_step, CFG, BBPath);
  std::cout << "MAXACC = " << max_acc_cnt << std::endl;

  /*
  for (int i = 0; i < giant_step; i++) {
    cout << i << ": ";
    for (auto x : BBPath[i]) cout << x << ' ';
    cout << endl;
  }
  */

  if (string(argv[1]) == "Cleartext") {
    SGC::delta = SGC::prg();
    // SGC::and_gate_id = 0;
    auto x = SGC::Bit<Role::Cleartext>(1);
    auto y = SGC::Bit<Role::Cleartext>(0);
    auto z = x & y;
  }
  if (string(argv[1]) == "Speculate") {
    
    auto x = SGC::Bit<Role::Speculate>(SGC::GARBLER);
    auto y = SGC::Bit<Role::Speculate>(SGC::EVALUATOR);
    auto z = x & y;
    z.Reveal();
    std::cerr << "Number ands: " << SGC::circuit_desc.nAnd << std::endl;
    std::cerr << "Number inps: " << SGC::circuit_desc.nInp << std::endl;
    std::cerr << "Number outs: " << SGC::circuit_desc.nOut << std::endl;
    
  }
  if (string(argv[1]) == "Alice") {

    set_delta(key);
    // Just comment out to see merge did not break anything
    //vector<string> as_program; as_program.clear();
    //ifstream fin(argv[2]);
    //OpenAs(fin, as_program);

    std::cerr << "Running Alice..." << std::endl;

    // Establish connection with bob
    unsigned int initial_port = 8880;
    emp::NetIO *io = new emp::NetIO(nullptr, initial_port);
    start1 = std::chrono::system_clock::now();
    SGC::setup_garbler(io);

    int bincnt, aincnt, pincnt;
    fin.open("alice.in");

    fin >> bincnt >> aincnt >> pincnt;
    int init_cnt = bincnt + aincnt + pincnt;

    // testing for GRAM
    initialize<Mode::S>(key);
    auto arr_s = EpiGRAM<Mode::S>::make(SGC::mem_w, SGC::mem_n);
    const auto ix_s = constant_nat<Mode::S>(log2(SGC::mem_n), 0);
    const auto val_s = constant_nat<Mode::S>(SGC::mem_w, 0);
    for (int i = 0; i < init_cnt; i++)
      arr_s.write(ix_s, val_s);
    
    /*
    const auto ix_s_0 = constant_nat<Mode::S>(log2(SGC::mem_n), 100 % SGC::mem_n);
    const auto val_s_0 = constant_nat<Mode::S>(SGC::mem_w, 200);
    const auto ix_s_1 = constant_nat<Mode::S>(log2(SGC::mem_n), 300 % SGC::mem_n);
    const auto val_s_1 = constant_nat<Mode::S>(SGC::mem_w, 400); 
    const auto ix_s_2 = constant_nat<Mode::S>(log2(SGC::mem_n), 100 % SGC::mem_n);
    const auto val_s_2 = constant_nat<Mode::S>(SGC::mem_w, 200);
    arr_s.write(ix_s_0, val_s_0);
    arr_s.write(ix_s_1, val_s_1);
    arr_s.write(ix_s_2, val_s_2);
    */

    // Generate Mode::S Data (necessary!)
    // last arguments is the access #
    PrepareAccess(arr_s, SGC::mem_w, SGC::mem_n, max_acc_cnt);

    std::cout << scost << std::endl;

    initialize<Mode::G>(key);
    auto arr_g = EpiGRAM<Mode::G>::make(SGC::mem_w, SGC::mem_n);

    for (int i = 0; i < bincnt; i++) {
      std::size_t ix;
      fin >> ix;
      auto val = SGC::Bit32<Role::Garbler>(SGC::EVALUATOR);
      const auto ix_g = constant_nat<Mode::G>(log2(SGC::mem_n), ix % SGC::mem_n);
      std::vector<Garbled::Bit<Mode::G>> val_g(mem_w);
      for (int j = 0; j < mem_w; j++) val_g[j] = Garbled::Bit<Mode::G>(Label(val.bits[j].wire));
      arr_g.write(ix_g, val_g);
    }

    for (int i = 0; i < aincnt; i++) {
      std::size_t ix;
      std::size_t _val;
      fin >> ix >> _val;
      auto val = SGC::Bit32<Role::Garbler>(SGC::GARBLER, _val);
      const auto ix_g = constant_nat<Mode::G>(log2(SGC::mem_n), ix % SGC::mem_n);
      std::vector<Garbled::Bit<Mode::G>> val_g(mem_w);
      for (int j = 0; j < mem_w; j++) val_g[j] = Garbled::Bit<Mode::G>(Label(val.bits[j].wire));
      arr_g.write(ix_g, val_g);
    }

    for (int i = 0; i < pincnt; i++) {
      std::size_t ix;
      std::size_t val;
      fin >> ix >> val;
      const auto ix_g = constant_nat<Mode::G>(log2(SGC::mem_n), ix % SGC::mem_n);
      const auto val_g = constant_nat<Mode::G>(SGC::mem_w, val);
      arr_g.write(ix_g, val_g);
    }

    fin.close();

    /*
    const auto ix_g_0 = constant_nat<Mode::G>(log2(SGC::mem_n), 1 % SGC::mem_n);
    const auto val_g_0 = constant_nat<Mode::G>(SGC::mem_w, 3);
    const auto ix_g_1 = constant_nat<Mode::G>(log2(SGC::mem_n), 2 % SGC::mem_n);
    const auto val_g_1 = constant_nat<Mode::G>(SGC::mem_w, 13);
    const auto ix_g_2 = constant_nat<Mode::G>(log2(SGC::mem_n), 0 % SGC::mem_n);
    const auto val_g_2 = constant_nat<Mode::G>(SGC::mem_w, 2);
    arr_g.write(ix_g_0, val_g_0);
    arr_g.write(ix_g_1, val_g_1);
    arr_g.write(ix_g_2, val_g_2);
    */

    //auto GR0 = SGC::Bit32<Role::Garbler>();
    //auto x = SGC::Bit<Role::Garbler>(true, SGC::EVALUATOR);
    std::vector<SGC::Bit32<Role::Garbler>> reg(reg_cnt);

    /*
    
    SGC::Fragment br0;
    br0.push_back(SGC::Inst{OPCODE::MUL, REG::GR0, REG::GR0, REG::GR1, 0});   
    br0.push_back(SGC::Inst{OPCODE::LOAD, REG::GR0, REG::GR0, REG::GR0, 0});   
    br0.push_back(SGC::Inst{OPCODE::COPY, REG::GR0, REG::GR1, REG::GR1, 0});
    SGC::Fragment br1;
    br1.push_back(SGC::Inst{OPCODE::ADD, REG::GR0, REG::GR0, REG::GR1, 0});
    br1.push_back(SGC::Inst{OPCODE::IMM, REG::GR1, REG::GR1, REG::GR1, 3});
    br1.push_back(SGC::Inst{OPCODE::EQ, REG::GR0, REG::GR1, REG::GR0, 0});
    br1.push_back(SGC::Inst{OPCODE::NOOP, REG::GR0, REG::GR0, REG::GR0, 0});   
    SGC::Fragment br2;
    br2.push_back(SGC::Inst{OPCODE::ADD, REG::GR0, REG::GR0, REG::GR1, 0});
    br2.push_back(SGC::Inst{OPCODE::LOAD, REG::GR0, REG::GR0, REG::GR0, 0});
    br2.push_back(SGC::Inst{OPCODE::STORE, REG::GR0, REG::GR0, REG::GR0, 0});
    br2.push_back(SGC::Inst{OPCODE::XOR, REG::GR0, REG::GR0, REG::GR1, 0});
    br2.push_back(SGC::Inst{OPCODE::ANDN0, REG::GR0, REG::GR0, REG::GR1, 0});
    SGC::Fragment br3;
    br3.push_back(SGC::Inst{OPCODE::ADD, REG::GR0, REG::GR0, REG::GR1, 0});
    br3.push_back(SGC::Inst{OPCODE::STORE, REG::GR0, REG::GR0, REG::GR0, 0});
    br3.push_back(SGC::Inst{OPCODE::LOAD, REG::GR0, REG::GR0, REG::GR0, 0});
    br3.push_back(SGC::Inst{OPCODE::IMM, REG::GR0, REG::GR0, REG::GR0, 12345});
    std::vector<Fragment> fragments;
    fragments.push_back(br0);
    fragments.push_back(br1);
    fragments.push_back(br2);
    fragments.push_back(br3);
    */

/*     GbExecuteOneSeg(reg, fragments, arr_g);
    GbExecuteOneSeg(reg, fragments, arr_g);
 */

    for (int i = 0; i < giant_step; i++) {
      // for testing active inst cnts
      // reg[REG::PC].Reveal();         
      // std::cout << "GBSTEP " << i << " : "  << SGC::material_id << std::endl;
      GbExecuteSingleBB(CFG, BBPath[i], reg, arr_g);
    }

    reg[REG::GR0].Reveal();
    reg[REG::GR1].Reveal();
    reg[REG::GR2].Reveal();
    reg[REG::GR4].Reveal();
    reg[REG::PC].Reveal();     


    start2 = std::chrono::system_clock::now();
    SGC::initiate_2pc_evaluation(io, 30000, adjusted_bandwidth_delay_product);
    end1 = std::chrono::system_clock::now();

    std::chrono::duration<double> elapsed_seconds1 = end1 - start2;
    std::chrono::duration<double> elapsed_seconds2 = start2 - start1;
  
    std::cout << "materials transferred time: " << elapsed_seconds1.count() << "s\n"
              << "garbling time: " << elapsed_seconds2.count() << "s\n";
    std::cout << "and gates without sgc for branching: " << and_gate_without_sgc << std::endl;
    std::cout << "and gates with sgc for branching: " << and_gate_with_sgc << std::endl;
  }
  if (string(argv[1]) == "Bob") {

    cout << "Running Bob... " << std::endl;

    // Establish connection with Alice
    const char *address = "127.0.0.1";
    unsigned int initial_port = 8880;
    emp::NetIO *io = new emp::NetIO(address, initial_port);
    start1 = std::chrono::system_clock::now();

    int bincnt, aincnt, pincnt;
    // evaluator inputs
    fin.open("bob.in");

    fin >> bincnt >> aincnt >> pincnt;
    std::vector<std::vector<Garbled::Bit<Mode::E>>> b_ix(bincnt);

    bool *evaluator_inputs = new bool [bincnt * mem_w];
    for (int i = 0; i < bincnt; i++) {
      std::size_t ix;
      std::size_t val;
      fin >> ix >> val;
      b_ix[i] = constant_nat<Mode::E>(log2(SGC::mem_n), ix % SGC::mem_n);
      for (int j = 0; j < mem_w; j++) {
        evaluator_inputs[ i * mem_w + j ] = val & 1;
        val >>= 1;
      }
    }

    // evaluator holds input on only a single wire
    // bool evaluator_inputs [1] = { true };
    //bool evaluator_inputs[0];
    //uint32_t ein = 1;
    //for (int i = 0; i < 0; i++, ein >>= 1) evaluator_inputs[i] = ein & 1;
    // Set up connection and receive material, inputs, etc
    SGC::setup_evaluator(evaluator_inputs, io, address, 30000, adjusted_bandwidth_delay_product);

    
    // initializing GRAM

    initialize<Mode::E>(key);
    auto arr_e = EpiGRAM<Mode::E>::make(SGC::mem_w, SGC::mem_n);
    //fin.open("mem.init");

    for (int i = 0; i < bincnt; i++) {
      auto val = SGC::Bit32<Role::Evaluator>(SGC::EVALUATOR);
      std::vector<Garbled::Bit<Mode::E>> val_e(mem_w);
      for (int j = 0; j < mem_w; j++) val_e[j] = Garbled::Bit<Mode::E>(Label(val.bits[j].wire));
      arr_e.write(b_ix[i], val_e);
    }

    for (int i = 0; i < aincnt; i++) {
      std::size_t ix;
      fin >> ix;
      auto val = SGC::Bit32<Role::Evaluator>(SGC::GARBLER);
      const auto ix_e = constant_nat<Mode::E>(log2(SGC::mem_n), ix % SGC::mem_n);
      std::vector<Garbled::Bit<Mode::E>> val_e(mem_w);
      for (int j = 0; j < mem_w; j++) val_e[j] = Garbled::Bit<Mode::E>(Label(val.bits[j].wire));
      arr_e.write(ix_e, val_e);
    }

    for (int i = 0; i < pincnt; i++) {
      std::size_t ix;
      std::size_t val;
      fin >> ix >> val;
      const auto ix_e = constant_nat<Mode::E>(log2(SGC::mem_n), ix % SGC::mem_n);
      const auto val_e = constant_nat<Mode::E>(SGC::mem_w, val);
      arr_e.write(ix_e, val_e);
    }

    fin.close();
    /*
    const auto ix_e_0 = constant_nat<Mode::E>(log2(SGC::mem_n), 1 % SGC::mem_n);
    const auto val_e_0 = constant_nat<Mode::E>(SGC::mem_w, 3);
    const auto ix_e_1 = constant_nat<Mode::E>(log2(SGC::mem_n), 2 % SGC::mem_n);
    const auto val_e_1 = constant_nat<Mode::E>(SGC::mem_w, 13);
    const auto ix_e_2 = constant_nat<Mode::E>(log2(SGC::mem_n), 0 % SGC::mem_n);
    const auto val_e_2 = constant_nat<Mode::E>(SGC::mem_w, 2);
    arr_e.write(ix_e_0, val_e_0);
    arr_e.write(ix_e_1, val_e_1);  
    arr_e.write(ix_e_2, val_e_2);
    */

    std::vector<SGC::Bit32<Role::Evaluator>> reg(reg_cnt);

    /*
    mem[0] = 2
    mem[1] = 3
    mem[2] = 13
    */

    /*
    LOAD x y: reg[x] = mem[reg[y]]
    STORE x y: mem[reg[x]] = reg[y]
    BR0: MUL 0 0 1, LOAD 0 0 (reg[0] = mem[reg[0]])
    BR1: ADD 0 0 1
    BR2: ADD 0 0 1, LOAD 0 0, STORE 0 0
    BR3: ADD 0 0 1, STORE 0 0, LOAD 0 0, IMM 0 12345
    OUTPUT reg[0] finally
    BR0: 3
    BR1: 2
    BR2: 13
    BR3: 123456
    */
    /*
    SGC::Fragment br0;
    br0.push_back(SGC::Inst{OPCODE::MUL, REG::GR0, REG::GR0, REG::GR1, 0});   
    br0.push_back(SGC::Inst{OPCODE::LOAD, REG::GR0, REG::GR0, REG::GR0, 0});   
    br0.push_back(SGC::Inst{OPCODE::COPY, REG::GR0, REG::GR1, REG::GR1, 0});
    SGC::Fragment br1;
    br1.push_back(SGC::Inst{OPCODE::ADD, REG::GR0, REG::GR0, REG::GR1, 0});
    br1.push_back(SGC::Inst{OPCODE::IMM, REG::GR1, REG::GR1, REG::GR1, 3});
    br1.push_back(SGC::Inst{OPCODE::EQ, REG::GR0, REG::GR1, REG::GR0, 0});
    br1.push_back(SGC::Inst{OPCODE::NOOP, REG::GR0, REG::GR0, REG::GR0, 0});   
    SGC::Fragment br2;
    br2.push_back(SGC::Inst{OPCODE::ADD, REG::GR0, REG::GR0, REG::GR1, 0});
    br2.push_back(SGC::Inst{OPCODE::LOAD, REG::GR0, REG::GR0, REG::GR0, 0});
    br2.push_back(SGC::Inst{OPCODE::STORE, REG::GR0, REG::GR0, REG::GR0, 0});
    br2.push_back(SGC::Inst{OPCODE::XOR, REG::GR0, REG::GR0, REG::GR1, 0});
    br2.push_back(SGC::Inst{OPCODE::ANDN0, REG::GR0, REG::GR0, REG::GR1, 0});
    SGC::Fragment br3;
    br3.push_back(SGC::Inst{OPCODE::ADD, REG::GR0, REG::GR0, REG::GR1, 0});
    br3.push_back(SGC::Inst{OPCODE::STORE, REG::GR0, REG::GR0, REG::GR0, 0});
    br3.push_back(SGC::Inst{OPCODE::LOAD, REG::GR0, REG::GR0, REG::GR0, 0});
    br3.push_back(SGC::Inst{OPCODE::IMM, REG::GR0, REG::GR0, REG::GR0, 12345});
    std::vector<Fragment> fragments;
    fragments.push_back(br0);
    fragments.push_back(br1);
    fragments.push_back(br2);
    fragments.push_back(br3);
    */

    /*    
    EvExecuteOneSeg(reg, fragments, arr_e);
    EvExecuteOneSeg(reg, fragments, arr_e);
    */ 
    for (int i = 0; i < giant_step; i++) {
      // for testing active inst cnts
      // reg[REG::PC].Reveal();      
      //std::cout << "EVSTEP " << i << " : "  << SGC::material_id << std::endl;
      EvExecuteSingleBB(CFG, BBPath[i], reg, arr_e);

    }

    reg[REG::GR0].Reveal();
    reg[REG::GR1].Reveal();
    reg[REG::GR2].Reveal();
    reg[REG::GR4].Reveal();
    reg[REG::PC].Reveal();

    // for testing active inst cnts
    /*
    for (auto &BB : CFG) {
      std::cout << BB.start_addr << ' ' << BB.fragment.size() << ' ' << std::endl;
    }
    */

    end1 = std::chrono::system_clock::now();

    std::chrono::duration<double> elapsed_seconds1 = end1 - start1;
  
    std::cout << "e2e time: " << elapsed_seconds1.count() << "s\n";

    std::cerr << "Result:" << std::endl;
    int cccc = 0;
    size_t dddd = 0;
    for (const auto &r : SGC::result_bits) {
      dddd += r*(1 << cccc);
      cccc++;
      if (cccc == 32) {
        std::cerr << dddd << endl;
        dddd = 0;
        cccc = 0;
      }
    }
    std::cerr << std::endl;


    
  }

  /*
  // PRG Example
  PRG prg_garbler; // initialize with random seed (garbler)
  std::cout << "PRG Garbler seed: " << prg_garbler.get_seed() << std::endl;
  PRG prg_evaluator (*prg_garbler.get_seed()); // initialize with a particular seed (evaluator)
  std::cout << "PRG Evaluator seed: " << prg_garbler.get_seed() << std::endl;
  // Verify both prgs produce the same strings
  for (size_t idx = 0; idx < 10; idx++) {
    std::bitset<128> block_gb = prg_garbler();
    std::bitset<128> block_ev = prg_evaluator();
    std::cerr << block_gb << std::endl;
    std::cerr << block_ev << std::endl;
    assert(block_gb == block_ev);
  }
  */
  return 0;
}
