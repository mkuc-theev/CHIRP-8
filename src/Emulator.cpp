#include "Emulator.h"

#include <map>

#include "SFML/Graphics/RectangleShape.hpp"

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

Emulator::Emulator(int argc, char *argv[]) : emulationCore(oldBehavior), oldBehavior(false) {
    if (argc < 2) throw std::runtime_error("Please provide rom path");
    romPath = argv[1];

    window.create(sf::VideoMode(
                      {64 * SCALING_FACTOR, 32 * SCALING_FACTOR}),
                  "CHIRP-8"
    );
}

void Emulator::startEmulation() {
    emulationCore.importROM(romPath);

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (keyMap.contains(keyPressed->scancode)) {
                    emulationCore.setKeyEvent(keyMap.at(keyPressed->scancode));
                }
            }
            if (event->is<sf::Event::KeyReleased>()) {
                emulationCore.setKeyEvent(0xFF);
            }
        }

        emulationCore.stepForward();
        //emulationCore.dumpDisplay();
        drawDisplay(emulationCore.getDisplay());
        window.display();
    }
    //emulationCore.dumpRAM();
}

void Emulator::drawDisplay(const bool *display) {
    for (size_t row = 0; row < 32; ++row) {
        for (size_t col = 0; col < 64; ++col) {
            sf::RectangleShape pixel({SCALING_FACTOR, SCALING_FACTOR});
            pixel.setPosition({static_cast<float>(col * SCALING_FACTOR), static_cast<float>(row * SCALING_FACTOR)});
            pixel.setFillColor(sf::Color::White);
            if (display[row * 64 + col]) window.draw(pixel);
        }
    }
}
