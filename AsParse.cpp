#include "AsParse.h"
#include "Bit32.h"
#include <map>
#include <cstring>
#include <algorithm>
#include <cstdlib>

namespace SGC {

void OpenAs(std::ifstream &fin, std::vector<std::string> &as_program)
{
	/* hex header*/
	// as_program.push_back("Cheat NOP");
	// as_program.push_back("HALT");
	// as_program.push_back("Proof NOP");
	// as_program.push_back("QED");
	// as_program.push_back("JUMP start");
	// as_program.push_back("RET");
	
	std::string inst;
	while (std::getline(fin, inst))
	{
		if (!inst.length()) continue;
		// replace(inst.begin(), inst.end(), '\t', ' ');

		bool ascii_mode = 0;
		
		for (int i = 0; i < inst.length(); i++)
		{
			if (inst[i] == '"') ascii_mode = !ascii_mode;
			if (ascii_mode) continue;
			if (inst[i] == '\t') inst[i] = ' ';
			if (inst[i] == ',') inst[i] = ' ';
			if (inst[i] == '\n') {
				inst.erase(i, 1);
				i--;
			}
		}
		
		// replace(inst.begin(), inst.end(), ',', ' ');
		// inst.erase(remove(inst.begin(), inst.end(), '\n'), inst.end());
		
		for (int i = 0; i < inst.length(); i++)
			if (inst[i] != ' ')
			{
				inst.erase(0, i);
				break;
			}
		for (int i = inst.length(); i > 0; i--)
			if (inst[i-1] != ' ')
			{
				inst.erase(i);
				break;
			}
		ascii_mode = 0;
		std::string init_res = "";
		for(int i = 0; inst[i] != '\0'; i++)
		{
			if (inst[i] == '"')
			// {
				ascii_mode = !ascii_mode;
			// 	init_res.append(1, inst[i]);
			// 	continue;
			// }
			if (ascii_mode)
			{
				init_res.append(1, inst[i]);
				continue;
			}
			if (inst[i] != ' ')
				init_res.append(1, inst[i]);
			else if(inst[i+1] != ' ')
				init_res.append(1, inst[i]);
		}

		if (init_res == " ") continue;

		if (init_res[0] == '#') continue;

		as_program.push_back(init_res);
	}
	as_program.push_back("BB_NOOP:");
	as_program.push_back("NOOP");
    // std::cout << "DEBUG " << as_program.size() << std::endl;
}

void GenerateCFG(std::vector<std::string> &as_program, std::vector<BasicBlock> &CFG)
{	

    //std::map<std::string, SGC::Bit32<role>> reg;

	int as_length = as_program.size();

	std::map<std::string, uint32_t> label_to_addr;
	uint32_t ctr = 0;

	for (int i = 0; i < as_length; i++)
	{
		std::string inst = as_program[i];

		if (inst[inst.length()-1] == ':') {
			label_to_addr[inst.erase(inst.length() - 1)] = ctr;
			continue;
		}

		ctr++;
    }

	BasicBlock bb; bb.clear(); ctr = 0;
	std::map<std::string, uint32_t> label_to_bbid;

	for (int i = 0; i < as_length; i++)
	{
		std::string inst = as_program[i];

		if (inst[inst.length()-1] == ':') {
			if (bb.fragment.size()) {
				bb.fragment.push_back(Inst{OPCODE::IMM, REG::PC, REG::PC, REG::PC, ctr});
				bb.next_BB.push_back(CFG.size() + 1);
				CFG.push_back(bb);
				bb.clear(); bb.start_addr = ctr;				
			}
			label_to_bbid[inst.erase(inst.length() - 1)] = CFG.size();
			continue;
		}

		ctr++;

		std::vector<std::string> inst_arr;
		std::stringstream input(inst);		
        std::string temp;
		while(getline(input, temp, ' '))
			inst_arr.push_back(temp);
		
		if (inst_arr[0] == "EQ") {
			bb.fragment.push_back(Inst{OPCODE::EQ, 
									   StringToREG(inst_arr[1]), 
									   StringToREG(inst_arr[2]),
									   StringToREG(inst_arr[3]),
									   0});
		} else if (inst_arr[0] == "EQI") {
			bb.fragment.push_back(Inst{OPCODE::EQI, 
									   StringToREG(inst_arr[1]), 
									   StringToREG(inst_arr[2]),
									   REG::GR0,
									   (uint32_t)std::stol(inst_arr[3])});
		} else if (inst_arr[0] == "RS1") {
			bb.fragment.push_back(Inst{OPCODE::RS1, 
									   StringToREG(inst_arr[1]), 
									   REG::GR0,
									   REG::GR0,
									   0});
		} else if (inst_arr[0] == "CMP") {
			bb.fragment.push_back(Inst{OPCODE::CMP, 
									   StringToREG(inst_arr[1]), 
									   StringToREG(inst_arr[2]),
									   StringToREG(inst_arr[3]),
									   0});
		} else if (inst_arr[0] == "SWAP") {
			bb.fragment.push_back(Inst{OPCODE::SWAP, 
									   REG::GR0, 
									   StringToREG(inst_arr[1]),
									   StringToREG(inst_arr[2]),
									   0});
		} else if (inst_arr[0] == "SUB") {
			bb.fragment.push_back(Inst{OPCODE::SUB, 
									   StringToREG(inst_arr[1]), 
									   StringToREG(inst_arr[2]),
									   StringToREG(inst_arr[3]),
									   0});
		} else if (inst_arr[0] == "SUBI") {
			bb.fragment.push_back(Inst{OPCODE::SUBI, 
									   StringToREG(inst_arr[1]), 
									   StringToREG(inst_arr[2]),
									   REG::GR0,
									   (uint32_t)std::stol(inst_arr[3])});
		} else if (inst_arr[0] == "ADD") {
			bb.fragment.push_back(Inst{OPCODE::ADD, 
									   StringToREG(inst_arr[1]), 
									   StringToREG(inst_arr[2]),
									   StringToREG(inst_arr[3]),
									   0});
		} else if (inst_arr[0] == "XOR") {
			bb.fragment.push_back(Inst{OPCODE::XOR, 
									   StringToREG(inst_arr[1]), 
									   StringToREG(inst_arr[2]),
									   StringToREG(inst_arr[3]),
									   0});
		} else if (inst_arr[0] == "ADDI") {
			bb.fragment.push_back(Inst{OPCODE::ADDI, 
									   StringToREG(inst_arr[1]), 
									   StringToREG(inst_arr[2]),
									   REG::GR0,
									   (uint32_t)std::stol(inst_arr[3])});
		} else if (inst_arr[0] == "J") {
			bb.fragment.push_back(Inst{OPCODE::IMM,
									   REG::PC,
									   REG::PC,
									   REG::PC,
									   label_to_addr[inst_arr[1]]});
			bb.branch_BB = inst_arr[1];
			CFG.push_back(bb);
			bb.clear(); bb.start_addr = ctr;
		} else if (inst_arr[0] == "JE") {
			bb.fragment.push_back(Inst{OPCODE::IMM, REG::SP0, REG::SP0, REG::SP0, ctr});
			bb.fragment.push_back(Inst{OPCODE::IMM, REG::SP1, REG::SP1, REG::SP1, label_to_addr[inst_arr[2]]});
			bb.fragment.push_back(Inst{OPCODE::ANDN0, REG::SP0, REG::SP0, StringToREG(inst_arr[1]), 0});
			bb.fragment.push_back(Inst{OPCODE::AND0, REG::SP1, REG::SP1, StringToREG(inst_arr[1]), 0});
			bb.fragment.push_back(Inst{OPCODE::XOR, REG::PC, REG::SP0, REG::SP1, 0});
			bb.branch_BB = inst_arr[2];
			bb.next_BB.push_back(CFG.size() + 1);
			CFG.push_back(bb);
			bb.clear(); bb.start_addr = ctr;
		} else if (inst_arr[0] == "JNE") {
			bb.fragment.push_back(Inst{OPCODE::IMM, REG::SP0, REG::SP0, REG::SP0, ctr});
			bb.fragment.push_back(Inst{OPCODE::IMM, REG::SP1, REG::SP1, REG::SP1, label_to_addr[inst_arr[2]]});
			bb.fragment.push_back(Inst{OPCODE::AND0, REG::SP0, REG::SP0, StringToREG(inst_arr[1]), 0});
			bb.fragment.push_back(Inst{OPCODE::ANDN0, REG::SP1, REG::SP1, StringToREG(inst_arr[1]), 0});
			bb.fragment.push_back(Inst{OPCODE::XOR, REG::PC, REG::SP0, REG::SP1, 0});
			bb.branch_BB = inst_arr[2];
			bb.next_BB.push_back(CFG.size() + 1);
			CFG.push_back(bb);
			bb.clear(); bb.start_addr = ctr;
		} else if (inst_arr[0] == "JB") {
			bb.fragment.push_back(Inst{OPCODE::IMM, REG::SP0, REG::SP0, REG::SP0, ctr});
			bb.fragment.push_back(Inst{OPCODE::IMM, REG::SP1, REG::SP1, REG::SP1, label_to_addr[inst_arr[2]]});
			bb.fragment.push_back(Inst{OPCODE::ANDN0, REG::SP0, REG::SP0, StringToREG(inst_arr[1]), 0});
			bb.fragment.push_back(Inst{OPCODE::AND0, REG::SP1, REG::SP1, StringToREG(inst_arr[1]), 0});
			bb.fragment.push_back(Inst{OPCODE::XOR, REG::PC, REG::SP0, REG::SP1, 0});
			bb.branch_BB = inst_arr[2];
			bb.next_BB.push_back(CFG.size() + 1);
			CFG.push_back(bb);
			bb.clear(); bb.start_addr = ctr;
		} else if (inst_arr[0] == "JL") {
			bb.fragment.push_back(Inst{OPCODE::IMM, REG::SP0, REG::SP0, REG::SP0, ctr});
			bb.fragment.push_back(Inst{OPCODE::IMM, REG::SP1, REG::SP1, REG::SP1, label_to_addr[inst_arr[2]]});
			bb.fragment.push_back(Inst{OPCODE::ANDN1, REG::SP0, REG::SP0, StringToREG(inst_arr[1]), 0});
			bb.fragment.push_back(Inst{OPCODE::AND1, REG::SP1, REG::SP1, StringToREG(inst_arr[1]), 0});
			bb.fragment.push_back(Inst{OPCODE::XOR, REG::PC, REG::SP0, REG::SP1, 0});
			bb.branch_BB = inst_arr[2];
			bb.next_BB.push_back(CFG.size() + 1);
			CFG.push_back(bb);
			bb.clear(); bb.start_addr = ctr;
		} else if (inst_arr[0] == "HALT") {
			bb.fragment.push_back(Inst{OPCODE::IMM,
									   REG::PC,
									   REG::PC,
									   REG::PC,
									   label_to_addr["BB_NOOP"]});
			bb.branch_BB = "BB_NOOP";
			CFG.push_back(bb);
			bb.clear(); bb.start_addr = ctr;			
		} else if (inst_arr[0] == "NOOP") {
			bb.fragment.push_back(Inst{OPCODE::NOOP,
									   REG::GR0,
									   REG::GR0,
									   REG::GR0,
									   0});
			bb.branch_BB = "BB_NOOP";
			CFG.push_back(bb);
			bb.clear(); bb.start_addr = ctr;
		} else if (inst_arr[0] == "IMM") {
			bb.fragment.push_back(Inst{OPCODE::IMM, StringToREG(inst_arr[1]), REG::GR0, REG::GR0, (uint32_t)std::stol(inst_arr[2])});
		} else if (inst_arr[0] == "COPY") {
			bb.fragment.push_back(Inst{OPCODE::COPY, StringToREG(inst_arr[1]), StringToREG(inst_arr[2]), REG::GR0, 0});
		} else if (inst_arr[0] == "CCOPY") {
			bb.fragment.push_back(Inst{OPCODE::CCOPY, 
									   StringToREG(inst_arr[1]), 
									   StringToREG(inst_arr[2]),
									   StringToREG(inst_arr[3]),
									   0});
		} else if (inst_arr[0] == "LOAD") {
			bb.fragment.push_back(Inst{OPCODE::LOAD, StringToREG(inst_arr[1]), StringToREG(inst_arr[2]), REG::GR0, 0});
			bb.acc_cnt++;
		} else if (inst_arr[0] == "STORE") {
			bb.fragment.push_back(Inst{OPCODE::STORE, StringToREG(inst_arr[1]), StringToREG(inst_arr[2]), REG::GR0, 0});
			bb.acc_cnt++;
		} else {
			std::cerr << "Invalid Inst in Program Parser: " << inst_arr[0] << std::endl;
			exit(-1);
		}
    }	

	for (int i = 0; i < CFG.size(); i++) 
		if (CFG[i].branch_BB != "")
			CFG[i].next_BB.push_back(label_to_bbid[CFG[i].branch_BB]);
}

uint32_t GenerateBBPath(const int &cnt, std::vector<BasicBlock> &CFG, std::vector<std::vector<uint32_t>> &BBPath) {
	BBPath.resize(cnt);
	uint32_t sum_acc_cnt = 0;
	bool check[CFG.size()];
	memset(check, 0, sizeof(check));
	check[0] = true;
	for (int i = 0; i < cnt; i++) {
		BBPath[i].clear();
		bool tmp_check[CFG.size()];
		uint32_t max_acc_cnt = 0;
		memset(tmp_check, 0, sizeof(tmp_check));
		for (int id = 0; id < CFG.size(); id++) {
			if (check[id] == false) continue;
			BBPath[i].push_back(id);
			max_acc_cnt = std::max(max_acc_cnt, CFG[id].acc_cnt);
			for (auto x : CFG[id].next_BB) tmp_check[x] = true;
		}
		sum_acc_cnt += max_acc_cnt;
		for (int id = 0; id < CFG.size(); id++) check[id] = tmp_check[id];
	}	
	return sum_acc_cnt;
}

};