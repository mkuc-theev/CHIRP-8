#include "CHIP_8.h"

#include <algorithm>
#include <iomanip>
#include <stdexcept>

constexpr unsigned char FONT[] {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

CHIP_8::CHIP_8() :
PC{0x200}, I {0}, delayTimer{0}, soundTimer{0}, keyEvent{0xFF} {
    //zero out the ram so there's no garbage data
    for (size_t i = 0; i < 4096; ++i) {
        RAM[i] = 0x00;
    }

    //initialize variable registers with zeroes
    for (size_t i = 0; i < 16; ++i) {
        V[i] = 0x00;
    }

    //initialize display as all black
    for (size_t i = 0; i < 32; ++i) {
        for (size_t j = 0; j < 64; ++j) {
            display[i][j] = false;
        }
    }

    //initialize RAM with font data (apparently convention is 0x050 - 0x09F)
    std::copy_n(FONT, sizeof(FONT), RAM + 0x050);
}

unsigned char CHIP_8::readByte(size_t address) const {
    if (address <= 0xFFF) {
        return RAM[address];
    } else {
        throw std::out_of_range("Memory access out of range (readByte)");
    }
}

unsigned short int CHIP_8::readInstruction(size_t address) const {
    if (address <= 0xFFE) {
        unsigned short int instruction = RAM[address] << 8 | RAM[address + 1];
        return instruction;
    } else {
        throw std::out_of_range("Memory access out of range (readInstruction)");
    }
}

void CHIP_8::writeByte(size_t address, unsigned char value) {
    if (address <= 0xFFF) {
        RAM[address] = value;
    } else {
        throw std::out_of_range("Memory access out of range (writeByte)");
    }
}

void CHIP_8::stepForward() {
    //Fetch the instruction and increment the program counter
    unsigned short int instruction = readInstruction(PC);
    PC += 2;

    unsigned char opcode = instruction >> 12;
    unsigned char X = (instruction & 0x0F00) >> 8;
    unsigned char Y = (instruction & 0x00F0) >> 4;
    unsigned char N = (instruction & 0x000F);
    uint8_t NN = instruction & 0x00FF;
    uint16_t NNN = instruction & 0x0FFF;

    switch (opcode) {
        default:
            std::stringstream error;
            error << "Unknown instruction 0x" << std::hex << instruction;
            throw std::out_of_range(error.str());
    }
}
