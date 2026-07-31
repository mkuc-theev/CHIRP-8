#include "CHIP_8.h"

#include <algorithm>
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
PC{0x200}, I {0}, delayTimer{0}, soundTimer{0}, V0{0},
V1{0}, V2{0}, V3{0}, V4{0}, V5{0}, V6{0}, V7{0}, V8{0},
V9{0}, VA{0}, VB{0}, VC{0}, VD{0}, VE{0}, VF{0}, keyEvent{0xFF} {
    //zero out the ram so there's no garbage data
    for (size_t i = 0; i < 4096; ++i) {
        this->RAM[i] = 0;
    }

    //initialize display as all black
    for (size_t i = 0; i < 32; ++i) {
        for (size_t j = 0; j < 64; ++j) {
            this->display[i][j] = false;
        }
    }

    //initialize RAM with font data (apparently convention is 0x050 - 0x09F)
    std::copy_n(FONT, sizeof(FONT), this->RAM + 0x050);
}

unsigned char CHIP_8::readByte(size_t address) const {
    if (address <= 0xFFF) {
        return this->RAM[address];
    } else {
        throw std::out_of_range("Memory access out of range (readByte)");
    }
}

unsigned short int CHIP_8::readInstruction(size_t address) const {
    if (address <= 0xFFE) {
        unsigned short int instruction = this->RAM[address] << 8 | this->RAM[address + 1];
        return instruction;
    } else {
        throw std::out_of_range("Memory access out of range (readInstruction)");
    }
}

void CHIP_8::writeByte(size_t address, unsigned char value) {
    if (address <= 0xFFF) {
        this->RAM[address] = value;
    } else {
        throw std::out_of_range("Memory access out of range (writeByte)");
    }
}

void CHIP_8::stepForward() {
    //Fetch the instruction
}
