#include "Graph.h"
#include "customAllocator.h"

#include <set>
#include <vector>


int chooseNextWeb(Graph<int>& g, const ColoringState& state) {

    int bestWebId = -1;
    int bestSat = -1;
    int bestDegree = -1;

    for (int webId : state.unprocessed) {
        Vertex<int>* v = g.findVertex(webId);
        if (v == nullptr) continue;

        int sat = state.saturation[webId];
        int degree = v->getAdj().size();

        if (sat > bestSat || (sat == bestSat && degree < bestDegree)) {
            bestSat = sat;
            bestDegree = degree;
            bestWebId = webId;
        }
    }

    return bestWebId;
}


bool isColorAvailable(ColoringState& state, const int webId, const int color) {

    return state.adjColors[webId].count(color) == 0;
}

void updateSaturation(Graph<int>& g, ColoringState& state, const int webId, const int color) {

    Vertex<int>* v = g.findVertex(webId);
    if (v == nullptr) return;

    for (auto e : v->getAdj()) {
        int adj = e->getDest()->getInfo();

        if (!state.processed[adj] && state.adjColors[adj].insert(color).second) {
            state.saturation[adj]++;
        }
    }
}

bool backtrackDSatur(Graph<int>& g, const int k, ColoringState& state, int spillsLeft) {

    if (state.unprocessed.empty()) return true;

    int webId = chooseNextWeb(g, state);
    if (webId == -1) return false;

    for (int color = 0; color < k; color++) {

        if (isColorAvailable(state, webId, color)) {

            // Guardar o estado inicial
            std::vector<int> oldColors = state.colors;
            std::vector<bool> oldSpilled = state.spilled;
            std::vector<bool> oldProcessed = state.processed;
            std::vector<std::set<bool>> oldAdjColors = state.adjColors;
            std::vector<int> oldSaturation = state.saturation;
            std::set<bool> oldUnprocessed = state.unprocessed;

            // Aplicar a cor e o que isso implica
            state.colors[webId] = color;
            state.processed[webId] = true;
            state.unprocessed.erase(webId);
            updateSaturation(g, state, webId, color);

            if (backtrackDSatur(g, k, state, spillsLeft)) return true;

            // Backtrack
            std::vector<int> colors = oldColors;
            std::vector<bool> spilled = oldSpilled;
            std::vector<bool> processed = oldProcessed;
            std::vector<std::set<bool>> adjColors = oldAdjColors;
            std::vector<int> saturation = oldSaturation;
            std::set<bool> unprocessed = oldUnprocessed;
        }
    }

    // Tentar dar spill para ver se resolve o problema
    if (spillsLeft > 0 ) {

        std::vector<bool> oldSpilled = state.spilled;
        std::vector<bool> oldProcessed = state.processed;
        std::set<bool> oldUnprocessed = state.unprocessed;

        // Alterar para spilled e tudo o que isso implica
        state.spilled[webId] = true;;
        state.processed[webId] = true;
        state.unprocessed.erase(webId);

        if (backtrackDSatur(g, k, state, spillsLeft - 1)) return true;

        // Backtrack
        state.spilled = oldSpilled;
        state.processed = oldProcessed;
        state.unprocessed = oldUnprocessed;
    }

    return false;
}

ColoringState initState(Graph<int>& g, int v) {

    ColoringState state;
    state.colors.resize(v, -1);
    state.spilled.resize(v, false);
    state.processed.resize(v, false);
    state.saturation.resize(v, 0);
    state.adjColors.resize(v);

    for (auto vertex : g.getVertexSet()) {
        state.unprocessed.insert(vertex->getInfo());
    }

    return state;
}

bool greedyDSatur(Graph<int>& g, int k, std::vector<int>& colors, std::vector<bool>& spilled) {

    int v = g.getNumVertex();

    ColoringState state = initState(g, v);

    while (!state.unprocessed.empty()) {

        int webId = chooseNextWeb(g, state);
        if (webId == -1) return false;

        bool assigned = false;

        for (int color = 0; color < k; color++) {

            if (isColorAvailable(state, webId, color)) {

                state.colors[webId] = color;
                state.processed[webId] = true;
                state.unprocessed.erase(webId);
                updateSaturation(g, state, webId, color);

                assigned = true;
                break;
            }
        }

        if (!assigned) {
            // Dá spill caso não consiga colorir
            state.spilled[webId] = true;
            state.processed[webId] = true;
            state.unprocessed.erase(webId);
        }
    }

    colors = state.colors;
    spilled = state.spilled;

    return true;
}



