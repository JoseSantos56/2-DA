/**
 * @file parser.h
 * @brief Estruturas de dados e parsing de ficheiros de entrada
 */

#ifndef DA_SECOND_PROJECT_PARSER_H
#define DA_SECOND_PROJECT_PARSER_H

#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include <map>

/**
 * @brief Intervalo de vida de uma variável
 */
struct LiveRange {
    std::string variable;
    std::vector<int> lines;
    bool startsWithDef;
    bool endsWithUse;

    LiveRange() : startsWithDef(false), endsWithUse(false) {}

    int getStart() const { return lines.empty() ? -1 : lines.front(); }
    int getEnd() const { return lines.empty() ? -1 : lines.back(); }
    size_t size() const { return lines.size(); }
};


/**
 * @brief Web (união de live ranges)
 */
struct Web {
    int id;
    std::string variable;
    std::set<int> lines;
    std::set<int> defsLines;
    std::set<int> usesLines;

    Web() : id(-1) {}
    Web(int webId, const std::string& var) : id(webId), variable(var) {}

    /**
     * @brief Adiciona linhas de um live range ao web
     * @param range Live range a adicionar
     */
    void addRange(const LiveRange& range) {
        lines.insert(range.lines.begin(), range.lines.end());

        // Registar definições e usos
        if (range.startsWithDef && !range.lines.empty()) {
            defsLines.insert(range.lines.front());
        }
        if (range.endsWithUse && !range.lines.empty()) {
            usesLines.insert(range.lines.back());
        }
    }

    /**
     * @brief Verifica se tem definição numa linha específica
     */
    bool hasDefAt(int line) const {
        return defsLines.find(line) != defsLines.end();
    }

    /**
     * @brief Verifica se tem uso numa linha específica
     */
    bool hasUseAt(int line) const {
        return usesLines.find(line) != usesLines.end();
    }

    /**
     * @brief Verifica interferência com outro web
     */
    bool interferesWith(const Web& other) const {
        if (lines.empty() || other.lines.empty()) return false;

        // Otimização: verificar limites
        if (*lines.rbegin() < *other.lines.begin() || *other.lines.rbegin() < *lines.begin()) {
            return false;
        }

        // Verificar interseção
        std::vector<int> intersection;
        std::set_intersection(lines.begin(), lines.end(), other.lines.begin(), other.lines.end(), std::back_inserter(intersection));

        return !intersection.empty();
    }

    size_t size() const { return lines.size(); }
};

/**
 * @brief Configuração de alocação
 */
struct AllocationConfiguration {
    int numRegisters;
    std::string algorithm;  // "basic", "spilling", "splitting", "custom"
    int parameter;

    AllocationConfiguration() : numRegisters(0), algorithm("basic"), parameter(0) {}
    bool isValid() const { return numRegisters > 0; }
};


/**
 * @brief Resultado da alocação de registos
 */
struct AllocationResult {
    bool success;                      // true se alocação bem-sucedida
    int registersUsed;                 // número de registos usados
    std::map<int, int> webToRegister;  // web ID -> registo (-1 = memória)
    std::vector<int> spilledWebs;      // IDs dos webs em memória

    AllocationResult() : success(false), registersUsed(0) {}
};

/**
 * @brief Parser de ficheiros
 */
class Parser {
public:
    static std::vector<LiveRange> parseRanges(const std::string& filename);
    static AllocationConfiguration parseConfiguration(const std::string& filename);

private:
    static std::string trim(const std::string& str);
    static LiveRange parseRangeLine(const std::string& line);
    static std::vector<int> parseLineNumbers(const std::string& str, bool& hasDef, bool& hasUse);
};

#endif /* DA_SECOND_PROJECT_PARSER_H */