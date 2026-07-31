#include "CHIP_8.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <random>

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

CHIP_8::CHIP_8(bool oldBehavior) :
PC{0x200}, I {0}, delayTimer{0}, soundTimer{0}, keyEvent{0xFF},
oldBehavior {oldBehavior}, gen(rd()), dist{0x00, 0xFF} {
    //zero out the ram so there's no garbage data
    for (size_t i = 0; i < 4096; ++i) {
        RAM[i] = 0x00;
    }

    //initialize variable registers with zeroes
    for (size_t i = 0; i < 16; ++i) {
        V[i] = 0x00;
    }

    //Screen all black :thumbsup:
    clearScreen();

    //initialize RAM with font data (apparently convention is 0x050 - 0x09F)
    std::copy_n(FONT, sizeof(FONT), RAM + 0x050);
}

void CHIP_8::decrementDelayTimer() {
    if (delayTimer > 0) delayTimer--;
}

void CHIP_8::decrementSoundTimer() {
    if (soundTimer > 0) soundTimer--;
}


void CHIP_8::moveProgramCounter(uint16_t address) {
    if (address > 0xFFF) throw std::out_of_range("Program counter moved out of range");

    PC = address;
}

void CHIP_8::pushToAddressStack(uint16_t value) {
    if (value > 0xFFF) throw std::out_of_range("Out-of-range address pushed onto stack");

    stack.push(value);
}

uint16_t CHIP_8::popFromAddressStack() {
    if (stack.empty()) throw std::logic_error("Stack is empty");

    uint16_t value = stack.top();
    stack.pop();
    return value;
}

void CHIP_8::clearScreen() {
    for (size_t i = 0; i < 32; ++i) {
        for (size_t j = 0; j < 64; ++j) {
            display[i][j] = false;
        }
    }
}

unsigned char CHIP_8::readByte(size_t address) const {
    if (address > 0xFFF) throw std::out_of_range("Memory access out of range (readByte)");

    return RAM[address];
}

unsigned short int CHIP_8::readInstruction(size_t address) const {
    if (address > 0xFFE) throw std::out_of_range("Memory access out of range (readInstruction)");

    unsigned short int instruction = RAM[address] << 8 | RAM[address + 1];
    return instruction;
}

void CHIP_8::writeByte(size_t address, unsigned char value) {
    if (address > 0xFFF) throw std::out_of_range("Memory access out of range (writeByte)");

    RAM[address] = value;
}

void CHIP_8::stepForward() {
    //Fetch the instruction and increment the program counter
    unsigned short int instruction = readInstruction(PC);
    PC += 2;

    if (instruction == 0x0000) return;

    unsigned char opcode = instruction >> 12;
    unsigned char X = (instruction & 0x0F00) >> 8;
    unsigned char Y = (instruction & 0x00F0) >> 4;
    unsigned char N = (instruction & 0x000F);
    uint8_t NN = instruction & 0x00FF;
    uint16_t NNN = instruction & 0x0FFF;

    unsigned char x;
    unsigned char y;

    //help
    switch (opcode) {
        case 0x0:
            switch (NNN) {
                case 0x0E0:   //0x00E0 Clear Screen
                    clearScreen();
                    break;
                case 0x0EE:   //0x00EE Return from Subroutine
                    PC = popFromAddressStack();
                    break;
                default:
                    std::stringstream error;
                    error << "Unknown instruction 0x" << std::hex << instruction;
                    throw std::out_of_range(error.str());
            }
            break;
        case 0x1:           //0x1NNN Jump
            moveProgramCounter(NNN);
            break;
        case 0x2:           //0x2NNN Call Subroutine
            pushToAddressStack(PC);
            moveProgramCounter(NNN);
            break;
        case 0x3:           //0x3XNN Branch if VX == NN
            if (V[X] == NN) PC += 2;
            break;
        case 0x4:           //0x4XNN Branch if VX != NN
            if (V[X] != NN) PC += 2;
            break;
        case 0x5:           //0x5XY0 Branch if VX == VY
            if (V[X] == V[Y]) PC += 2;
            break;
        case 0x6:           //0x6XNN Set
            V[X] = NN;
            break;
        case 0x7:           //0x7XNN Add
            V[X] += NN;
            break;
        case 0x8:           //Logical and arithmetic instructions
            switch (N) {
                case 0x0:   //0x8XY0 Set
                    V[X] = V[Y];
                    break;
                case 0x1:   //0x8XY1 Binary OR
                    V[X] |= V[Y];
                    break;
                case 0x2:   //0x8XY2 Binary AND
                    V[X] &= V[Y];
                    break;
                case 0x3:   //0x8XY3 Logical XOR
                    V[X] ^= V[Y];
                    break;
                case 0x4:   //0x8XY4 Add (with carry flag)
                    if (V[X] > 0xFF - V[Y]) V[0xF] = 1;
                    V[X] += V[Y];
                    break;
                case 0x5:   //0x8XY5 Subtract VX - VY
                    if (V[X] >= V[Y]) V[0xF] = 1;
                    V[X] -= V[Y];
                    break;
                case 0x6:   //0x8XY6 Shift Right
                    if (oldBehavior) V[X] = V[Y];
                    V[0xF] = V[X] & 0x1;
                    V[X] >>= 1;
                    break;
                case 0x7:   //0x8XY7 Subtract VY - VX
                    if (V[Y] >= V[X]) V[0xF] = 1;
                    V[X] = V[Y] - V[X];
                    break;
                case 0xE:   //0x8XY6 Shift Left
                    if (oldBehavior) V[X] = V[Y];
                    V[0xF] = V[X] >> 7;
                    V[X] <<= 1;
                    break;
                default:
                    std::stringstream error;
                    error << "Unknown instruction 0x" << std::hex << instruction;
                    throw std::out_of_range(error.str());
            }
            break;
        case 0x9:           //0x9XY0 Branch if VX != VY
            if (V[X] != V[Y]) PC += 2;
            break;
        case 0xA:           //0xANNN Set Index
            I = NNN;
            break;
        case 0xB:           //0xBNNN/0xBXNN Jump with Offset
            if (oldBehavior) moveProgramCounter(NNN + V[0]);
            moveProgramCounter(NNN + V[X]);
            break;
        case 0xC:           //0xCXNN Random
            V[X] = dist(gen) & NN;
            break;
        case 0xD:           //0xDXYN Display
            x = V[X] % 64;
            y = V[Y] % 32;
            V[0xF] = 0x00;
            for (size_t row = 0; row < N; ++row) {
                if (row + y > 31) break;
                unsigned char spriteRow = RAM[I + row];
                for (size_t column = 0; column < 8; ++column) {
                    if (column + x > 63) break;
                    if (spriteRow & 0x80) {
                        if (display[row + y][column + x] && V[0xF] == 0x00) V[0xF] = 0x1;
                        display[row + y][column + x] = !display[row + y][column + x];
                    }
                    spriteRow <<= 1;
                }
            }
            break;
        case 0xE:           //Skip if key
            switch (NN) {
                case 0x9E:  //0xEX9E Skip if key pressed
                    if (keyEvent == V[X]) PC += 2;
                    break;
                case 0xA1:   //0xEXA1 Skip if key not pressed
                    if (keyEvent != V[X]) PC += 2;
                    break;
                default:
                    std::stringstream error;
                    error << "Unknown instruction 0x" << std::hex << instruction;
                    throw std::out_of_range(error.str());
            }
            break;
        default:
            std::stringstream error;
            error << "Unknown instruction 0x" << std::hex << instruction;
            throw std::out_of_range(error.str());
    }
}

void CHIP_8::importROM(const std::string &path) {
    std::filesystem::path romPath{path};

    auto length = std::filesystem::file_size(romPath);
    if (length > 3584) throw std::out_of_range("ROM is too large");   //ROM injected at 0x200, can't exceed 3584B

    std::ifstream rom(path, std::ios_base::binary);
    if (!rom.is_open()) throw std::runtime_error("Could not open ROM file");

    rom.read(reinterpret_cast<char*>(RAM + 0x200), length);

    rom.close();
}

void CHIP_8::dumpDisplay() const {
    for (size_t row = 0; row < 32; ++row) {
        for (size_t column = 0; column < 64; ++column) {
            std::cout << display[row][column];
        }
        std::cout << std::endl;
    }
}

void CHIP_8::dumpRAM() const {
    for (size_t i = 0x000; i <= 0xFFF; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(RAM[i]) << " ";
    }
}

bool * CHIP_8::getDisplay() {
    return *display;
}
