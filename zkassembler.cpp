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

using namespace std;

vector<string> as_program;
vector<uint64_t> hex_program;
vector<uint8_t> data_program;
map<string, uint32_t> label2pos;
map<string, uint32_t> global_var;

struct Gate
{
	int in_cnt;
	int out_cnt;
	vector <int>in_id;
	vector <int>out_id;
	string type; // XOR, AND, INV
};

void OpenAs(ifstream &fin)
{
	/* hex header*/
	// as_program.push_back("Cheat NOP");
	// as_program.push_back("HALT");
	// as_program.push_back("Proof NOP");
	// as_program.push_back("QED");
	// as_program.push_back("JUMP start");
	// as_program.push_back("RET");
	
	string inst;
	while (getline(fin, inst))
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
		string init_res = "";
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
		
		as_program.push_back(init_res);
	}
}

vector<Gate> gate;
// TODO: add INPUT inst
vector<int> input_wire;
// TODO: check OUTPUT condition
// TODO: optimize OUTPUT somehow (now, two INV?)
vector<int> output_wire;
map<string, int> reg2wire;

void Pass(int phase)
{

	int wire_cnt = 0;
	gate.clear();		
	
	int as_length = as_program.size();
	for (int i = 0; i < as_length; i++)
	{
		string inst = as_program[i];
		vector<std::string> inst_arr;
		stringstream input(inst);
		string temp;
		while(getline(input, temp, ' '))
			inst_arr.push_back(temp);
		
		int gate_n, wire_n;
		int junk_val;
		ifstream cirfin;
		if (inst_arr[0] == "ADD") cirfin.open("./BasicCircuit/adder32.txt");
		else if (inst_arr[0] == "SUB") cirfin.open("./BasicCircuit/sub32.txt");
		else if (inst_arr[0] == "MUL") cirfin.open("./BasicCircuit/mult32.txt");
		else if (inst_arr[0] == "OUTPUT")
		{
			output_wire.push_back(reg2wire[inst_arr[1]]);
			continue;
		}

		cirfin >> gate_n >> wire_n;
		
		cirfin >> junk_val >> junk_val >> junk_val;
		cirfin >> junk_val >> junk_val;

		vector<Gate> new_inst_cir;
		new_inst_cir.clear();
		new_inst_cir.resize(gate_n);
		for (int i = 0; i < gate_n; i++)
		{
			cirfin >> new_inst_cir[i].in_cnt >> new_inst_cir[i].out_cnt;
			
			new_inst_cir[i].in_id.resize(new_inst_cir[i].in_cnt);
			for (int j = 0; j < new_inst_cir[i].in_cnt; j++)
				cirfin >> new_inst_cir[i].in_id[j];
			new_inst_cir[i].out_id.resize(new_inst_cir[i].out_cnt);
			for (int j = 0; j < new_inst_cir[i].out_cnt; j++)
				cirfin >> new_inst_cir[i].out_id[j];

			cirfin >> new_inst_cir[i].type;
		}
		
		cirfin.close();				

		vector<int> idmap;
		idmap.clear();
		idmap.resize(wire_n);
		for (int i = 0; i < wire_n; i++) idmap[i] = -1;

		if (reg2wire.find(inst_arr[2]) != reg2wire.end())
		{
			for (int i = 0; i < 32; i++)
				idmap[i] = reg2wire[inst_arr[2]] + i;
		}
		else
		{
			for (int i = 0; i < 32; i++)
				idmap[i] = wire_cnt + i;
			reg2wire[inst_arr[2]] = wire_cnt;
			input_wire.push_back(wire_cnt);
			wire_cnt += 32;
		}
		
		if (reg2wire.find(inst_arr[3]) != reg2wire.end())
		{
			for (int i = 0; i < 32; i++)
				idmap[32+i] = reg2wire[inst_arr[3]] + i;
		}
		else
		{
			for (int i = 0; i < 32; i++)
				idmap[32+i] = wire_cnt + i;
			reg2wire[inst_arr[3]] = wire_cnt;
			input_wire.push_back(wire_cnt);
			wire_cnt += 32;
		}
		
		for (int i = 64; i < wire_n; i++)
			idmap[i] = wire_cnt++;

		reg2wire[inst_arr[1]] = idmap[wire_n - 32];

		// append new gates
		for (int i = 0; i < gate_n; i++)
		{
			for (int j = 0; j < new_inst_cir[i].in_cnt; j++) new_inst_cir[i].in_id[j] = idmap[new_inst_cir[i].in_id[j]];
			for (int j = 0; j < new_inst_cir[i].out_cnt; j++) new_inst_cir[i].out_id[j] = idmap[new_inst_cir[i].out_id[j]];
			gate.push_back(new_inst_cir[i]);
		}		
	}

	// TODO: optimize following two process
	
	// add output wires
	for (int i = 0; i < output_wire.size(); i++)
	{
		for (int j = 0; j < 32; j++)
		{
			Gate outputgate;
			outputgate.in_cnt = outputgate.out_cnt = 1;
			outputgate.type = "INV";
			outputgate.in_id.resize(1);
			outputgate.out_id.resize(1);
			outputgate.in_id[0] = output_wire[i] + j;
			outputgate.out_id[0] = wire_cnt++;
			gate.push_back(outputgate);
		}
	}
	for (int i = output_wire.size() * 32, tmp_wire_cnt = wire_cnt; i > 0 ; i--)
	{
		Gate outputgate;
		outputgate.in_cnt = outputgate.out_cnt = 1;
		outputgate.type = "INV";
		outputgate.in_id.resize(1);
		outputgate.out_id.resize(1);
		outputgate.in_id[0] = tmp_wire_cnt - i;
		outputgate.out_id[0] = wire_cnt++;
		gate.push_back(outputgate);
	}		

	// rearrange input wires
	vector<int> idmap;
	vector<int> inverse_idmap;
	idmap.resize(wire_cnt);
	inverse_idmap.resize(wire_cnt);
	for (int i = 0; i < wire_cnt; i++) idmap[i] = inverse_idmap[i] = i;
	for (int i = 0; i < input_wire.size(); i++)
	{
		// input_wire[i] ~ input_wire[i]+32
		// map to
		// i*32 ~ (i+1)*32

		for (int j = 0; j < 32; j++)
		{
			int tmp = idmap[input_wire[i]+j];
			idmap[input_wire[i]+j] = i*32+j;
			idmap[inverse_idmap[i*32+j]] = tmp;

			int tmp1 = inverse_idmap[i*32+j];
			inverse_idmap[i*32+j] = input_wire[i]+j;
			inverse_idmap[tmp] = tmp1;
		}
	}

	cout << gate.size() << ' ' << wire_cnt << endl;
	cout << input_wire.size();
	for (int i = 0; i < input_wire.size(); i++) cout << ' ' << 32;
	cout << endl;
	cout << output_wire.size();
	for (int i = 0; i < output_wire.size(); i++) cout << ' ' << 32;
	cout << endl;
	cout << endl;
	for (int i = 0; i < gate.size(); i++)
	{
		cout << gate[i].in_cnt << ' ' << gate[i].out_cnt << ' ';
		for (int j = 0; j < gate[i].in_cnt; j++)
			cout << idmap[gate[i].in_id[j]] << ' ';
		for (int j = 0; j < gate[i].out_cnt; j++)
			cout << idmap[gate[i].out_id[j]] << ' ';
		cout << gate[i].type << endl;
	}
}

void OutputHex(const string & filename)
{
	ofstream fout;

	fout.open(filename + ".text.hex");
	int hex_length = hex_program.size();
	for (int i = 0; i < hex_length; i++)
		fout << setfill('0') << setw(16) << hex << hex_program[i] << endl;
	fout.close();

	fout.open(filename + ".data.hex");
	int data_length = data_program.size();
	for (int i = 0; i < data_length; i++)
	{
		if (i && i%4 == 0) fout << endl;
		fout << setfill('0') << setw(2) << hex << static_cast<int>(data_program[i]);
	}
	while (data_length % 4)
	{
		fout << setfill('0') << setw(2) << hex << 0;
		data_length++;
	}
	fout.close();
	
}

int main(int argc, char ** argv)
{
	if (argc < 2)
	{
		std::cerr << "Usage: " << argv[0] << " <as_program>\n";
		std::exit(1);		
	}

	as_program.clear();
	hex_program.clear();
	data_program.clear();
	label2pos.clear();
	global_var.clear();

	data_program.push_back(0);
	data_program.push_back(0);
	data_program.push_back(0);
	data_program.push_back(0);
  
	string fullname = string(argv[1]);
	size_t lastindex = fullname.find_last_of("."); 
	string filename = fullname.substr(0, lastindex);

	ifstream fin(argv[1]);
	OpenAs(fin);
	fin.close();
	Pass(1);
	// Pass(2);
	// OutputHex(filename);
	return 0;
}
