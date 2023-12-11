#include <iostream>
#include <cstdint>
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>
#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#endif

using namespace std;

#include "instructions.h"

constexpr uint32_t program_len = 32768; // 64 KiB
uint32_t program[program_len];

constexpr uint32_t missing_label_address = 0x80000000;

bool is_instruction_type(const string& type) {
    if (type.size() == 2 && isdigit(type[1])) {
        return type[0] >= 'A' && type[0] <= 'D';
    }
    if (type.size() == 3 && isdigit(type[1]) && isdigit(type[2])) {
        const string num = type.substr(1);
        return stoi(num) <= 15 && type[0] >= 'A' && type[0] <= 'D';
    }
    return false;
}

uint32_t to_instruction(const string& type, const uint32_t argument) {
    uint32_t instruction = 0;
    if(type[0] == 'A') {
        instruction = GROUP_A << 16;
    } else if(type[0] == 'B') {
        instruction = GROUP_B << 16;
    } else if(type[0] == 'C') {
        instruction = GROUP_C << 16;
    } else if(type[0] == 'D') {
        instruction = GROUP_D << 16;
    }
    const string num = type.substr(1);
    instruction |= stoi(num) << 16;
    instruction |= argument;
    return instruction;
}

void remove_comments(string &inputString) {
    size_t semicolonPos = inputString.find(';');
    if (semicolonPos != string::npos) {
        inputString.erase(semicolonPos);
    }
}

uint32_t to_argument(const std::string &str, bool *to_arg) {
    *to_arg = true;
    if (str.size() > 2 && str.substr(0, 2) == "0b") {
        istringstream iss(str.substr(2));
        uint32_t result = 0;
        char ch;
        while (iss >> ch) {
            if (ch == '0' || ch == '1') {
                result = (result << 1) + (ch - '0');
            } else {
                *to_arg = false;
                return 0;
            }
        }
        *to_arg = true;
        return result;
    }
    if (str.size() > 2 && str.substr(0, 2) == "0x") {
        std::istringstream iss(str.substr(2));
        uint32_t result;
        if (!(iss >> std::hex >> result) || iss.peek() != EOF) {
            *to_arg = false;
        }
        return result;
    }
    std::istringstream iss(str);
    uint32_t result;
    if (!(iss >> result) || iss.peek() != EOF) {
        *to_arg = false;
    }
    return result;
}

struct label {
  string name;
  bool defined;
  uint32_t address;
};

vector<label> labels;
uint32_t pc;
string instructionType;
bool inInstruction = false;

size_t put_label(const label &label) {
    labels.push_back(label);
    return labels.size()-1;
}

int find_label(const string &name) {
    for(size_t i=0; i<labels.size(); i++) {
        if(labels[i].name == name) {
            return (int)i;
        }
    }
    return -1;
}

void set_label(const string &name, const uint32_t address) {
    int labelIdx = find_label(name);
    if(labelIdx == -1) {
        put_label((label){name, true, address});
    } else {
        labels[labelIdx].defined = true;
        labels[labelIdx].address = address;
    }
}

void append_instruction(uint32_t arg) {
    program[pc++] = to_instruction(instructionType, arg);
}

void append_literal(uint32_t arg) {
    program[pc++] = arg;
}

void process_label(string label) {
    label.pop_back(); // Remove ":" from the end
    if (label.empty() || label.find(':') != string::npos) {
        cerr << "Invalid name for label: " + label;
        exit(1);
    }
    if (is_instruction_type(label)) {
        cerr << "Found unexpected label with instruction name: " + label;
        exit(1);
    }
    set_label(label, pc);
}

bool process_arg(const string &token, uint32_t arg) {
    if(!inInstruction) {
        cerr << "Found unexpected argument: " + token;
        exit(1);
    }
    if(arg > 0xFFFF) {
        cerr << "Argument does not fit in 16 bits: " + arg;
        exit(1);
    }
    inInstruction = false;
    append_instruction(arg);
    return false;
}

void process_instruction(const string &instruction) {
    if(inInstruction) { // Previous instruction had no arguments
        append_instruction(0);
    }
    instructionType = instruction;
    inInstruction = true;
}

void process_reference_to_label(const string &label) {
    inInstruction = false;
    int labelIdx = find_label(label);
    if(labelIdx == -1) {
        labelIdx = put_label({label, false, 0});
    }
    if(labelIdx > 0xFFFF) {
        cerr << "Label index does not fit in 16 bits: " + label;
        exit(1);
    }
    append_instruction(missing_label_address | labelIdx);
}

uint32_t load_program_from_stdin(uint16_t startAddr) {
    pc = startAddr;

    string line;
    while(getline(cin, line) && pc <= program_len) {
        remove_comments(line);
        if (line.empty()) continue; // skip empty lines

        istringstream lineStream(line);
        string token;
        while(lineStream >> token) {
            if(token.find(':') != string::npos) {
                process_label(token);
            } else {
                if(token[0] == '\'') {
                    if(inInstruction) {
                        cerr << "Literal cannot be inserted in the middle of an instruction";
                        exit(1);
                    }
                    bool is_literal;
                    uint32_t literal = to_argument(token.substr(1), &is_literal);
                    if(!is_literal) {
                        cerr << "Literal is invalid " << token;
                        exit(1);
                    }
                    append_literal(literal);
                } else {
                    bool isArg;
                    uint32_t arg = to_argument(token, &isArg);
                    if(isArg) {
                        process_arg(token, arg);
                    } else if(is_instruction_type(token)) {
                        process_instruction(token);
                    } else {
                        process_reference_to_label(token);
                    }
                }
            }
        }
        if(inInstruction) {
            append_instruction(0); // Dummy "0" argument if last opcode doesn't have one
        }
    }

    return pc;
}

void update_symbols() {
    for (const auto& label : labels) {
        if(!label.defined) {
            cerr << "Missing definition for " << label.name << endl;
            exit(1);
        }
    }

    for (size_t i=0; i<pc; i++) {
        if(program[i] & missing_label_address) { // Address requiring definition
            uint16_t labelidx = program[i] & 0xFFFF;
            program[i] = (program[i] & 0x00FF0000) | labels[labelidx].address;
        }
    }
}

void write_program_binary_to_stdout(uint16_t baseAddr, uint16_t programLength) {
    for (uint16_t addr = baseAddr; addr < baseAddr + programLength; addr++) {
        uint32_t word = program[addr];

        uint8_t b0 = (word >>  0) & 0xFF;
        uint8_t b1 = (word >>  8) & 0xFF;
        uint8_t b2 = (word >> 16) & 0xFF;

        cout << b0 << b1 << b2;
    }
}

int main() {
    #ifdef _WIN32
    // Avoids unwanted \r issue when writing binary file to stdout
    setmode(fileno(stdout),O_BINARY);
    #endif
    load_program_from_stdin(0);
    update_symbols();
    write_program_binary_to_stdout(0, pc);
    return 0;
}
