#ifndef CHIRP_8_CHIP_8_H
#define CHIRP_8_CHIP_8_H

#include <cstdint>
#include <random>
#include <stack>
#include <string>
//todo fix keypad, figure out audio

class CHIP_8 {
    unsigned char RAM[4096];    //memory
    uint16_t PC;                //program counter
    uint16_t I;                 //index register
    std::stack<uint16_t> stack; //stack for subroutine addresses
    uint8_t delayTimer;         //delay timer (some sort of frame counter?)
    uint8_t soundTimer;         //sound timer (computer beeps when this is above 0)
    unsigned char V[16];        //variable registers
    bool display[32][64];       //64x32 display matrix (monochromatic)
    uint16_t keypad;
    uint16_t oldKeypad;
    unsigned char awaitedKey;
    bool oldBehavior;           //Behavior of the 0x8XY6 and 0x8XYE instructions
    std::random_device rd;      //seed source for RNG
    std::mt19937 gen;           //random number generator engine seeded with rd
    std::uniform_int_distribution<> dist;   //Bounds for RNG i guess?

    void jumpProgramCounter(uint16_t address);
    void pushToAddressStack(uint16_t value);
    [[nodiscard]] bool keyIsPressed(uint16_t key) const;
    [[nodiscard]] bool keyWasPressed(uint16_t key) const;
    [[nodiscard]] unsigned char pollReleasedKey() const;
    [[nodiscard]] unsigned char pollPressedKey() const;
    uint16_t popFromAddressStack();
    void clearScreen();
    [[nodiscard]] unsigned char readByte(size_t address) const;
    [[nodiscard]] unsigned short int readInstruction(size_t address) const;
    void writeByte(size_t address, unsigned char value);
    void drawSprite(unsigned char X, unsigned char Y, unsigned char N);
    void getKey(unsigned char X);
    void addWithCarry(unsigned char X, unsigned char Y);
    void subtractXY(unsigned char X, unsigned char Y);
    void subtractYX(unsigned char X, unsigned char Y);
    void shiftRight(unsigned char X, unsigned char Y);
    void shiftLeft(unsigned char X, unsigned char Y);
    void oldJumpWithOffset(uint16_t NNN);
    void jumpWithOffset(unsigned char X, uint8_t NN);
    void addToIndex(unsigned char X);
    void binaryCodedDecimal(unsigned char X);
    void writeRegisters(unsigned char X);
    void readRegisters(unsigned char X);

    static void throwInstructionError(uint8_t instruction);

    public:
    explicit CHIP_8(bool oldBehavior);

    void decrementDelayTimer();
    void decrementSoundTimer();
    void stepForward();
    void importROM(const std::string &path);
    void dumpDisplay() const;
    void dumpRAM() const;

    bool* getDisplay();
    uint8_t* getSoundTimer();

    void setSoundTimer(unsigned char newSoundTimer);
    void toggleKey(uint16_t keycode);
};

#endif //CHIRP_8_CHIP_8_H
