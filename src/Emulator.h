#ifndef CHIRP_8_EMULATOR_H
#define CHIRP_8_EMULATOR_H

#include <map>

#include "CHIP_8.h"
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Window/Keyboard.hpp"

const std::map<sf::Keyboard::Scan, unsigned char> keyMap {
    {sf::Keyboard::Scan::Num1, 0x0},
    {sf::Keyboard::Scan::Num2, 0x1},
    {sf::Keyboard::Scan::Num3, 0x2},
    {sf::Keyboard::Scan::Num4, 0x3},
    {sf::Keyboard::Scan::Q, 0x4},
    {sf::Keyboard::Scan::W, 0x5},
    {sf::Keyboard::Scan::E, 0x6},
    {sf::Keyboard::Scan::R, 0x7},
    {sf::Keyboard::Scan::A, 0x8},
    {sf::Keyboard::Scan::S, 0x9},
    {sf::Keyboard::Scan::D, 0xA},
    {sf::Keyboard::Scan::F, 0xB},
    {sf::Keyboard::Scan::Z, 0xC},
    {sf::Keyboard::Scan::X, 0xD},
    {sf::Keyboard::Scan::C, 0xE},
    {sf::Keyboard::Scan::V, 0xF}
};

constexpr int SCALING_FACTOR = 20.0;

class Emulator {
    CHIP_8 emulationCore;
    sf::RenderWindow window;
    std::string romPath;

    public:
    Emulator(int argc, char* argv[]);

    void startEmulation();
    void drawDisplay(const bool* display);
};

#endif //CHIRP_8_EMULATOR_H
