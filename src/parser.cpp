/**
 * @file parser.cpp
 * @brief Implementação do parser
 */

#include "parser.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cctype>

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

std::string Parser::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, last - first + 1);
}