#ifndef COLORING_H
#define COLORING_H

#include "Graph.h"
#include <map>
#include <vector>
#include <string>

/**
 * @brief Result of a register allocation attempt.
 *
 * Maps each web ID to a register index (0..K-1),
 * or to -1 if the web was spilled to memory.
 */
struct AllocationResult {
    int registersUsed;                  ///< Number of registers actually used (0 if infeasible)
    std::map<int, int> webToRegister;   ///< web ID -> register index (-1 = spilled)
    bool feasible;                      ///< true if allocation succeeded without spilling
};

/**
 * @brief Greedy graph coloring algorithm (T2.1 - algorithm: basic).
 *
 * Implements the greedy coloring heuristic from the project description:
 * repeatedly removes nodes with degree < K onto a stack, then pops
 * and assigns colors. If the graph cannot be colored with K colors,
 * returns a result with feasible=false and all webs assigned to memory.
 *
 * Time complexity: O(V * (V + E)) where V = number of webs, E = number of edges.
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
 * @param g              Interference graph
 * @param spilledWebs    Set of already-spilled web IDs to skip
 * @return               Web ID to spill, or -1 if none available
 */
int chooseSpillWeb(Graph<int>& g, const std::vector<int>& spilledWebs);

#endif // COLORING_H
