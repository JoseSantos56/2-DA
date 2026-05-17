/**
@file menu.h
@brief Interface do menu interativo e modo batch
*/
#ifndef DA_SECOND_PROJECT_MENU_H
#define DA_SECOND_PROJECT_MENU_H

#include "parser.h"
#include "web_builder.h"
#include "coloring.h"
#include "splitting.h"
#include "customAllocator.h"
#include <string>
#include <vector>

class Menu {
public:
    /**
    @brief Executa o menu interativo
    */
    void executeMenu();

    /**
    @brief Executa o programa em modo batch
    @param rangesFile Ficheiro de live ranges
    @param registersFile Ficheiro de configuração de registos
    @param outputFile Ficheiro de output para a alocação
    */
    void executeBatch(const std::string& rangesFile,
                     const std::string& registersFile,
                     const std::string& outputFile);

private:
    /**
    @brief Processa a alocação e escreve o resultado no ficheiro
    */
    bool processAndWrite(const std::string& rangesFile,
                        const std::string& configFile,
                        const std::string& outputFile);

    /**
    @brief Escreve o resultado da alocação no formato especificado
    */
    void writeOutput(const std::string& filename,
                    const std::vector<Web>& webs,
                    const AllocationResult& result);

    /**
    @brief Mostra o menu principal interativo
    */
    void showMainMenu();

    /**
    @brief Mostra submenu de algoritmos
    */
    void showAlgorithmMenu();

    /**
    @brief Executa um teste com os parâmetros fornecidos
    */
    void runTest(const std::string& rangesFile,
                const std::string& configFile);
};

#endif /* DA_SECOND_PROJECT_MENU_H */