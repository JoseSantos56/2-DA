#ifndef DA_SECOND_PROJECT_PARSER_H
#define DA_SECOND_PROJECT_PARSER_H

#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include <map>

/**
 * @brief Estruturas de dados e parsing de ficheiros de entrada para alocação de registos
 *
 * Este ficheiro define as estruturas fundamentais para representar:
 * - Live ranges (intervalos de vida de variáveis)
 * - Webs (união de live ranges)
 * - Configuração de alocação
 * - Resultados de alocação
 *
 * Também fornece a interface de parsing para ler ficheiros de entrada.
 *
 */


/**
 * @struct LiveRange
 * @brief Representa um intervalo de vida de uma variável
 *
 * Um live range define um conjunto contíguo ou não-contíguo de linhas
 * de programa onde uma variável está "viva" (seu valor pode ser usado).
 */

struct LiveRange {
    std::string variable;        ///< Nome da variável (ex: "x", "sum", "i")
    std::vector<int> lines;      ///< Linhas onde a variável está viva (ordenadas, sem duplicados)
    bool startsWithDef;          ///< true se começa com definição (marcador +)
    bool endsWithUse;            ///< true se termina com uso (marcador -)

    /**
    * @brief Construtor padrão
    *
    * Inicializa um live range vazio com marcadores a false.
    *
    * @complexity O(1)
    */

    LiveRange() : startsWithDef(false), endsWithUse(false) {}

    /**
    * @brief Obtém a primeira linha do range
    *
    * @return Número da primeira linha, ou -1 se o range estiver vazio
    * @complexity O(1) - acesso direto ao primeiro elemento do vector
    */
    int getStart() const { return lines.empty() ? -1 : lines.front(); }

    /**
     * @brief Obtém a última linha do range
     *
     * @return Número da última linha, ou -1 se o range estiver vazio
     * @complexity O(1) - acesso direto ao último elemento do vector
     */
    int getEnd() const { return lines.empty() ? -1 : lines.back(); }

    /**
     * @brief Obtém o número de linhas no range
     *
     * @return Quantidade de linhas onde a variável está viva
     * @complexity O(1) - tamanho do vector
     */
    size_t size() const { return lines.size(); }
};

/**
 * @struct Web
 * @brief Representa um web (união de live ranges da mesma variável)
 *
 * Um web é formado pela fusão de todos os live ranges de uma variável
 * que se sobrepõem ou se tocam em algum ponto do programa.
 *
 * Dois webs interferem se estão simultaneamente vivos em alguma linha,
 * exceto no caso especial onde um termina com uso (-) e outro começa
 * com definição (+) na mesma linha (exemplo: i=i+1).
 *
 * @invariant lines é sempre ordenado (propriedade do std::set)
 * @invariant defsLines ⊆ lines (definições estão contidas nas linhas)
 * @invariant usesLines ⊆ lines (usos estão contidos nas linhas)
 */

struct Web {
    int id;                     ///< Identificador único do web (≥0)
    std::string variable;       ///< Nome da variável associada
    std::set<int> lines;        ///< Todas as linhas do web (ordenadas automaticamente)
    std::set<int> defsLines;    ///< Linhas onde há definições (marcador +)
    std::set<int> usesLines;    ///< Linhas onde há usos finais (marcador -)

    /**
     * @brief Construtor padrão
     *
     * Cria um web inválido (id = -1).
     *
     * @complexity O(1)
     */
    Web() : id(-1) {}

    /**
    * @brief Construtor com ID e nome de variável
    *
    * @param webId Identificador único para este web
    * @param var Nome da variável
    *
    * @complexity O(1)
    */
    Web(int webId, const std::string& var) : id(webId), variable(var) {}

    /**
     * @brief Adiciona linhas de um live range ao web
     *
     * Insere todas as linhas do range no web e regista os marcadores
     * de definição (+) e uso (-) nas posições apropriadas.
     *
     * @param range Live range a adicionar
     *
     * @complexity O(L log W) onde:
     *   - L = número de linhas no range
     *   - W = número total de linhas já no web
     *
     * Justificação:
     * - insert em std::set é O(log n) por elemento
     * - fazemos L inserções no pior caso
     */
    void addRange(const LiveRange& range) {
        // Inserir todas as linhas do range
        // O(L log W) onde L = range.lines.size(), W = lines.size()
        lines.insert(range.lines.begin(), range.lines.end());

        // Registar definições (marcador +)
        // O(log W) - inserção em set
        if (range.startsWithDef && !range.lines.empty()) {
            defsLines.insert(range.lines.front());
        }

        // Registar usos (marcador -)
        // O(log W) - inserção em set
        if (range.endsWithUse && !range.lines.empty()) {
            usesLines.insert(range.lines.back());
        }
    }

    /**
     * @brief Verifica se existe definição numa linha específica
     *
     * @param line Número da linha a verificar
     * @return true se há uma definição (marcador +) nesta linha
     *
     * @complexity O(log D) onde D = número de definições no web
     *
     * Justificação:
     * - find em std::set é O(log n)
     */
    bool hasDefAt(int line) const {
        // find em std::set: O(log n)
        return defsLines.find(line) != defsLines.end();
    }

    /**
    * @brief Verifica se existe uso numa linha específica
    *
    * @param line Número da linha a verificar
    * @return true se há um uso final (marcador -) nesta linha
    *
    * @complexity O(log U) onde U = número de usos no web
    *
    * Justificação:
    * - find em std::set é O(log n)
    */
    bool hasUseAt(int line) const {
        // find em std::set: O(log n)
        return usesLines.find(line) != usesLines.end();
    }

    /**
     * @brief Verifica interferência com outro web
     *
     * Dois webs interferem se estão simultaneamente vivos em pelo menos
     * uma linha do programa.
     *
     * EXCEÇÃO: Se um web termina com uso (-) e outro começa com definição (+)
     * na mesma linha, eles NÃO interferem nessa linha (caso i=i+1).
     *
     * @param other Outro web a verificar
     * @return true se os webs interferem
     *
     * @complexity O(min(|this|, |other|)) onde |x| = número de linhas no web x
     *
     * Justificação:
     * - Verificação de limites: O(1) - acesso a begin() e rbegin()
     * - std::set_intersection: O(m + n) onde m, n são os tamanhos dos sets
     * - No pior caso: O(min(m, n)) se um set é muito menor
     */
    bool interferesWith(const Web& other) const {
        // Caso base: um dos webs está vazio
        // O(1)
        if (lines.empty() || other.lines.empty()) return false;

        // Otimização: verificar limites antes de calcular interseção
        // O(1) - acesso a begin() e rbegin() em std::set
        // *lines.rbegin() = maior elemento deste web
        // *other.lines.begin() = menor elemento do outro web
        if (*lines.rbegin() < *other.lines.begin() || *other.lines.rbegin() < *lines.begin()) {
            return false;  // Não há sobreposição possível
        }

        // Calcular interseção dos dois sets
        // O(m + n) onde m = |this.lines|, n = |other.lines|
        std::vector<int> intersection;
        std::set_intersection(lines.begin(), lines.end(), other.lines.begin(), other.lines.end(), std::back_inserter(intersection));

        return !intersection.empty();
    }

    /**
     * @brief Obtém o número de linhas no web
     *
     * @return Quantidade de linhas onde a variável está viva
     * @complexity O(1) - tamanho do set
     */
    size_t size() const { return lines.size(); }
};

/**
 * @struct AllocationConfiguration
 * @brief Configuração para algoritmo de alocação de registos
 *
 * Especifica os parâmetros de entrada para o algoritmo de alocação:
 * - Número de registos disponíveis
 * - Algoritmo a usar
 * - Parâmetros específicos do algoritmo
 */
struct AllocationConfiguration {
    int numRegisters;       ///< Número de registos físicos disponíveis (K)
    std::string algorithm;  ///< Nome do algoritmo: "basic", "spilling", "splitting", "custom"
    int parameter;          ///< Parâmetro do algoritmo (significado depende do algoritmo)

    /**
     * @brief Construtor padrão
     *
     * Inicializa com valores padrão: 0 registos, algoritmo "basic", parâmetro 0.
     *
     * @complexity O(1)
     */
    AllocationConfiguration() : numRegisters(0), algorithm("basic"), parameter(0) {}

    /**
     * @brief Valida se a configuração é utilizável
     *
     * @return true se numRegisters > 0
     * @complexity O(1)
     */
    bool isValid() const { return numRegisters > 0; }
};


/**
 * @struct AllocationResult
 * @brief Resultado da alocação de registos
 *
 * Contém o mapeamento de webs para registos físicos ou memória,
 * além de métricas sobre o sucesso da alocação.
 */
struct AllocationResult {
    bool success;                      ///< true se alocação bem-sucedida
    int registersUsed;                 ///< número de registos usados
    std::map<int, int> webToRegister;  ///< web ID -> registo (-1 = memória)
    std::vector<int> spilledWebs;      ///< IDs dos webs em memória

    /**
     * @brief Construtor padrão
     *
     * Inicializa como alocação falhada (success=false, registersUsed=0).
     *
     * @complexity O(1)
     */
    AllocationResult() : success(false), registersUsed(0) {}
};

/**
 * @class Parser
 * @brief Parser para ficheiros de entrada de live ranges e configuração
 *
 * Fornece métodos estáticos para ler e processar ficheiros de texto
 * no formato especificado para o projeto de alocação de registos.
 *
 * Formato de ficheiro de ranges:
 * @code
 * # comentário
 * variavel: linha1+,linha2,linha3-
 * variavel2: linha4+,linha5-
 * @endcode
 *
 * Formato de ficheiro de configuração:
 * @code
 * # comentário
 * registers: N
 * algorithm: basic | spilling, K | splitting, K | custom
 * @endcode
 *
 * @note Todos os métodos são estáticos - não é necessário criar instância.
 */
class Parser {
public:
    /**
    * @brief Faz parsing de um ficheiro de live ranges
    *
    * Lê um ficheiro de texto linha por linha, extrai os live ranges
    * e valida o formato.
    *
    * @param filename Caminho do ficheiro a ler
    * @return Vector de live ranges parseados
    *
    * @throws std::runtime_error se:
    *   - Ficheiro não pode ser aberto
    *   - Formato de linha é inválido
    *   - Nenhum live range foi encontrado
    *
    * @complexity O(N * M) onde:
    *   - N = número de linhas não vazias/comentários no ficheiro
    *   - M = comprimento médio de cada linha
    */
    static std::vector<LiveRange> parseRanges(const std::string& filename);

    /**
     * @brief Faz parsing de um ficheiro de configuração
     *
     * Lê um ficheiro de configuração e extrai:
     * - Número de registos disponíveis
     * - Algoritmo a usar
     * - Parâmetro do algoritmo (se aplicável)
     *
     * @param filename Caminho do ficheiro a ler
     * @return Configuração parseada
     *
     * @throws std::runtime_error se:
     *   - Ficheiro não pode ser aberto
     *   - Número de registos é inválido (≤0)
     *
     * @complexity O(N) onde N = número de linhas no ficheiro
     *
     */
    static AllocationConfiguration parseConfiguration(const std::string& filename);

private:

    /**
     * @brief Remove espaços em branco no início e fim de uma string
     *
     * Remove caracteres ' ', '\t', '\r', '\n' das extremidades.
     *
     * @param str String a processar
     * @return String sem espaços nas extremidades
     *
     * @complexity O(N) onde N = comprimento da string
     */
    static std::string trim(const std::string& str);

    /**
    * @brief Faz parsing de uma linha de live range
    *
    * Processa uma linha no formato: "variavel: linha1+,linha2,linha3-"
    *
    * @param line Linha a processar
    * @return LiveRange parseado
    *
    * @throws std::runtime_error se formato for inválido
    *
    * @complexity O(M) onde M = comprimento da linha
    */
    static LiveRange parseRangeLine(const std::string& line);

    /**
     * @brief Extrai números de linha com marcadores +/-
     *
     * Processa string como "1+,2,3,4-" e extrai:
     * - Vector de números de linha
     * - Se primeiro tem marcador +
     * - Se algum tem marcador -
     *
     * @param str String com números separados por vírgulas
     * @param hasDef [out] true se primeiro número tem '+'
     * @param hasUse [out] true se algum número tem '-'
     * @return Vector de números ordenados sem duplicados
     *
     * @throws std::runtime_error se número for inválido
     *
     * @complexity O(K log K) onde K = número de elementos
     */
    static std::vector<int> parseLineNumbers(const std::string& str, bool& hasDef, bool& hasUse);
};

#endif /* DA_SECOND_PROJECT_PARSER_H */