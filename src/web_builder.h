#ifndef WEB_BUILDER_H
#define WEB_BUILDER_H

#include "Graph.h"
#include "parser.h"
#include <vector>
#include <map>

/**
 * @brief Constrói webs a partir dos live ranges e o grafo de interferência.
 *
 * Um web é a união dos live ranges da mesma variável que se sobrepõem
 * em algum ponto do programa. Dois webs interferem se estiverem
 * simultaneamente vivos em algum ponto do programa.
 */
class WebBuilder {
public:
    /**
     * @brief Funde os live ranges em webs para cada variável.
     *
     * Os live ranges da mesma variável são fundidos num único web
     * se partilharem alguma linha de programa. Usa uma abordagem greedy de união.
     *
     * Complexidade temporal: O(R^2 * L) onde R = número de ranges, L = linhas por range.
     *
     * @param ranges  Todos os live ranges lidos do ficheiro de input
     * @return        Vetor de webs (cada um com um ID único)
     */
    static std::vector<Web> buildWebs(const std::vector<LiveRange>& ranges);

    /**
     * @brief Constrói o grafo de interferência a partir de um conjunto de webs.
     *
     * Adiciona um vértice por web. Adiciona uma aresta bidirecional entre dois webs
     * se interferirem (ou seja, se estiverem simultaneamente vivos em algum ponto).
     *
     * Caso especial: um web A que começa na linha X (definição) e um web B
     * que termina na linha X (uso) NÃO interferem nesse ponto.
     *
     * Complexidade temporal: O(W^2 * L) onde W = número de webs, L = linhas por web.
     *
     * @param webs  Vetor de webs
     * @param g     Grafo de interferência de output (Graph<int>, nó = ID do web)
     */
    static void buildInterferenceGraph(const std::vector<Web>& webs, Graph<int>& g);
};

#endif // WEB_BUILDER_H
