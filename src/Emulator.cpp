#include "Emulator.h"

#include <map>

#include "SFML/Graphics/RectangleShape.hpp"

const std::map<sf::Keyboard::Scan, uint16_t> keyMap {
    {sf::Keyboard::Scan::Num1,  0b0000000000000010},
    {sf::Keyboard::Scan::Num2,  0b0000000000000100},
    {sf::Keyboard::Scan::Num3,  0b0000000000001000},
    {sf::Keyboard::Scan::Num4,  0b0001000000000000},
    {sf::Keyboard::Scan::Q,     0b0000000000010000},
    {sf::Keyboard::Scan::W,     0b0000000000100000},
    {sf::Keyboard::Scan::E,     0b0000000001000000},
    {sf::Keyboard::Scan::R,     0b0010000000000000},
    {sf::Keyboard::Scan::A,     0b0000000010000000},
    {sf::Keyboard::Scan::S,     0b0000000100000000},
    {sf::Keyboard::Scan::D,     0b0000001000000000},
    {sf::Keyboard::Scan::F,     0b0100000000000000},
    {sf::Keyboard::Scan::Z,     0b0000010000000000},
    {sf::Keyboard::Scan::X,     0b0000000000000001},
    {sf::Keyboard::Scan::C,     0b0000100000000000},
    {sf::Keyboard::Scan::V,     0b1000000000000000}
};

constexpr int SCALING_FACTOR = 20.0;

constexpr int INSTRUCTIONS_PER_FRAME = 11;

Emulator::Emulator(int argc, char *argv[]) : emulationCore(false), oldBehavior(false) {
    if (argc < 2) throw std::runtime_error("Please provide rom path");
    romPath = argv[1];

    window.create(sf::VideoMode(
                      {64 * SCALING_FACTOR, 32 * SCALING_FACTOR}),
                  "CHIRP-8"
    );
    window.setKeyRepeatEnabled(false);
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
                    emulationCore.toggleKey(keyMap.at(keyPressed->scancode));
                }
            }
            if (const auto* keyReleased = event->getIf<sf::Event::KeyReleased>()) {
                emulationCore.toggleKey(keyMap.at(keyReleased->scancode));
            }
        }
        emulationCore.decrementDelayTimer();
        emulationCore.decrementSoundTimer();
        for (int i = 0; i < INSTRUCTIONS_PER_FRAME; ++i) {
            emulationCore.stepForward();
        }
        //emulationCore.dumpDisplay();
        window.clear(sf::Color::Black);
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
