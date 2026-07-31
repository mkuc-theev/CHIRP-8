#ifndef CHIRP_8_CHIP_8_H
#define CHIRP_8_CHIP_8_H

#include <cstdint>
#include <stack>

#include "SFML/Graphics/RenderWindow.hpp"


class CHIP_8 {

    unsigned char RAM[4096];    //memory
    uint16_t PC, I;             //program counter and index register
    std::stack<uint16_t> stack; //stack for subroutine addresses
    uint8_t delayTimer;
    uint8_t soundTimer;
    unsigned char V0, V1, V2, V3, V4, V5, V6, V7, V8, V9, VA, VB, VC, VD, VE, VF;   //variable registers
    sf::RenderWindow display;

    public:
    CHIP_8();

};

#endif //CHIRP_8_CHIP_8_H