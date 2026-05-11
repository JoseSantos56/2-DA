/**
 * @file menu.cpp
 */

#include "menu.h"
#include <iostream>
#include <fstream>

int Menu::executeBatch(const std::string& rangesFile, const std::string& registersFile, const std::string& outputFile) {
    try {
        // Parse
        auto ranges = Parser::parseRanges(rangesFile);
        auto registers = Parser::parseConfiguration(registersFile);

    }

    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}

int Menu::executeMenu() {
    // Implementação simplificada
    std::cout << "interactive mode (to implement)" << std::endl;
    return 0;
}
