#ifndef SGC2PC_2PC_PRG_H_
#define SGC2PC_2PC_PRG_H_

// Project Headers
#include "Prf.h"

namespace SGC {
struct PRG {
 public:
  PRG() : nonce(0) { }
  PRG(PRF prf) : prf(std::move(prf)), nonce(0) { }
  PRG(std::bitset<128> seed) : prf(std::move(seed)), nonce(0) { }

  std::bitset<128> operator()() { return prf(nonce++); }

  // TODO: check it is secure
  std::bitset<128> use_as_aes(std::size_t _nonce) { return prf(_nonce); }

  std::bitset<128> *get_seed() { return prf.get_key(); }

 private:
  PRF prf;
  std::size_t nonce;
};
};

#endif //SGC2PC_2PC_PRG_H_
