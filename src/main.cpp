#include <iostream>
#include <SFML/Graphics.hpp>
#include "Emulator.h"

int main(const int argc, char** argv)
{
	try {
		Emulator emulator(argc, argv);
		emulator.startEmulation();
	} catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
		exit(1);
	}
	exit(0);
}
