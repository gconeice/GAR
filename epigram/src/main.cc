#include <lazypermutation.h>
#include <permute.h>
#include <util.h>
#include <epigram.h>
#include <arithmetic.h>

#include <util.h>
#include <chrono>

template <typename F>
auto timed(F f) {
  auto start = std::chrono::high_resolution_clock::now();
  f();
  auto finish = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = finish - start;
  return elapsed.count();
}

/* int main() { */
/*   constexpr std::size_t n = 512; */
/*   constexpr std::size_t w = 32; */
/*   constexpr std::size_t t = 1024; */
/*   constexpr std::size_t scan_pref = 128; */
/*   auto out = ckt([]<Mode m> { */
/*     auto arr = EpiGRAM<m>::make(w, n, scan_pref); */

/*     auto ix = constant_nat<m>(log2(n), 16); */
/*     auto val = constant_nat<m>(w, 100); */
/*     auto r = arr.write(ix, val); */
/*     for (std::size_t i = 1; i < t; ++i) { */
/*       r = arr.write(ix, val); */
/*     } */
/*     return dec(std::span<const Garbled::Bit<m>> { r }); */
/*   }); */

/*   for (auto o: out) { */
/*     std::cout << o; */
/*   } */
/*   std::cout << '\n'; */
/* } */


#include <cpu.h>


int main() {
  Config cfg { 256, 256, 64 };

  constexpr std::size_t n = 64;
  constexpr std::size_t w = 10;
  EPCCRH key = EPPRG()();
  EPCCRH key22;

  initialize<Mode::S>(key22);
  auto arr_s = EpiGRAM<Mode::S>::make(w, n);
  const auto ix_s_0 = constant_nat<Mode::S>(log2(n), 100 % n);
  const auto val_s_0 = constant_nat<Mode::S>(w, 200 % (1 << w));
  const auto ix_s_1 = constant_nat<Mode::S>(log2(n), 300 % n);
  const auto val_s_1 = constant_nat<Mode::S>(w, 400 % (1 << w));
  arr_s.write(ix_s_0, val_s_0);
  arr_s.write(ix_s_1, val_s_1);
  const auto ix_s = constant_nat<Mode::S>(log2(n), 10 % n);
  auto out_s = arr_s.read(ix_s);
  auto out_s_int = dec_nat<Mode::S>(std::span { out_s });
  std::cout << out_s_int << std::endl;
  std::vector<Garbled::Bit<Mode::S>> acc_s(6);
  for (int i = 0; i < 6; i++) acc_s[i] = ginput<Mode::S>((20 & (1 << i)) > 0);
  auto out_s_1 = arr_s.read(acc_s);
  auto out_s_1_int = dec_nat<Mode::S>(std::span { out_s_1 });
  std::cout << out_s_1_int << std::endl;

  initialize<Mode::G>(key);
  auto arr_g = EpiGRAM<Mode::G>::make(w, n);
  const auto ix_g_0 = constant_nat<Mode::G>(log2(n), 0 % n);
  const auto val_g_0 = constant_nat<Mode::G>(w, 2 % (1 << w));
  const auto ix_g_1 = constant_nat<Mode::G>(log2(n), 1 % n);
  const auto val_g_1 = constant_nat<Mode::G>(w, 1 % (1 << w));
  arr_g.write(ix_g_0, val_g_0);
  arr_g.write(ix_g_1, val_g_1);
  const auto ix_g = constant_nat<Mode::G>(log2(n), 0 % n);
  auto out_g = arr_g.read(ix_g);
  auto out_g_int = dec_nat<Mode::G>(std::span { out_g });
  std::cout << out_g_int << std::endl;
  std::vector<Garbled::Bit<Mode::G>> acc_g(6);
  for (int i = 0; i < 6; i++) acc_g[i] = ginput<Mode::G>((1 & (1 << i)) > 0);
  auto out_g_1 = arr_g.read(acc_g);
  auto out_g_1_int = dec_nat<Mode::G>(std::span { out_g_1 });
  std::cout << out_g_1_int << std::endl;  

  initialize<Mode::E>(key);
  auto arr_e = EpiGRAM<Mode::E>::make(w, n);
  const auto ix_e_0 = constant_nat<Mode::E>(log2(n), 0 % n);
  const auto val_e_0 = constant_nat<Mode::E>(w, 2 % (1 << w));
  const auto ix_e_1 = constant_nat<Mode::E>(log2(n), 1 % n);
  const auto val_e_1 = constant_nat<Mode::E>(w, 1 % (1 << w));
  arr_e.write(ix_e_0, val_e_0);
  arr_e.write(ix_e_1, val_e_1);  
  const auto ix_e = constant_nat<Mode::E>(log2(n), 0 % n);
  auto out_e = arr_e.read(ix_e);
  auto out_e_int = dec_nat<Mode::E>(std::span { out_e });
  std::cout << out_e_int << std::endl;
  std::vector<Garbled::Bit<Mode::E>> acc_e(6);
  for (int i = 0; i < 6; i++) acc_e[i] = ginput<Mode::E>((0 & (1 << i)) > 0);
  auto out_e_1 = arr_e.read(acc_e);
  auto out_e_1_int = dec_nat<Mode::E>(std::span { out_e_1 });
  std::cout << out_e_1_int << std::endl;   

  std::vector<Instr> program;
  Instr i;
  while (std::cin >> i) {
    program.push_back(i);
  }
  Instr halt { HLT, 0, 0, 0, 0 };
  while (!is_pow2(program.size())) {
    program.push_back(halt);
  }

  ckt([&]<Mode m> {
    auto cpu = CPU<m>::make(cfg, program);
    cpu.run();
  });
}
