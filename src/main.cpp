// Lógica principal menu etc..

/**
 * @file main.cpp
 * @brief Ponto de entrada do programa de alocação de registos
 */

#include <iostream>
#include <string>
#include "menu.h"

/**
 * @brief Função principal
 * @param argc Número de argumentos
 * @param argv Array de argumentos
 * @return Código de saída
 */
int main(int argc, char* argv[]) {
    // Modo batch: myProg -b ranges.txt registers.txt allocation.txt
    if (argc == 5 && std::string(argv[1]) == "-b") {
        std::string rangesFile = argv[2];
        std::string registersFile = argv[3];
        std::string outputFile = argv[4];

        return Menu::executeBatch(rangesFile, registersFile, outputFile);
    }
    // Modo interativo
    else if (argc == 1) {
        return Menu::executemenu();
    }
    return 0;
}