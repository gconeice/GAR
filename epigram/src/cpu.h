#ifndef CPU_H__
#define CPU_H__


#include <epigram.h>
#include <word.h>


constexpr std::size_t registry_size = 8;


enum OP : std::uint8_t {
  ADD,
  SUB,
  BNZ,
  LD,
  STR,
  OUT,
  IMM,
  HLT,
};


struct Instr {
  OP op;
  std::uint8_t r0;
  std::uint8_t r1;
  std::uint8_t r2;
  std::uint16_t imm;
};


struct Config {
  std::size_t main_memory_size;
  std::size_t cycles;
  std::size_t linear_scan_preference;
};


std::istream& operator>>(std::istream&, Instr&);
std::ostream& operator<<(std::ostream&, const Instr&);


template <Mode m>
struct CPU {
  std::size_t cycles_left;
  Garbled::Word<m> pc;
  std::vector<Garbled::Bit<m>> reg;
  EpiGRAM<m> mem;
  EpiGRAM<m> program;

  void step();
  void run();

  static CPU make(const Config&, std::span<const Instr>);
};


#endif
