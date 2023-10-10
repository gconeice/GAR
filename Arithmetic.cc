// Project Headers
#include "Arithmetic.h"

emp::block *toBlock(const std::bitset<128> &bs) {
  return (emp::block *)(&bs);
}

std::bitset<128> *toBitset(const emp::block *b) {
  return (std::bitset<128> *)(b);
}
