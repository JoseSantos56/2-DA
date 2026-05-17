#ifndef COLORING_H
#define COLORING_H

#include "Graph.h"
#include "parser.h"
#include <vector>

/**
 * @brief Greedy graph coloring algorithm (T2.1 - algorithm: basic).
 *
 * Implements the greedy coloring heuristic from the project description:
 * repeatedly removes nodes with degree < K onto a stack, then pops
 * and assigns colors. If the graph cannot be colored with K colors,
 * reports infeasibility via stderr and returns all webs to memory.
 *
 * Time complexity: O(V * (V + E)) where V = number of webs, E = edges.
 *
 * @param g   Interference graph (nodes = web IDs)
 * @param K   Maximum number of registers available
 * @return    AllocationResult with the web->register assignment
 */
AllocationResult basicColoring(Graph<int>& g, int K);

/**
 * @brief Register allocation with web spilling (T2.2 - algorithm: spilling, K).
 *
 * Attempts basic coloring first. If it fails, spills up to maxSpills webs
 * (chosen by highest degree - most interfering) and retries coloring.
 * Spilled webs are assigned to memory (register = -1).
 *
 * Time complexity: O(maxSpills * V * (V + E))
 *
 * @param g          Interference graph (nodes = web IDs)
 * @param K          Maximum number of registers available
 * @param maxSpills  Maximum number of webs allowed to spill
 * @return           AllocationResult with the web->register assignment
 */
AllocationResult spillingColoring(Graph<int>& g, int K, int maxSpills);

/**
 * @brief Chooses the best web to spill (highest degree = most interfering).
 *
 * Rationale: spilling the most-connected web removes the most edges,
 * giving the best chance of making the remaining graph K-colorable.
 *
 * @param g              Interference graph
 * @param spilledWebs    Already-spilled web IDs to skip
 * @return               Web ID to spill, or -1 if none available
 */
int chooseSpillWeb(Graph<int>& g, const std::vector<int>& spilledWebs);

#endif // COLORING_H
