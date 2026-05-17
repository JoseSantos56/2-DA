#include "parser.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cctype>

/**
 * @brief Implementação do parser de ficheiros de entrada
 *
 * Implementa os métodos de parsing para ler e processar:
 * - Ficheiros de live ranges
 * - Ficheiros de configuração
 */

/**
 * @details
 * Algoritmo:
 * 1. Abre o ficheiro para leitura
 * 2. Lê linha por linha
 * 3. Ignora linhas vazias e comentários (#)
 * 4. Para cada linha válida:
 *    a. Faz trim para remover espaços
 *    b. Chama parseRangeLine para extrair o LiveRange
 *    c. Adiciona ao vector de resultados
 * 5. Valida que pelo menos um range foi encontrado
 * 6. Retorna vector de ranges
 *
 * @complexity O(N * M) onde N = linhas, M = comprimento médio
 */
std::vector<LiveRange> Parser::parseRanges(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Error to open: " + filename);
    }

    std::vector<LiveRange> ranges;
    std::string line;
    int lineNum = 0;

    while (std::getline(file, line)) {
        lineNum++;
        line = trim(line);

        if (line.empty() || line[0] == '#') continue;

        try {
            ranges.push_back(parseRangeLine(line));
        } catch (const std::exception& e) {
            throw std::runtime_error("Line " + std::to_string(lineNum) + ": " + e.what());
        }
    }

    if (ranges.empty()) {
        throw std::runtime_error("No live range found");
    }

    return ranges;
}

/**
 * @details
 * Formato esperado:
 * @code
 * registers: 4
 * algorithm: basic
 * # ou
 * algorithm: spilling, 2
 * @endcode
 *
 * @complexity O(N) onde N = número de linhas
 */
AllocationConfiguration Parser::parseConfiguration(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Error to open: " + filename);
    }

    AllocationConfiguration config;
    std::string line;

    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;

        size_t colon = line.find(':');
        if (colon == std::string::npos) continue;

        std::string key = trim(line.substr(0, colon));
        std::string value = trim(line.substr(colon + 1));

        if (key == "registers") {
            config.numRegisters = std::stoi(value);
        } else if (key == "algorithm") {
            size_t comma = value.find(',');
            if (comma != std::string::npos) {
                config.algorithm = trim(value.substr(0, comma));
                config.parameter = std::stoi(trim(value.substr(comma + 1)));
            } else {
                config.algorithm = value;
            }
        }
    }

    if (config.numRegisters <= 0) {
        throw std::runtime_error("Invalid number of registers");
    }

    return config;
}

/**
 * @details
 * Processa linha no formato: "variavel: 1+,2,3,4-"
 *
 * Passos:
 * 1. Procura ':' para dividir variável e números
 * 2. Extrai nome da variável (antes do ':')
 * 3. Extrai e processa números (depois do ':')
 * 4. Retorna LiveRange preenchido
 *
 * @complexity O(M + K log K) onde:
 *   - M = comprimento da linha
 *   - K = número de elementos numéricos
 */
LiveRange Parser::parseRangeLine(const std::string& line) {
    size_t colon = line.find(':');
    if (colon == std::string::npos) {
        throw std::runtime_error("Invalid formate (missing ':')");
    }

    LiveRange range;
    range.variable = trim(line.substr(0, colon));

    std::string numbers = trim(line.substr(colon + 1));
    range.lines = parseLineNumbers(numbers, range.startsWithDef, range.endsWithUse);

    return range;
}

/**
 * @details
 * Processa string como "1+,2,3,4-" e extrai:
 * - Vector de números: [1,2,3,4]
 * - hasDef = true (primeiro tem +)
 * - hasUse = true (algum tem -)
 *
 * Algoritmo:
 * 1. Dividir string por vírgulas
 * 2. Para cada token:
 *    a. Verificar marcadores + e -
 *    b. Remover marcador se existir
 *    c. Converter para inteiro
 *    d. Adicionar ao vector
 * 3. Ordenar vector
 * 4. Remover duplicados
 * 5. Retornar
 *
 * @complexity O(K log K) onde K = número de elementos
 */
std::vector<int> Parser::parseLineNumbers(const std::string& str,bool& hasDef, bool& hasUse) {
    std::vector<int> lines;
    std::stringstream ss(str);
    std::string token;
    hasDef = hasUse = false;
    bool first = true;

    while (std::getline(ss, token, ',')) {
        token = trim(token);
        if (token.empty()) continue;

        bool def = (token.back() == '+');
        bool use = (token.back() == '-');

        if (def || use) {
            token = token.substr(0, token.length() - 1);
        }

        lines.push_back(std::stoi(trim(token)));

        if (first && def) hasDef = true;
        if (use) hasUse = true;
        first = false;
    }

    std::sort(lines.begin(), lines.end());
    lines.erase(std::unique(lines.begin(), lines.end()), lines.end());

    return lines;
}

/**
 * @details
 * Remove ' ', '\t', '\r', '\n' das extremidades da string.
 *
 * Algoritmo:
 * 1. Procura primeiro caractere não-espaço (início)
 * 2. Se não encontrar, retorna string vazia
 * 3. Procura último caractere não-espaço (fim)
 * 4. Retorna substring entre primeiro e último
 *
 * @complexity O(N) onde N = comprimento da string
 */
std::string Parser::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, last - first + 1);
}