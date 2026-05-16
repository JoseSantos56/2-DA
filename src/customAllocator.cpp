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
