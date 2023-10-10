#include <cpu.h>
#include <word.h>
#include <linearscan.h>
#include <string>


template <Mode m>
void encode_instr(const Instr& i, std::span<Garbled::Bit<m>> buff) {
  ginput_nat(static_cast<std::size_t>(i.op), buff.subspan(0, 5));
  if (i.op != IMM) {
    ginput_nat(static_cast<std::size_t>(i.r0), buff.subspan(7, 3));
    ginput_nat(static_cast<std::size_t>(i.r1), buff.subspan(10, 3));
    ginput_nat(static_cast<std::size_t>(i.r2), buff.subspan(13, 3));
  } else {
    ginput_nat(static_cast<std::size_t>(i.imm), buff.subspan(5, 11));
  }
}


template <Mode m>
std::vector<Garbled::Bit<m>> encode_program(std::span<const Instr> p) {
  std::vector<Garbled::Bit<m>> out(p.size() * 16);
  for (std::size_t i = 0; i < p.size(); ++i) {
    encode_instr(p[i], to_span(out, i*16, 16));
  }
  return out;
}



template <Mode m>
CPU<m> CPU<m>::make(const Config& cfg, std::span<const Instr> p) {
  return {
    .cycles_left = cfg.cycles,
    .pc = Garbled::Word<m>::constant(0),
    .reg = std::vector<Garbled::Bit<m>>(word_size * registry_size),
    .mem = EpiGRAM<m>::make(word_size, cfg.main_memory_size, cfg.linear_scan_preference),
    .program = EpiGRAM<m>::make(16, encode_program<m>(p), cfg.linear_scan_preference),
  };
}

template <Mode m>
void CPU<m>::step() {
  const auto w = word_size;
  auto instr = program.read(pc.to_span(0, log2(program.size())));

  const std::span<const Garbled::Bit<m>> opcode = to_span(instr, 0, 5);
  auto r0 = to_span(instr, 7, 3);
  const std::span<const Garbled::Bit<m>> r1 = to_span(instr, 10, 3);
  const std::span<const Garbled::Bit<m>> r2 = to_span(instr, 13, 3);
  const auto imm_slice = to_span(instr, 5, 11);
  Garbled::Word<m> imm;
  std::copy(imm_slice.begin(), imm_slice.end(), imm.to_span().begin());

  const Garbled::Word<m> arg0 = read_scan(w, r1, std::span<const Garbled::Bit<m>> { reg });
  const Garbled::Word<m> arg1 = read_scan(w, r2, std::span<const Garbled::Bit<m>> { reg });

  const auto opmap = one_hot(opcode);

  const Garbled::Word<m> mval =
    mem.access([&](auto buff) {
      // store only on STR instruction
      buff ^= arg1.to_span();
      buff &= ~opmap[STR];
      buff ^= arg1.to_span();
    }, arg0.to_span(0, log2(mem.size())));


  Garbled::Word<m> val;

  val ^= (arg0 + arg1) * opmap[ADD];
  val ^= (arg0 - arg1) * opmap[SUB];
  val ^= mval * opmap[LD];
  val ^= imm * opmap[IMM];
  val ^= arg0 * opmap[OUT];
  val ^= arg0 * opmap[STR];

  // if the opcode is IMM, set r0 to be register zero
  r0 &= ~opmap[IMM];

  std::span<const Garbled::Bit<m>> v = val;
  write_scan(v, std::span<const Garbled::Bit<m>> { r0 }, std::span { reg });

  { // OUTPUT
    // TODO strengthen this functionality with a stack of outputs
    bool is_out = dec(opmap[OUT]);
    std::size_t out = (arg0 * opmap[OUT]).dec();

    if constexpr (m == Mode::E) {
      if (is_out) {
        std::cout << out << '\n';
      }
    }
  }

  const auto is_jump = opmap[BNZ] & (arg0 != Garbled::Word<m>::constant(0));
  const auto is_halt = opmap[HLT];

  Garbled::Word<m> new_pc;
  new_pc ^= (pc + Garbled::Word<m>::constant(1)) * ~(is_jump ^ is_halt);
  new_pc ^= arg1 * is_jump;
  new_pc ^= pc * is_halt;
  pc = new_pc;
}

template <Mode m>
void CPU<m>::run() {
  while (cycles_left != 0) {
    step();
    --cycles_left;
  }
}

template struct CPU<Mode::G>;
template struct CPU<Mode::E>;
template struct CPU<Mode::S>;


std::istream& operator>>(std::istream& is, Instr& i) {

  i.r0 = 0;
  i.r1 = 0;
  i.r2 = 0;

  std::string code;
  is >> code;
  if (code == "ADD") { i.op = ADD; }
  if (code == "SUB") { i.op = SUB; }
  if (code == "BNZ") { i.op = BNZ; }
  if (code == "LD")  { i.op = LD; }
  if (code == "STR") { i.op = STR; }
  if (code == "OUT") { i.op = OUT; }
  if (code == "IMM") { i.op = IMM; }
  if (code == "HLT") { i.op = HLT; }

  if (i.op == IMM) {
    is >> i.imm;
  } else if (i.op == ADD || i.op == SUB) {
    is >> i.r0;
    is >> i.r1;
    is >> i.r2;
  } else if (i.op == LD) {
    is >> i.r0;
    is >> i.r1;
  } else if (i.op == STR || i.op == BNZ) {
    is >> i.r1;
    is >> i.r2;
    i.r0 = i.r1;
  } else if (i.op == OUT) {
    is >> i.r1;
    i.r0 = i.r1;  
  } else if (i.op == HLT) {
    // do nothing
  }
  return is;
}

std::ostream& operator<<(std::ostream& os, const Instr& i) {
  switch (i.op) {
    case ADD: os << "ADD"; break;
    case SUB: os << "SUB"; break;
    case BNZ: os << "BNZ"; break;
    case LD:  os << "LD" ; break;
    case STR: os << "STR"; break;
    case OUT: os << "OUT"; break;
    case IMM: os << "IMM"; break;
    case HLT: os << "HLT"; break;
  }
  if (i.op == IMM) {
    os << ' ' << i.imm;
  } else if (i.op == ADD || i.op == SUB) {
    os << ' ' << i.r0;
    os << ' ' << i.r1;
    os << ' ' << i.r2;
  } else if (i.op == LD) {
    os << ' ' << i.r0;
    os << ' ' << i.r1;
  } else if (i.op == STR || i.op == BNZ) {
    os << ' ' << i.r1;
    os << ' ' << i.r2;
  } else if (i.op == OUT) {
    os << ' ' << i.r1;
  } else if (i.op == HLT) {
    // do nothing
  }
  return os;
}
