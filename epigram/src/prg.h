#ifndef EPPRG_H__
#define EPPRG_H__


#include "prf.h"


struct EPPRG {
public:
  EPPRG() : nonce(0) { }
  EPPRG(EPCCRH f) : f(std::move(f)), nonce(0) { }
  EPPRG(std::bitset<128> seed) : f(std::move(seed)), nonce(0) { }

  Label operator()() { return f(Label { static_cast<int>(nonce++) }); }
  WLabel wide() {
    auto nonces = WLabel {
      std::array<Label, 8> {
        nonce + 0,
        nonce + 1,
        nonce + 2,
        nonce + 3,
        nonce + 4,
        nonce + 5,
        nonce + 6,
        nonce + 7
      } };
    return f(nonces);
  }

private:
  EPCCRH f;
  std::size_t nonce;
};


#endif
