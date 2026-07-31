#include "Emulator.h"

#include "SFML/Graphics/RectangleShape.hpp"

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
            if (event->is<sf::Event::Closed>())
                window.close();
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
