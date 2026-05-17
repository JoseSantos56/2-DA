/**
 * @file coloring.cpp
 * @brief Graph coloring algorithms for register allocation (T2.1 and T2.2).
 */
#include "coloring.h"
#include <stack>
#include <set>
#include <iostream>

static int effectiveDegree(Vertex<int>* v, const std::set<int>& disabled) {
    int deg = 0;
    for (auto e : v->getAdj()) {
        if (!disabled.count(e->getDest()->getInfo())) deg++;
    }
    return deg;
}

/**
 * @brief Greedy graph coloring (T2.1 - basic).
 * Phase 1: simplification - remove nodes with degree < K onto stack.
 * Phase 2: coloring - pop and assign lowest available color.
 * Time complexity: O(V * (V + E))
 */
AllocationResult basicColoring(Graph<int>& g, int K) {
    AllocationResult result;
    result.success = false;
    result.registersUsed = 0;

    std::vector<Vertex<int>*> vertices = g.getVertexSet();
    if (vertices.empty()) { result.success = true; return result; }

    std::set<int> disabled, spilled;
    std::stack<int> S;
    int totalNodes = (int)vertices.size();

    while ((int)(disabled.size() + spilled.size()) < totalNodes) {
        bool removed = true;
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
        bool allDone = true;
        for (auto v : g.getVertexSet()) {
            int id = v->getInfo();
            if (!disabled.count(id) && !spilled.count(id)) { allDone = false; break; }
        }
        if (!allDone) {
            int spillId = -1, maxDeg = -1;
            for (auto v : g.getVertexSet()) {
                int id = v->getInfo();
                if (disabled.count(id) || spilled.count(id)) continue;
                int deg = effectiveDegree(v, disabled);
                if (deg > maxDeg) { maxDeg = deg; spillId = id; }
            }
            if (spillId != -1) { spilled.insert(spillId); disabled.insert(spillId); }
        }
    }

    std::map<int, int> color;
    std::set<int> usedColors;
    while (!S.empty()) {
        int id = S.top(); S.pop();
        std::set<int> neighborColors;
        Vertex<int>* v = g.findVertex(id);
        if (v) {
            for (auto e : v->getAdj()) {
                int nid = e->getDest()->getInfo();
                if (color.count(nid)) neighborColors.insert(color[nid]);
            }
        }
        int assignedColor = -1;
        for (int c = 0; c < K; c++) {
            if (!neighborColors.count(c)) { assignedColor = c; break; }
        }
        if (assignedColor == -1) { spilled.insert(id); }
        else { color[id] = assignedColor; usedColors.insert(assignedColor); }
    }

    for (auto v : g.getVertexSet()) {
        int id = v->getInfo();
        if (spilled.count(id)) {
            result.webToRegister[id] = -1;
            result.spilledWebs.push_back(id);
        } else {
            result.webToRegister[id] = color.count(id) ? color[id] : -1;
        }
    }
    result.registersUsed = (int)usedColors.size();
    if (spilled.empty()) {
        result.success = true;
    } else {
        result.success = false;
        result.registersUsed = 0;
        std::cerr << "Warning: register allocation with " << K << " registers is not possible.\n";
    }
    return result;
}

/**
 * @brief Selects web with highest degree not yet spilled.
 * Rationale: removing the most-connected web simplifies the graph most.
 */
int chooseSpillWeb(Graph<int>& g, const std::vector<int>& spilledWebs) {
    std::set<int> spilledSet(spilledWebs.begin(), spilledWebs.end());
    int bestId = -1, maxDeg = -1;
    for (auto v : g.getVertexSet()) {
        int id = v->getInfo();
        if (spilledSet.count(id)) continue;
        int deg = (int)v->getAdj().size();
        if (deg > maxDeg) { maxDeg = deg; bestId = id; }
    }
    return bestId;
}

/**
 * @brief Register allocation with web spilling (T2.2 - spilling, K).
 * Tries basicColoring, if it fails spills the highest-degree web and retries.
 * Time complexity: O(maxSpills * V * (V + E))
 */
AllocationResult spillingColoring(Graph<int>& g, int K, int maxSpills) {
    AllocationResult result;
    result.success = false;
    result.registersUsed = 0;

    std::vector<int> spilledWebs;

    for (int spills = 0; spills <= maxSpills; spills++) {
        std::set<int> spilledSet(spilledWebs.begin(), spilledWebs.end());

        Graph<int> reduced;
        for (auto v : g.getVertexSet()) {
            if (!spilledSet.count(v->getInfo())) reduced.addVertex(v->getInfo());
        }
        for (auto v : g.getVertexSet()) {
            if (spilledSet.count(v->getInfo())) continue;
            for (auto e : v->getAdj()) {
                int destId = e->getDest()->getInfo();
                if (spilledSet.count(destId)) continue;
                bool exists = false;
                for (auto re : reduced.findVertex(v->getInfo())->getAdj()) {
                    if (re->getDest()->getInfo() == destId) { exists = true; break; }
                }
                if (!exists) reduced.addEdge(v->getInfo(), destId, 1.0);
            }
        }

        AllocationResult attempt = basicColoring(reduced, K);
        if (attempt.success) {
            result = attempt;
            for (int sid : spilledWebs) {
                result.webToRegister[sid] = -1;
                result.spilledWebs.push_back(sid);
            }
            result.success = true;
            return result;
        }

        if (spills < maxSpills) {
            int toSpill = chooseSpillWeb(g, spilledWebs);
            if (toSpill == -1) break;
            spilledWebs.push_back(toSpill);
        }
    }

    result.success = false;
    result.registersUsed = 0;
    for (auto v : g.getVertexSet()) {
        result.webToRegister[v->getInfo()] = -1;
        result.spilledWebs.push_back(v->getInfo());
    }
    std::cerr << "Warning: register allocation with " << K << " registers and up to "
              << maxSpills << " spills is not possible.\n";
    return result;
}
