#ifndef SGC_ASPARSE_H__
#define SGC_ASPARSE_H__

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <vector>
#include <map>
#include <fstream>
#include "Role.h"
//#include "SGCUtils.h"
#include "BasicType.h"

namespace SGC {

void OpenAs(std::ifstream &fin, std::vector<std::string> &as_program);
void GenerateCFG(std::vector<std::string> &as_program, std::vector<BasicBlock> &CFG);
uint32_t GenerateBBPath(const int &cnt, std::vector<BasicBlock> &CFG, std::vector<std::vector<uint32_t>> &BBPath);

inline REG StringToREG(const std::string &in) {
    if (in == "%GR0") return REG::GR0;
    if (in == "%GR1") return REG::GR1;
    if (in == "%GR2") return REG::GR2;
    if (in == "%GR3") return REG::GR3;
    if (in == "%GR4") return REG::GR4;
    if (in == "%GR5") return REG::GR5;
    if (in == "%GR6") return REG::GR6;
    if (in == "%GR7") return REG::GR7;
    if (in == "%GR8") return REG::GR8;
    if (in == "%PC") return REG::PC;
    if (in == "%SP0") return REG::SP0;
    if (in == "%SP1") return REG::SP1;
    if (in == "%SP2") return REG::SP2;
    std::cerr << "Invalid REG: " << in << std::endl;
    exit(-1);
}

};

#endif