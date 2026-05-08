#include "Graph.h"
#include "splitting.h"

#include <vector>

// Vou ter de dar include ao coloring.h

int chooseWeb(Graph<int>& g) {          // Greedy
    int density = -1;
    int resWeb = -1;

    for (auto w: g.getVertexSet()) {        // A web com maior densidade é escolhida
        int indegree = w->getIndegree();

        if (indegree > density) {
            density = indegree;
            resWeb = w->getInfo();
        }
    }

    return resWeb;
}