#ifndef SGC2PC_2PC_ARITHMETIC_H_
#define SGC2PC_2PC_ARITHMETIC_H_

// Standard Library Headers
#include <bitset>
#include <vector>

// Submodule Headers
#include <emp-tool/utils/block.h>

emp::block *toBlock(const std::bitset<128> &bs);
std::bitset<128> *toBitset(const emp::block *b);

#endif //SGC2PC_2PC_ARITHMETIC_H_
