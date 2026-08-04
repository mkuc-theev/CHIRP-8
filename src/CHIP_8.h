#ifndef CHIRP_8_CHIP_8_H
#define CHIRP_8_CHIP_8_H

#include <cstdint>
#include <random>
#include <stack>
#include <string>

class CHIP_8 {
    uint8_t RAM[4096];          //memory
    uint16_t PC;                //program counter
    uint16_t I;                 //index register
    std::stack<uint16_t> stack; //stack for subroutine addresses
    uint8_t delayTimer;         //delay timer (some sort of frame counter?)
    uint8_t soundTimer;         //sound timer (computer beeps when this is above 0)
    uint8_t V[16];              //variable registers
    bool display[32][64];       //64x32 display matrix (monochromatic)
    uint16_t keypad;
    uint16_t oldKeypad;
    uint8_t awaitedKey;
    bool oldBehavior;           //Behavior of the 0x8XY6 and 0x8XYE instructions
    std::random_device rd;      //seed source for RNG
    std::mt19937 gen;           //random number generator engine seeded with rd
    std::uniform_int_distribution<> dist;   //Bounds for RNG i guess?

    void jumpProgramCounter(uint16_t address);
    void pushToAddressStack(uint16_t value);
    [[nodiscard]] bool keyIsPressed(uint16_t key) const;
    [[nodiscard]] bool keyWasPressed(uint16_t key) const;
    [[nodiscard]] uint8_t pollReleasedKey() const;
    [[nodiscard]] uint8_t pollPressedKey() const;
    uint16_t popFromAddressStack();
    void clearScreen();
    [[nodiscard]] uint8_t readByte(uint16_t address) const;
    [[nodiscard]] uint16_t readInstruction(uint16_t address) const;
    void writeByte(uint16_t address, uint8_t value);
    void drawSprite(uint8_t X, uint8_t Y, uint8_t N);
    void getKey(uint8_t X);
    void addWithCarry(uint8_t X, uint8_t Y);
    void subtractXY(uint8_t X, uint8_t Y);
    void subtractYX(uint8_t X, uint8_t Y);
    void shiftRight(uint8_t X, uint8_t Y);
    void shiftLeft(uint8_t X, uint8_t Y);
    void oldJumpWithOffset(uint16_t NNN);
    void jumpWithOffset(uint8_t X, uint8_t NN);
    void addToIndex(uint8_t X);
    void binaryCodedDecimal(uint8_t X);
    void writeRegisters(uint8_t X);
    void readRegisters(uint8_t X);

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

    void setSoundTimer(uint8_t newSoundTimer);
    void toggleKey(uint16_t keycode);
};

#endif //CHIRP_8_CHIP_8_H
