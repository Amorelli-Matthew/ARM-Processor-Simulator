//Matthew Amorelli
//April 27, 2025
//Project 3


#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <cstdint>
#include <iomanip>
#include <unordered_map>
#include <fstream>
#include <algorithm>
#include <stdexcept>
#include <bitset>

using namespace std;

// Processor state containing registers, memory, flags, and program counter
struct ProcessorState {
    uint32_t registers[12] = {0};
    uint32_t memory[5] = {0};
    bool N_flag = false, Z_flag = false, C_flag = false, V_flag = false;
    size_t pc = 0;
    unordered_map<string, size_t> labels;
};

// Parses the operand value
uint32_t parseValue(const string& s, const ProcessorState& state) {
    if (s.empty())
    {
        return 0;
    }

    if (s[0] == 'R')
    {
        int reg;

        if (s.size() > 2 && s[1] == '1' && s[2] == '0')
        {
            reg = 10;
        }
        else if (s.size() > 2 && s[1] == '1' && s[2] == '1')
        {
            reg = 11;
        }
        else
        {
            reg = stoi(s.substr(1));
        }

        if (reg < 0 || reg >= 12)
        {
            throw out_of_range("Invalid register");
        }

        return state.registers[reg];
    }

    size_t start = (s[0] == '#') ? 1 : 0;

    if (s.size() >= start + 2 && s.substr(start, 2) == "0x")
    {
        return stoul(s.substr(start), nullptr, 16);
    }

    return stoul(s.substr(start));
}

// Splits a string into tokens
vector<string> split(const string& str) {
    vector<string> tokens;
    stringstream ss(str);
    string token;

    while (ss >> token)
    {
        if (!token.empty())
        {
            tokens.push_back(token);
        }
    }

    return tokens;
}

// Determines if a conditional instruction should execute
bool shouldExecute(const string& cond, const ProcessorState& state) {
    if (cond.empty())
    {
        return true;
    }

    if (cond == "EQ")
    {
        return state.Z_flag;
    }

    if (cond == "NE")
    {
        return !state.Z_flag;
    }

    if (cond == "GT")
    {
        return !state.Z_flag && (state.N_flag == state.V_flag);
    }

    if (cond == "GE")
    {
        return (state.N_flag == state.V_flag);
    }

    if (cond == "LT")
    {
        return (state.N_flag != state.V_flag);
    }

    if (cond == "LE")
    {
        return state.Z_flag || (state.N_flag != state.V_flag);
    }

    return false;
}

// Updates flags after an arithmetic or logical operation
void updateFlags(ProcessorState& state, uint32_t result, uint32_t op1, uint32_t op2, bool isSub = false) {
    state.Z_flag = (result == 0);
    state.N_flag = (result >> 31) & 1;

    if (isSub)
    {
        state.C_flag = (op1 >= op2);
        state.V_flag = ((op1 ^ op2) & (op1 ^ result)) >> 31;
    }
    else
    {
        state.C_flag = (result < op1);
        state.V_flag = (~(op1 ^ op2) & (op1 ^ result)) >> 31;
    }
}

// Prints the current processor state
void printState(const ProcessorState& state, const string& instruction) {
    cout << instruction << endl;
    cout << "Register array:" << endl;

    for (int i = 0; i < 12; ++i)
    {
        cout << "R" << (i < 10 ? to_string(i) : (i == 10 ? "10" : "11")) << "=0x" << hex << state.registers[i] << " ";

        if (i == 5)
        {
            cout << endl;
        }
    }

    cout << endl;
    cout << "NZCV: " << state.N_flag << state.Z_flag << state.C_flag << state.V_flag << endl;
    cout << "Memory array:" << endl;

    for (int i = 0; i < 5; ++i)
    {
        if (state.memory[i] == 0)
        {
            cout << "___";
        }
        else
        {
            cout << "0x" << hex << state.memory[i];
        }

        cout << (i < 4 ? "," : "\n");
    }

    cout << endl;
}

// Executes a single instruction
void executeInstruction(const string& line, ProcessorState& state) {
    if (line.empty())
    {
        return;
    }

    string processed = line;
    replace(processed.begin(), processed.end(), ',', ' ');
    vector<string> tokens = split(processed);

    if (tokens.empty())
    {
        return;
    }

    if (tokens[0].back() == ':')
    {
        return;
    }

    string op = tokens[0];
    string cond;
    bool updateFlagsRequested = false;

    static const vector<string> condSuffixes = {"EQ", "NE", "GT", "GE", "LT", "LE"};

    for (const auto& suffix : condSuffixes)
    {
        if (op.size() > suffix.size() && op.substr(op.size() - suffix.size()) == suffix)
        {
            cond = suffix;
            op = op.substr(0, op.size() - suffix.size());
            break;
        }
    }

    if (!op.empty() && op.back() == 'S')
    {
        updateFlagsRequested = true;
        op.pop_back();
    }

    if (!shouldExecute(cond, state))
    {
        printState(state, line);
        return;
    }

    try
    {
        if (op == "MOV")
        {
            int rd = stoi(tokens[1].substr(1));
            state.registers[rd] = parseValue(tokens[2], state);
        }
        else if (op == "MVN")
        {
            int rd = stoi(tokens[1].substr(1));
            state.registers[rd] = ~parseValue(tokens[2], state);

            if (updateFlagsRequested)
            {
                state.Z_flag = (state.registers[rd] == 0);
                state.N_flag = (state.registers[rd] >> 31) & 1;
            }
        }
        else if (op == "CMP")
        {
            uint32_t op1 = parseValue(tokens[1], state);
            uint32_t op2 = parseValue(tokens[2], state);
            updateFlags(state, op1 - op2, op1, op2, true);
        }
        else if (op == "ADD" || op == "SUB" || op == "AND" || op == "ORR" || op == "EOR")
        {
            int rd = stoi(tokens[1].substr(1));
            uint32_t rn = parseValue(tokens[2], state);
            uint32_t op2 = parseValue(tokens[3], state);

            uint32_t result;

            if (op == "ADD")
            {
                result = rn + op2;
            }
            else if (op == "SUB")
            {
                result = rn - op2;
            }
            else if (op == "AND")
            {
                result = rn & op2;
            }
            else if (op == "ORR")
            {
                result = rn | op2;
            }
            else
            {
                result = rn ^ op2;
            }

            state.registers[rd] = result;

            if (updateFlagsRequested)
            {
                updateFlags(state, result, rn, op2, op == "SUB");
            }
        }
        else if (op == "LSL" || op == "LSR" || op == "ASR" || op == "ROR")
        {
            int rd = stoi(tokens[1].substr(1));
            uint32_t rn = parseValue(tokens[2], state);
            uint32_t shift = parseValue(tokens[3], state);

            if (shift > 31)
            {
                throw invalid_argument("Shift amount must be 0-31");
            }

            uint32_t result;

            if (op == "LSL")
            {
                result = rn << shift;
            }
            else if (op == "LSR")
            {
                result = rn >> shift;
            }
            else if (op == "ASR")
            {
                result = static_cast<uint32_t>(static_cast<int32_t>(rn) >> shift);
            }
            else
            {
                result = (rn >> shift) | (rn << (32 - shift));
            }

            state.registers[rd] = result;

            if (updateFlagsRequested)
            {
                updateFlags(state, result, rn, 0);
            }
        }
        else if (op == "LDR" || op == "STR")
        {
            int rd = stoi(tokens[1].substr(1));
            string addrStr = tokens[2].substr(1, tokens[2].size() - 2);
            int addrReg = stoi(addrStr.substr(1));
            uint32_t addr = state.registers[addrReg];

            if (addr < 0x100 || addr > 0x110 || (addr % 4 != 0))
            {
                throw out_of_range("Invalid memory address");
            }

            size_t index = (addr - 0x100) / 4;

            if (index >= 5)
            {
                throw out_of_range("Memory index out of bounds");
            }

            if (op == "LDR")
            {
                state.registers[rd] = state.memory[index];
            }
            else
            {
                state.memory[index] = state.registers[rd];
            }
        }
        else if (op == "BEQ")
        {
            if (state.Z_flag)
            {
                string label = tokens[1];

                if (state.labels.count(label))
                {
                    state.pc = state.labels[label];
                    printState(state, line);
                    return;
                }
            }
        }
        else if (op == "NOP")
        {
            // No operation
        }
        else
        {
            throw invalid_argument("Unknown instruction");
        }
    }
    catch (const exception& e)
    {
        cerr << "[ERROR] In instruction '" << line << "': " << e.what() << endl;
    }

    printState(state, line);
}

// Reads instructions from a file
vector<string> readInstructionsFromFile(const string& filename, ProcessorState& state) {
    vector<string> instructions;
    ifstream file(filename);

    if (!file.is_open())
    {
        cerr << "Error opening file: " << filename << endl;
        return instructions;
    }

    string line;

    while (getline(file, line))
    {
        size_t commentPos = line.find(';');

        if (commentPos != string::npos)
        {
            line = line.substr(0, commentPos);
        }

        line.erase(line.find_last_not_of(" \t\n\r\f\v") + 1);
        line.erase(0, line.find_first_not_of(" \t\n\r\f\v"));

        if (line.empty())
        {
            continue;
        }

        if (line.find(':') != string::npos)
        {
            size_t colonPos = line.find(':');
            string label = line.substr(0, colonPos);
            label.erase(0, label.find_first_not_of(" \t"));
            label.erase(label.find_last_not_of(" \t") + 1);

            state.labels[label] = instructions.size();

            string instruction = line.substr(colonPos + 1);
            instruction.erase(0, instruction.find_first_not_of(" \t"));

            if (!instruction.empty())
            {
                instructions.push_back(instruction);
            }
            continue;
        }
        else
        {
            instructions.push_back(line);
        }
    }

    return instructions;
}


int main(int argc, char* argv[]) {
    if (argc != 2)
    {
        cout << "Usage: " << argv[0] << " <input_file>" << endl;
        return 1;
    }

    ProcessorState state;
    vector<string> instructions = readInstructionsFromFile(argv[1], state);

    if (instructions.empty())
    {
        cout << "No instructions to execute." << endl;
        return 1;
    }

    while (state.pc < instructions.size())
    {
        executeInstruction(instructions[state.pc], state);
        state.pc++;
    }

    return 0;
}
