#ifndef CHIRP_8_CHIP_8_H
#define CHIRP_8_CHIP_8_H

#include <cstdint>
#include <stack>

class CHIP_8 {
    unsigned char RAM[4096];    //memory
    uint16_t PC;                //program counter
    uint16_t I;                 //index register
    std::stack<uint16_t> stack; //stack for subroutine addresses
    uint8_t delayTimer;
    uint8_t soundTimer;
    unsigned char V0, V1, V2, V3, V4, V5, V6, V7, V8, V9, VA, VB, VC, VD, VE, VF;   //variable registers
    bool display[32][64];       //64x32 display matrix (monochromatic)
    unsigned char keyEvent;     //Currently pressed key, 0xFF if none

    [[nodiscard]] unsigned char readByte(size_t address) const;
    [[nodiscard]] unsigned short int readInstruction(size_t address) const;
    void writeByte(size_t address, unsigned char value);
    public:
    CHIP_8();

    void loadROM();
    void decrementDelayTimer();
    void decrementSoundTimer();
    void stepForward();

};

#endif //CHIRP_8_CHIP_8_H
