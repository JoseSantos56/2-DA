/**
@file menu.cpp
@brief Implementação do menu interativo e modo batch
*/
#include "menu.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <limits>
#include <algorithm>
#include <map>
#include <set>

// Escrever output no formato especificado
void Menu::writeOutput(const std::string& filename,const std::vector<Web>& webs,const AllocationResult& result) {
    std::ofstream out(filename);
    if (!out.is_open()) {
        std::cerr << "Error: cannot write to " << filename << "\n";
        return;
    }

    // 1. Escrever webs
    out << "webs: " << webs.size() << "\n";
    for (const auto& web : webs) {
        out << "web" << web.id << ": ";
        std::vector<int> sortedLines(web.lines.begin(), web.lines.end());
        std::sort(sortedLines.begin(), sortedLines.end());

        bool first = true;
        for (int line : sortedLines) {
            if (!first) out << ",";
            first = false;

            // Adicionar + se for definição inicial, - se for último uso
            if (web.hasDefAt(line) && line == *web.lines.begin()) {
                out << line << "+";
            } else if (web.hasUseAt(line) && line == *web.lines.rbegin()) {
                out << line << "-";
            } else {
                out << line;
            }
        }
        out << "\n";
    }
    out << "\n";

    // 2. Escrever alocação
    if (result.success && result.registersUsed > 0) {
        out << "registers: " << result.registersUsed << "\n";

        // Agrupar webs por registo
        std::map<int, std::vector<int>> regToWebs;
        for (const auto& [webId, reg] : result.webToRegister) {
            if (reg >= 0) {
                regToWebs[reg].push_back(webId);
            }
        }

        // Escrever assignment
        for (const auto& [reg, webIds] : regToWebs) {
            for (int wid : webIds) {
                out << "r" << reg << ": web" << wid << "\n";
            }
        }

        // Webs em memória (se houver)
        for (int wid : result.spilledWebs) {
            out << "M: web" << wid << "\n";
        }
    } else {
        // Alocação impossível
        out << "registers: 0\n";
        for (const auto& web : webs) {
            out << "M: web" << web.id << "\n";
        }
    }

    out.close();
}

// Processar alocação completa
bool Menu::processAndWrite(const std::string& rangesFile,const std::string& configFile,const std::string& outputFile) {
    try {
        // 1. Parse dos inputs
        auto ranges = Parser::parseRanges(rangesFile);
        auto config = Parser::parseConfiguration(configFile);

        if (!config.isValid()) {
            std::cerr << "Error: invalid configuration\n";
            return false;
        }

        // 2. Construir webs e interference graph
        auto webs = WebBuilder::buildWebs(ranges);
        Graph<int> interferenceGraph;
        WebBuilder::buildInterferenceGraph(webs, interferenceGraph);

        // 3. Executar algoritmo de alocação
        AllocationResult result;

        if (config.algorithm == "basic") {
            result = basicColoring(interferenceGraph, config.numRegisters);
        }
        else if (config.algorithm == "spilling") {
            result = spillingColoring(interferenceGraph,
                                     config.numRegisters,
                                     config.parameter);
        }
        else if (config.algorithm == "splitting") {
            result = splitWeb(interferenceGraph, webs, config.parameter, config.numRegisters);
        }
        else if (config.algorithm == "free") {
            std::vector<int> colors;
            std::vector<bool> spilled;
            bool success = customAllocate(interferenceGraph,
                                         config.numRegisters,
                                         colors,
                                         spilled);
            result.success = success;
            result.registersUsed = 0;
            if (success) {
                std::set<int> used;
                for (int c : colors) if (c >= 0) { used.insert(c); result.registersUsed++; }
                for (size_t i = 0; i < colors.size(); i++) {
                    result.webToRegister[i] = spilled[i] ? -1 : colors[i];
                    if (spilled[i]) result.spilledWebs.push_back((int)i);
                }
            }
        }
        else {
            std::cerr << "Error: unknown algorithm '" << config.algorithm << "'\n";
            return false;
        }

        // 4. Escrever output
        writeOutput(outputFile, webs, result);
        return true;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return false;
    }
}

// Modo batch
void Menu::executeBatch(const std::string& rangesFile,
                       const std::string& registersFile,
                       const std::string& outputFile) {
    processAndWrite(rangesFile, registersFile, outputFile);
}

// Menu interativo
void Menu::showMainMenu() {
    std::cout << "\n=== Register Allocation Tool ===\n";
    std::cout << "1. Run test with predefined files\n";
    std::cout << "2. Run with custom files\n";
    std::cout << "3. View available test datasets\n";
    std::cout << "0. Exit\n";
    std::cout << "Choice: ";
}

void Menu::showAlgorithmMenu() {
    std::cout << "\nSelect algorithm:\n";
    std::cout << "1. basic\n";
    std::cout << "2. spilling,K\n";
    std::cout << "3. splitting,K\n";
    std::cout << "4. free (custom)\n";
    std::cout << "Choice: ";
}

void Menu::runTest(const std::string& rangesFile, const std::string& configFile) {
    std::cout << "\nRunning: " << rangesFile << " + " << configFile << "\n";

    std::string outputFile = "output_test.txt";
    if (processAndWrite(rangesFile, configFile, outputFile)) {
        std::cout << "Success! Output written to " << outputFile << "\n";

        // Mostrar resumo
        std::ifstream out(outputFile);
        std::string line;
        std::cout << "\n--- Output Preview ---\n";
        int shown = 0;
        while (std::getline(out, line) && shown < 15) {
            std::cout << line << "\n";
            shown++;
        }
        if (shown == 15) std::cout << "...\n";
        std::cout << "----------------------\n";
    } else {
        std::cout << "Failed. Check errors above.\n";
    }
}

void Menu::executeMenu() {
    int choice;
    do {
        showMainMenu();
        std::cin >> choice;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (choice == 1) {
            // Testes predefinidos
            std::cout << "\nAvailable tests:\n";
            std::cout << "1. ranges1.txt + registers2.txt (2 regs)\n";
            std::cout << "2. ranges2.txt + registers2.txt (2 regs)\n";
            std::cout << "3. ranges3.txt + registers2.txt (2 regs)\n";
            std::cout << "4. ranges4.txt + registers1.txt (1 reg)\n";
            std::cout << "5. ranges5.txt + registers1.txt (1 reg)\n";
            std::cout << "6. ranges6.txt + registers3.txt (3 regs)\n";
            std::cout << "Choice: ";

            int test;
            std::cin >> test;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            std::string ranges, regs;
            switch(test) {
                case 1: ranges = "ranges1.txt"; regs = "registers2.txt"; break;
                case 2: ranges = "ranges2.txt"; regs = "registers2.txt"; break;
                case 3: ranges = "ranges3.txt"; regs = "registers2.txt"; break;
                case 4: ranges = "ranges4.txt"; regs = "registers1.txt"; break;
                case 5: ranges = "ranges5.txt"; regs = "registers1.txt"; break;
                case 6: ranges = "ranges6.txt"; regs = "registers3.txt"; break;
                default: std::cout << "Invalid test number\n"; continue;
            }
            runTest(ranges, regs);
        }
        else if (choice == 2) {
            // Ficheiros custom
            std::string ranges, config, output;
            std::cout << "Ranges file: "; std::getline(std::cin, ranges);
            std::cout << "Config file: "; std::getline(std::cin, config);
            std::cout << "Output file: "; std::getline(std::cin, output);

            if (processAndWrite(ranges, config, output)) {
                std::cout << "Success! Output: " << output << "\n";
            }
        }
        else if (choice == 3) {
            std::cout << "\nTest datasets:\n";
            std::cout << "- ranges1-6.txt: live ranges examples\n";
            std::cout << "- registers1-3.txt: 1, 2, or 3 registers + algorithm\n";
            std::cout << "\nAlgorithm formats:\n";
            std::cout << "  algorithm: basic\n";
            std::cout << "  algorithm: spilling,2\n";
            std::cout << "  algorithm: splitting,3\n";
            std::cout << "  algorithm: free\n";
        }
        else if (choice != 0) {
            std::cout << "Invalid option\n";
        }
    } while (choice != 0);

    std::cout << "Goodbye!\n";
}