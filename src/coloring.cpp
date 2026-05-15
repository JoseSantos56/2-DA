#include "coloring.h"
#include <stack>
#include <set>
#include <algorithm>
#include <iostream>

// ============================================================
// Helper: compute effective degree of a vertex in the
// "active" subgraph (ignoring disabled vertices).
// ============================================================
static int effectiveDegree(Vertex<int>* v, const std::set<int>& disabled) {
    int deg = 0;
    for (auto e : v->getAdj()) {
        if (disabled.find(e->getDest()->getInfo()) == disabled.end()) {
            deg++;
        }
    }
    return deg;
}

// ============================================================
// T2.1 - Basic greedy graph coloring
// ============================================================

/**
 * @brief Greedy graph coloring (basic).
 *
 * Phase 1 - Simplification:
 *   While there exists a node with effective degree < K:
 *     remove it from the active graph and push onto stack S.
 *   If no such node exists and graph is not empty:
 *     select a node to spill (mark as uncolorable), remove it.
 *
 * Phase 2 - Coloring:
 *   Pop nodes from S and assign the lowest color not used
 *   by any already-colored neighbor.
 *
 * Time complexity: O(V * (V + E))
 */
AllocationResult basicColoring(Graph<int>& g, int K) {
    AllocationResult result;
    result.registersUsed = 0;
    result.feasible = false;

    std::vector<Vertex<int>*> vertices = g.getVertexSet();
    if (vertices.empty()) {
        result.feasible = true;
        return result;
    }

    // disabled = nodes removed from active graph
    std::set<int> disabled;
    // spilled = nodes that could not be simplified (forced spills)
    std::set<int> spilled;
    // stack for coloring phase
    std::stack<int> S;

    int totalNodes = (int)vertices.size();

    // Phase 1: Simplification loop
    while ((int)(disabled.size() + spilled.size()) < totalNodes) {

        bool removed = true;

        // Keep removing nodes with degree < K until none remain
        while (removed) {
            removed = false;
            for (auto v : g.getVertexSet()) {
                int id = v->getInfo();
                if (disabled.count(id) || spilled.count(id)) continue;

                if (effectiveDegree(v, disabled) < K) {
                    disabled.insert(id);
                    S.push(id);
                    removed = true;
                }
            }
        }

        // If nodes remain and none had degree < K, we must spill one
        bool allDone = true;
        for (auto v : g.getVertexSet()) {
            int id = v->getInfo();
            if (!disabled.count(id) && !spilled.count(id)) {
                allDone = false;
                break;
            }
        }

        if (!allDone) {
            // Spill: choose node with highest effective degree
            int spillId = -1;
            int maxDeg = -1;
            for (auto v : g.getVertexSet()) {
                int id = v->getInfo();
                if (disabled.count(id) || spilled.count(id)) continue;
                int deg = effectiveDegree(v, disabled);
                if (deg > maxDeg) {
                    maxDeg = deg;
                    spillId = id;
                }
            }
            if (spillId != -1) {
                spilled.insert(spillId);
                disabled.insert(spillId); // treat as removed for degree calc
            }
        }
    }

    // Phase 2: Coloring - pop from stack and assign colors
    std::map<int, int> color; // web ID -> color (register)
    int maxColorUsed = -1;

    while (!S.empty()) {
        int id = S.top();
        S.pop();

        // Find colors used by neighbors
        std::set<int> neighborColors;
        Vertex<int>* v = g.findVertex(id);
        if (v) {
            for (auto e : v->getAdj()) {
                int neighId = e->getDest()->getInfo();
                if (color.count(neighId)) {
                    neighborColors.insert(color[neighId]);
                }
            }
        }

        // Assign lowest available color
        int assignedColor = -1;
        for (int c = 0; c < K; c++) {
            if (!neighborColors.count(c)) {
                assignedColor = c;
                break;
            }
        }

        if (assignedColor == -1) {
            // Should not happen if degree < K was guaranteed, but handle safely
            spilled.insert(id);
        } else {
            color[id] = assignedColor;
            if (assignedColor > maxColorUsed) maxColorUsed = assignedColor;
        }
    }

    // Build result
    std::set<int> usedColors;
    for (auto v : g.getVertexSet()) {
        int id = v->getInfo();
        if (spilled.count(id)) {
            result.webToRegister[id] = -1; // memory
        } else {
            result.webToRegister[id] = color.count(id) ? color[id] : -1;
            if (color.count(id)) usedColors.insert(color[id]);
        }
    }

    result.registersUsed = (int)usedColors.size();

    // feasible = no forced spills
    if (spilled.empty()) {
        result.feasible = true;
    } else {
        result.feasible = false;
        result.registersUsed = 0;
        std::cerr << "Warning: register allocation with " << K
                  << " registers is not possible without spilling.\n";
    }

    return result;
}

// ============================================================
// Helper: choose best web to spill (highest degree)
// ============================================================

/**
 * @brief Selects the web with highest degree not yet spilled.
 *
 * Rationale: spilling the most-connected web removes the most
 * edges from the interference graph, giving the best chance of
 * making the remaining graph K-colorable.
 */
int chooseSpillWeb(Graph<int>& g, const std::vector<int>& spilledWebs) {
    std::set<int> spilledSet(spilledWebs.begin(), spilledWebs.end());

    int bestId = -1;
    int maxDeg = -1;

    for (auto v : g.getVertexSet()) {
        int id = v->getInfo();
        if (spilledSet.count(id)) continue;

        int deg = (int)v->getAdj().size();
        if (deg > maxDeg) {
            maxDeg = deg;
            bestId = id;
        }
    }

    return bestId;
}

// ============================================================
// T2.2 - Register allocation with web spilling
// ============================================================

/**
 * @brief Register allocation with web spilling.
 *
 * Strategy:
 *   1. Try basicColoring with K registers.
 *   2. If it fails, spill the web with the highest degree
 *      (most interfering - removing it simplifies the graph most).
 *   3. Retry basicColoring on the reduced graph (without spilled webs).
 *   4. Repeat up to maxSpills times.
 *
 * Spilled webs are assigned to memory (register = -1).
 *
 * Time complexity: O(maxSpills * V * (V + E))
 */
AllocationResult spillingColoring(Graph<int>& g, int K, int maxSpills) {
    AllocationResult result;
    result.registersUsed = 0;
    result.feasible = false;

    std::vector<int> spilledWebs; // IDs of webs chosen for spilling

    // Try with 0 spills first, then increase
    for (int spills = 0; spills <= maxSpills; spills++) {

        // Build a subgraph without spilled webs
        // We work with the original graph but skip spilled nodes
        std::set<int> spilledSet(spilledWebs.begin(), spilledWebs.end());

        // Temporarily collect active vertices
        std::vector<Vertex<int>*> activeVertices;
        for (auto v : g.getVertexSet()) {
            if (!spilledSet.count(v->getInfo())) {
                activeVertices.push_back(v);
            }
        }

        if (activeVertices.empty()) {
            // All spilled
            result.feasible = false;
            break;
        }

        // Build a temporary reduced graph
        Graph<int> reducedGraph;
        for (auto v : activeVertices) {
            reducedGraph.addVertex(v->getInfo());
        }
        for (auto v : activeVertices) {
            for (auto e : v->getAdj()) {
                int destId = e->getDest()->getInfo();
                if (!spilledSet.count(destId)) {
                    // addEdge only if not already present (bidirectional edges)
                    if (!reducedGraph.findVertex(v->getInfo())->getAdj().empty()) {
                        bool exists = false;
                        for (auto re : reducedGraph.findVertex(v->getInfo())->getAdj()) {
                            if (re->getDest()->getInfo() == destId) {
                                exists = true;
                                break;
                            }
                        }
                        if (!exists) reducedGraph.addEdge(v->getInfo(), destId, 1.0);
                    } else {
                        reducedGraph.addEdge(v->getInfo(), destId, 1.0);
                    }
                }
            }
        }

        // Try coloring the reduced graph
        AllocationResult attempt = basicColoring(reducedGraph, K);

        if (attempt.feasible) {
            // Success! Add spilled webs as memory allocations
            result = attempt;
            for (int sid : spilledWebs) {
                result.webToRegister[sid] = -1;
            }
            result.feasible = true;
            return result;
        }

        // Coloring failed - spill one more web if we have budget
        if (spills < maxSpills) {
            int toSpill = chooseSpillWeb(g, spilledWebs);
            if (toSpill == -1) break; // no more webs to spill
            spilledWebs.push_back(toSpill);
        }
    }

    // Could not allocate even with maxSpills spills
    result.feasible = false;
    result.registersUsed = 0;
    for (auto v : g.getVertexSet()) {
        result.webToRegister[v->getInfo()] = -1;
    }
    std::cerr << "Warning: register allocation with " << K
              << " registers and up to " << maxSpills
              << " spills is not possible.\n";

    return result;
}
