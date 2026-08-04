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

constexpr uint16_t FONT_OFFSET = 0x50;
constexpr uint16_t ROM_OFFSET = 0x200;

CHIP_8::CHIP_8(bool oldBehavior) :
RAM{}, PC{ROM_OFFSET}, I {0}, delayTimer{0}, soundTimer{0},
V{}, display{}, keypad{0}, oldKeypad{0}, awaitedKey {0xFF},
oldBehavior {false}, gen(rd()), dist{0x00, 0xFF} {
    //initialize RAM with font data (apparently convention is 0x050 - 0x09F)
    std::copy_n(FONT, sizeof(FONT), RAM + FONT_OFFSET);
}

void CHIP_8::decrementDelayTimer() {
    if (delayTimer > 0) delayTimer--;
}

void CHIP_8::decrementSoundTimer() {
    if (soundTimer > 0) soundTimer--;
}


void CHIP_8::jumpProgramCounter(const uint16_t address) {
    if (address > 0xFFF) throw std::out_of_range("Program counter moved out of range");

    PC = address;
}

void CHIP_8::pushToAddressStack(const uint16_t value) {
    if (value > 0xFFF) throw std::out_of_range("Out-of-range address pushed onto stack");

    stack.push(value);
}

bool CHIP_8::keyIsPressed(const uint16_t key) const {
    if (key > 0xF) throw std::out_of_range("Keypad button code out of range");

    return keypad >> key & 1;
}

bool CHIP_8::keyWasPressed(const uint16_t key) const {
    if (key > 0xF) throw std::out_of_range("Keypad button code out of range");

    return oldKeypad >> key & 1;
}

unsigned char CHIP_8::pollReleasedKey() const {
    return std::countr_zero(static_cast<uint16_t>((oldKeypad ^ keypad) & oldKeypad));
}

unsigned char CHIP_8::pollPressedKey() const {
    return std::countr_zero(static_cast<uint16_t>((oldKeypad ^ keypad) & keypad));
}

uint16_t CHIP_8::popFromAddressStack() {
    if (stack.empty()) throw std::logic_error("Stack is empty");

    const uint16_t value = stack.top();
    stack.pop();
    return value;
}

void CHIP_8::clearScreen() {
    for (auto & i : display) {
        for (bool & j : i) {
            j = false;
        }
    }
}

unsigned char CHIP_8::readByte(const uint16_t address) const {
    if (address > 0xFFF) throw std::out_of_range("Memory access out of range (readByte)");

    return RAM[address];
}

unsigned short int CHIP_8::readInstruction(const uint16_t address) const {
    if (address > 0xFFE) throw std::out_of_range("Memory access out of range (readInstruction)");

    const unsigned short int instruction = RAM[address] << 8 | RAM[address + 1];
    return instruction;
}

void CHIP_8::writeByte(const uint16_t address, const unsigned char value) {
    if (address > 0xFFF) throw std::out_of_range("Memory access out of range (writeByte)");

    RAM[address] = value;
}

void CHIP_8::drawSprite(const unsigned char X, const unsigned char Y, const unsigned char N) {
    const unsigned char x = V[X] % 64;
    const unsigned char y = V[Y] % 32;
    V[0xF] = 0;

    for (size_t row = 0; row < N; ++row) {
        if (row + y > 31) break;
        unsigned char spriteRow = readByte(I + row);
        for (size_t column = 0; column < 8; ++column) {
            if (column + x > 63) break;
            if (spriteRow & 0b10000000) {
                if (display[row + y][column + x] && !V[0xF]) V[0xF] = 1;
                display[row + y][column + x] = !display[row + y][column + x];
            }
            spriteRow <<= 1;
        }
    }
}

void CHIP_8::getKey(const unsigned char X) {
    if (awaitedKey > 0xF) {
        awaitedKey = pollPressedKey();
    } else if (const unsigned char key = pollReleasedKey(); key == awaitedKey)  {
        V[X] = key;
        awaitedKey = 0xFF;
        return;
    }
    jumpProgramCounter(PC - 2);
}

void CHIP_8::addWithCarry(unsigned char X, unsigned char Y) {
    const bool carry = V[X] > 0xFF - V[Y];
    V[X] += V[Y];
    V[0xF] = carry;
}

void CHIP_8::subtractXY(unsigned char X, unsigned char Y) {
    const bool carry = V[X] >= V[Y];
    V[X] -= V[Y];
    V[0xF] = carry;
}

void CHIP_8::subtractYX(unsigned char X, unsigned char Y) {
    const bool carry = (V[Y] >= V[X]);
    V[X] = V[Y] - V[X];
    V[0xF] = carry;
}

void CHIP_8::shiftRight(unsigned char X, unsigned char Y) {
    if (oldBehavior) V[X] = V[Y];
    const bool ejectedBit = V[X] & 0x1;
    V[X] >>= 1;
    V[0xF] = ejectedBit;
}

void CHIP_8::shiftLeft(unsigned char X, unsigned char Y) {
    if (oldBehavior) V[X] = V[Y];
    const bool ejectedBit = V[X] >> 7;
    V[X] <<= 1;
    V[0xF] = ejectedBit;
}

void CHIP_8::oldJumpWithOffset(uint16_t NNN) {
        jumpProgramCounter(NNN + V[0]);
}

void CHIP_8::jumpWithOffset(unsigned char X, uint8_t NN) {
        jumpProgramCounter(NN + V[X]);
}

void CHIP_8::addToIndex(unsigned char X) {
    const bool carry = !oldBehavior && V[X] > 0xFFF - I;
    I += V[X];
    V[0xF] = carry;
}

void CHIP_8::binaryCodedDecimal(unsigned char X) {
    writeByte(I, V[X]/100);
    writeByte(I + 1, (V[X] / 10) % 10);
    writeByte(I + 2, V[X] % 10);
}

void CHIP_8::writeRegisters(unsigned char X) {
    for (size_t n = 0; n <= X; ++n) {
        if (oldBehavior) {
            writeByte(I++, V[n]);
        } else {
            writeByte(I + n, V[n]);
        }
    }
}

void CHIP_8::readRegisters(unsigned char X) {
    for (size_t n = 0; n <= X; ++n) {
        if (oldBehavior) {
            V[n] = readByte(I++);
        } else {
            V[n] = readByte(I + n);
        }
    }
}

void CHIP_8::throwInstructionError(const uint8_t instruction) {
    std::stringstream error;
    error << "Unknown instruction 0x" << std::hex << instruction;
    throw std::out_of_range(error.str());
}

void CHIP_8::stepForward() {
    //Fetch the instruction and increment the program counter
    const uint16_t instruction = readInstruction(PC);
    jumpProgramCounter(PC + 2);

    //Catch NOP
    if (instruction == 0x0000) return;

    //Extract important bytes using bit masks and shifts
    const unsigned char opcode = instruction >> 12;
    const unsigned char X = (instruction & 0x0F00) >> 8;
    const unsigned char Y = (instruction & 0x00F0) >> 4;
    const unsigned char N = (instruction & 0x000F);
    const uint8_t NN = instruction & 0x00FF;
    const uint16_t NNN = instruction & 0x0FFF;

    //Decode and execute opcode
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
                    throwInstructionError(instruction);
            }
            break;
        case 0x1:           //0x1NNN Jump
            jumpProgramCounter(NNN);
            break;
        case 0x2:           //0x2NNN Call Subroutine
            pushToAddressStack(PC);
            jumpProgramCounter(NNN);
            break;
        case 0x3:           //0x3XNN Branch if VX == NN
            if (V[X] == NN) jumpProgramCounter(PC + 2);
            break;
        case 0x4:           //0x4XNN Branch if VX != NN
            if (V[X] != NN) jumpProgramCounter(PC + 2);
            break;
        case 0x5:           //0x5XY0 Branch if VX == VY
            if (V[X] == V[Y]) jumpProgramCounter(PC + 2);
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
                    addWithCarry(X, Y);
                    break;
                case 0x5:   //0x8XY5 Subtract VX - VY
                    subtractXY(X, Y);
                    break;
                case 0x6:   //0x8XY6 Shift Right
                    shiftRight(X, Y);
                    break;
                case 0x7:   //0x8XY7 Subtract VY - VX
                    subtractYX(X, Y);
                    break;
                case 0xE:   //0x8XYE Shift Left
                    shiftLeft(X, Y);
                    break;
                default:
                    throwInstructionError(instruction);
            }
            break;
        case 0x9:           //0x9XY0 Branch if VX != VY
            if (V[X] != V[Y]) jumpProgramCounter(PC + 2);
            break;
        case 0xA:           //0xANNN Set Index
            I = NNN;
            break;
        case 0xB:           //0xBNNN/0xBXNN Jump with Offset
            oldBehavior ? oldJumpWithOffset(NNN) : jumpWithOffset(X, NN);
            break;
        case 0xC:           //0xCXNN Random
            V[X] = dist(gen) & NN;
            break;
        case 0xD:           //0xDXYN Display
            drawSprite(X, Y, N);
            break;
        case 0xE:           //Skip if key
            switch (NN) {
                case 0x9E:  //0xEX9E Skip if key pressed
                    if (keyIsPressed(V[X])) jumpProgramCounter(PC + 2);
                    break;
                case 0xA1:   //0xEXA1 Skip if key not pressed
                    if (!keyIsPressed(V[X])) jumpProgramCounter(PC + 2);
                    break;
                default:
                    throwInstructionError(instruction);
            }
            break;
        case 0xF:
            switch (NN) {
                case 0x07:  //0xFX07 VX = Delay timer
                    V[X] = delayTimer;
                    break;
                case 0x15:  //0xFX15 Delay timer = VX
                    delayTimer = V[X];
                    break;
                case 0x18:  //0xFX17 Sound timer = VX
                    soundTimer = V[X];
                    break;
                case 0x1E:  //0xFX1E Add to index
                    addToIndex(X);
                    break;
                case 0x0A:  //0xFX0A Get key
                    getKey(X);
                    break;
                case 0x29:  //0xFX29 Font character
                    I = (V[X] & 0x0F) * 5 + FONT_OFFSET;
                    break;
                case 0x33:  //0xFX33 Binary-coded decimal conversion
                    binaryCodedDecimal(X);
                    break;
                case 0x55:  //0xFX55 Store register to memory
                    writeRegisters(X);
                    break;
                case 0x65:  //0xFX65 Load register from memory
                    readRegisters(X);
                    break;
                default:
                    throwInstructionError(instruction);
            }
            break;
        default:
            throwInstructionError(instruction);
    }
}

void CHIP_8::importROM(const std::string &path) {
    const std::filesystem::path romPath{path};

    const auto length = std::filesystem::file_size(romPath);
    if (length > 0xFFF - ROM_OFFSET + 1) throw std::out_of_range("ROM is too large");

    std::ifstream rom(path, std::ios_base::binary);
    if (!rom.is_open()) throw std::runtime_error("Could not open ROM file");

    rom.read(reinterpret_cast<char*>(RAM + ROM_OFFSET), static_cast<std::streamsize>(length));

    rom.close();
}

void CHIP_8::dumpDisplay() const {
    for (const auto & row : display) {
        for (const bool column : row) {
            std::cout << column;
        }
        std::cout << std::endl;
    }
}

void CHIP_8::dumpRAM() const {
    for (uint16_t i = 0x000; i <= 0xFFF; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(readByte(i)) << " ";
    }
}

bool * CHIP_8::getDisplay() {
    return *display;
}


uint8_t * CHIP_8::getSoundTimer() {
    return &soundTimer;
}


void CHIP_8::setSoundTimer(const unsigned char newSoundTimer) {
    soundTimer = newSoundTimer;
}

void CHIP_8::toggleKey(const uint16_t keycode) {
    oldKeypad = keypad;
    keypad ^= keycode;
}