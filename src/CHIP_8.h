#ifndef CHIRP_8_CHIP_8_H
#define CHIRP_8_CHIP_8_H

#include <cstdint>
#include <stack>
#include <string>

class CHIP_8 {
    unsigned char RAM[4096];    //memory
    uint16_t PC;                //program counter
    uint16_t I;                 //index register
    std::stack<uint16_t> stack; //stack for subroutine addresses
    uint8_t delayTimer;         //delay timer (some sort of frame counter?)
    uint8_t soundTimer;         //sound timer (computer beeps when this is above 0)
    unsigned char V[16];        //variable registers
    bool display[32][64];       //64x32 display matrix (monochromatic)
    unsigned char keyEvent;     //Currently pressed key, 0xFF if none

    void moveProgramCounter(uint16_t address);
    void pushToAddressStack(uint16_t value);
    uint16_t popFromAddressStack();
    void clearScreen();
    [[nodiscard]] unsigned char readByte(size_t address) const;
    [[nodiscard]] unsigned short int readInstruction(size_t address) const;
    void writeByte(size_t address, unsigned char value);

    public:
    CHIP_8();

    void decrementDelayTimer();
    void decrementSoundTimer();
    void stepForward();
    void importROM(const std::string &path);
    void dumpDisplay() const;
    void dumpRAM() const;

    bool* getDisplay();
};

#endif //CHIRP_8_CHIP_8_H
