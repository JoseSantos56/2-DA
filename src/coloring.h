#ifndef COLORING_H
#define COLORING_H

#include "Graph.h"
#include "parser.h"
#include <vector>

/**
 * @brief Algoritmo greedy de coloração de grafos (T2.1 - algoritmo: basic).
 *
 * Implementa a heurística greedy de coloração descrita no enunciado:
 * remove repetidamente nós com grau < K para uma stack e depois
 * faz pop atribuindo cores. Se o grafo não puder ser colorido com K cores,
 * reporta inviabilidade via stderr e devolve todos os webs para memória.
 *
 * Complexidade temporal: O(V * (V + E)) onde V = número de webs, E = arestas.
 *
 * @param g   Grafo de interferência (nós = IDs dos webs)
 * @param K   Número máximo de registos disponíveis
 * @return    AllocationResult com a atribuição web->registo
 */
AllocationResult basicColoring(Graph<int>& g, int K);

/**
 * @brief Alocação de registos com spilling de webs (T2.2 - algoritmo: spilling, K).
 *
 * Tenta primeiro o basicColoring. Se falhar, faz spill de até maxSpills webs
 * (escolhidos pelo maior grau - mais interferentes) e tenta novamente.
 * Webs em spill são alocados a memória (registo = -1).
 *
 * Complexidade temporal: O(maxSpills * V * (V + E))
 *
 * @param g          Grafo de interferência (nós = IDs dos webs)
 * @param K          Número máximo de registos disponíveis
 * @param maxSpills  Número máximo de webs permitidos em spill
 * @return           AllocationResult com a atribuição web->registo
 */
AllocationResult spillingColoring(Graph<int>& g, int K, int maxSpills);

/**
 * @brief Escolhe o melhor web para spill (maior grau = mais interferente).
 *
 * Racional: fazer spill do web mais conectado remove o maior número de arestas,
 * dando a melhor hipótese de tornar o grafo restante K-colorível.
 *
 * @param g              Grafo de interferência
 * @param spilledWebs    IDs dos webs já em spill a ignorar
 * @return               ID do web a fazer spill, ou -1 se não houver nenhum disponível
 */
int chooseSpillWeb(Graph<int>& g, const std::vector<int>& spilledWebs);

#endif // COLORING_H
