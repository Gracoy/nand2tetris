#include<string>
#include<iostream>
#include<fstream>
#include<unordered_map>
#include<algorithm>

using namespace std;
unordered_map<string, int> init_symbol();
string get_filename(string & in_filename);
string A_decTobin(string &dec);
string C_compTobin(string &s);
string C_destTobin(string &s);
string C_jumpTobin(string &s);

int main(int argc, char** argv)
{
	// A-type: A_decTobin(dec)
	// C-type: 111 + comp + dest + jump

	unordered_map<string, int> symbolTable = init_symbol();
	int var_idx = 16;
	int pc = 0;
	string instruction = "";
	string in_filename = argv[1];

	ifstream in_f(in_filename);
	if (!in_f.is_open())
	{
		cout << "Open file " << in_filename << " fail." << endl;
		exit(1);
	}

	// 1st pass, only focus on (SYMBOL)
	while (getline(in_f, instruction))
	{
		const int n = instruction.length();
		if (instruction[0] == '\r')
			continue;

		for (int i = 0; i < n; i++)
		{
			if (instruction[i] == '/')
				break;
			if (instruction[i] == ' ' || instruction[i] == '\t')
				continue;
			if (instruction[i] != '(') // Normal instruction
			{
				pc++;
				break;
			}
			if (instruction[i] == '(') // Jump symbol
			{
				i++;
				string sym = "";
				while (instruction[i] != ')')
				{
					sym += instruction[i];
					i++;
				}
				if (symbolTable.find(sym) == symbolTable.end())
					symbolTable[sym] = pc;
				break;
			}
		}
	}
	in_f.close();

	in_f.open(in_filename);
	if (!in_f.is_open())
	{
		cout << "Open file " << in_filename << " fail." << endl;
		exit(1);
	}

	string out_filename = get_filename(in_filename);
	ofstream out_f(out_filename);
	if (!out_f.is_open())
	{
		cout << "here";
		cout << "Open file " << out_filename << " fail." << endl;
		exit(1);
	}

	// 2nd pass
	while (getline(in_f, instruction))
	{
		const int n = instruction.length();
		if (instruction[0] == '\r')
			continue;

		for (int i = 0; i < n; i++)
		{
			if (instruction[i] == '/' || instruction[i] == '(')
				break;

			if (instruction[i] == '@')
			{
				string sym = "";
				i++;
				if (isdigit(instruction[i]))
				{
					while (i < n)
					{
						if (isdigit(instruction[i]))
							sym += instruction[i];
						i++;
					}
				}
				else
				{
					while (i < n && instruction[i] != '\r' && instruction[i] != '/')
					{
						if (instruction[i] != ' ' && instruction[i] != '\t')
						{
							sym += instruction[i];
						}
						i++;
					}
					if (symbolTable.find(sym) == symbolTable.end())
					{
						symbolTable[sym] = var_idx;
						sym = to_string(var_idx++);				
					}
					else
						sym = to_string(symbolTable[sym]);
				}
				out_f << A_decTobin(sym) << endl;
				break;
			}
			else if (isalpha(instruction[i]) || isdigit(instruction[i]))
			{
				string dest = "";
				string comp = "";
				string jump = "";

				auto it = find(instruction.begin(), instruction.end(), '=');
				if (it != instruction.end()) // '=' in C-instruction --> dest = comp
				{
					while (i < n && instruction[i] != '=')
					{
						if (instruction[i] != ' ' && instruction[i] != '\t')
							dest += instruction[i];
						i++;
					}
					i++;
					while (i < n && instruction[i] != '\r' && instruction[i] != '/')
					{
						if (instruction[i] != ' ' && instruction[i] != '\t')
							comp += instruction[i];
						i++;
					}
				}
				else			    // ';' in C-instruction --> comp;jump
				{
					while (i < n && instruction[i] != ';')
					{
						if (instruction[i] != ' ' && instruction[i] != '\t')
							comp += instruction[i];
						i++;
					}
					i++;
					while (jump.length() < 3)
					{
						if (isalpha(instruction[i]))
							jump += instruction[i];
						i++;
					}
				}
				//cout << "Dest: " << dest << " Comp: " << comp << " Jump: " << jump << endl;
				//cout << C_compTobin(comp) << C_destTobin(dest) << C_jumpTobin(jump) << endl;
				out_f << "111" << C_compTobin(comp) << C_destTobin(dest) << C_jumpTobin(jump) << endl;
				break;
			}
		}
	}
	in_f.close();
	out_f.close();
	return 0;
}

unordered_map<string, int> init_symbol()
{
	unordered_map<string, int> sbt;
	sbt["R0"] = 0;
	sbt["R1"] = 1;
	sbt["R2"] = 2;
	sbt["R3"] = 3;
	sbt["R4"] = 4;
	sbt["R5"] = 5;
	sbt["R6"] = 6;
	sbt["R7"] = 7;
	sbt["R8"] = 8;
	sbt["R9"] = 9;
	sbt["R10"] = 10;
	sbt["R11"] = 11;
	sbt["R12"] = 12;
	sbt["R13"] = 13;
	sbt["R14"] = 14;
	sbt["R15"] = 15;
	sbt["SCREEN"] = 16384;
	sbt["KBD"] = 24576;
	sbt["SP"] = 0;
	sbt["LCL"] = 1;
	sbt["ARG"] = 2;
	sbt["THIS"] = 3;
	sbt["THAT"] = 4;

	return sbt;
}

string get_filename(string &in_filename)
{
	string out_filename = "";
	for (char &x: in_filename)
	{
		if (x == '.')
			break;
		out_filename += x;
	}
	out_filename += ".hack";
	return out_filename;
}

string A_decTobin(string& instr)
{
	string res = "";
	int n = stoi(instr);
	while (n)
	{
		res = (n & 1) ? "1" + res : "0" + res;
		n >>= 1;
	}
	while (res.length() < 16)
		res = "0" + res;

	return res;
}

string C_compTobin(string &s)
{
	if (s == "0")   return "0101010";
	if (s == "1")   return "0111111";
	if (s == "-1")  return "0111010";
	if (s == "D")   return "0001100";
	if (s == "A")   return "0110000";

	if (s == "!D")  return "0001101";
	if (s == "!A")  return "0110001";
	if (s == "-D")  return "0001111";
	if (s == "-A")  return "0110011";
	if (s == "D+1" || s == "1+D") return "0011111";

	if (s == "A+1" || s == "1+A") return "0110111";
	if (s == "D-1") return "0001110";
	if (s == "A-1") return "0110010";
	if (s == "D+A" || s == "A+D") return "0000010";
	if (s == "D-A") return "0010011";

	if (s == "A-D") return "0000111";
	if (s == "D&A" || s == "A&D") return "0000000";
	if (s == "D|A" || s == "A|D") return "0010101";

	if (s == "M")   return "1110000";
	if (s == "!M")  return "1110001";
	if (s == "-M")  return "1110011";
	if (s == "M+1") return "1110111";
	if (s == "M-1") return "1110010";

	if (s == "D+M" || s == "M+D") return "1000010";
	if (s == "D-M") return "1010011";
	if (s == "M-D") return "1000111";
	if (s == "D&M" || s == "M&D") return "1000000";
	if (s == "D|M" || s == "M|D") return "1010101";

	return "Comp encoding error: " + s;
}

string C_destTobin(string &s)
{
	string dest = "000";
	for (char &x: s)
	{
		if (x == 'M')
			dest[2] = '1';
		else if (x == 'D')
			dest[1] = '1';
		else if (x == 'A')
			dest[0] = '1';	
		else
			return "Dest encoding error !";
	}
	return dest;
}

string C_jumpTobin(string &s)
{
	if (s == "")    return "000";
	if (s == "JGT") return "001";
	if (s == "JEQ") return "010";
	if (s == "JGE") return "011";
	if (s == "JLT") return "100";
	if (s == "JNE") return "101";
	if (s == "JLE") return "110";
	if (s == "JMP") return "111";

	return "Jump encoding error !";
}
