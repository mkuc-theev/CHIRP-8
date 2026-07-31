#include <iostream>
#include <SFML/Graphics.hpp>
#include "CHIP_8.h"
int main()
{
	try {
		CHIP_8 chip_8{};
	} catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
		exit(1);
	}
	exit(0);
}
