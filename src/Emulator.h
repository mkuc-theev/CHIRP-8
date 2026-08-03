#ifndef CHIRP_8_EMULATOR_H
#define CHIRP_8_EMULATOR_H

#include "CHIP_8.h"
#include "SFML/Graphics/RenderWindow.hpp"

class Emulator {
    CHIP_8 emulationCore;
    sf::RenderWindow window;
    std::string romPath;
    bool oldBehavior;

    public:
    Emulator(int argc, char* argv[]);

    void startEmulation();
    void drawDisplay(const bool* display);
};

#endif //CHIRP_8_EMULATOR_H
