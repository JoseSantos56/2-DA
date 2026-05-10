#include "Graph.h"
#include "splitting.h"

#include <vector>

// Vou ter de dar include ao coloring.h

int chooseWeb(Graph<int>& g) {          // Greedy
    int density = -1;
    int bestWebId = -1;

    for (auto w: g.getVertexSet()) {        // A web com maior densidade é escolhida
        int indegree = w->getIndegree();

        if (indegree > density) {
            density = indegree;
            bestWebId = w->getInfo();
        }
    }

    return bestWebId;      // Retorna -1 caso não tenha webs
}

int findLargestGap(const webInfo& web) {		// Escolhe o maior gap da web selecionada
	// Em caso de empate escolhemos o primeiro gap
	int bestGapId = -1;
	int largestGapSize = -1;

	for (size_t i = 0; i + 1 < web.intervals.size(); i++) {
		int left = web.intervals[i].end;					// Máximo deste intervalo
		int right =	web.intervals[i + 1].start;				// Mínimo deste intervalo
		int gap = right - left - 1;

		if (gap > largestGapSize) {
			largestGapSize = gap;
			bestGapId = i;
		}
	}

	return bestGapId;		// Return -1 em caso de não ter gaps
}

bool websIntersection(const webInfo& web1, const webInfo& web2) {		// Rever mais tarde por causa dos sinais
	size_t i = 0, j = 0;

	while (i < web1.intervals.size() && j < web2.intervals.size()) {
		int s1 = web1.intervals[i].start;
		int e1 = web1.intervals[i].end;
		int s2 = web2.intervals[j].start;
		int e2 = web2.intervals[j].end;


		if (e1 < s2) {
			i++;
		}

		else if (e2 < s1) {
			j++;
		}

		else return true;
	}

	return false;
}


int splitGap(Graph<int>& g, std::vector<webInfo>& allWebs, int bestWebId, int bestGapId) {
	webInfo old = allWebs[bestWebId];

	// Descobrir os ids e os intervalos para cada nova web
	webInfo leftWeb;
	int leftId = old.id;

	leftWeb.id = leftId;
	for (int i = 0; i < bestGapId + 1; i++){
		leftWeb.intervals.push_back(old.intervals[i]);
	}

	webInfo rightWeb;
	int rightId = g.getNumVertex();

	rightWeb.id = rightId;
	for (size_t i = bestGapId + 1; i < old.intervals.size(); i++){
		rightWeb.intervals.push_back(old.intervals[i]);
	}

	g.removeVertex(bestWebId);
	g.addVertex(leftId);
	g.addVertex(rightId);

	for (const auto& other : allWebs) {
		if(other.id == bestWebId) continue;

		if (websIntersection(leftWeb, other)) {
			g.addEdge(leftId, other.id, 1);
			g.addEdge(other.id,leftId, 1);
		}

		if (websIntersection(rightWeb, other)) {
			g.addEdge(rightId, other.id, 1);
			g.addEdge(other.id,rightId, 1);
		}

	}

	// A posição da web tem de ser igual ao ID
	std::vector<webInfo> newAllWebs;
	for (size_t i = 0; i < allWebs.size(); i++) {
		if ((int)i == leftId) newAllWebs.push_back(leftWeb);
		else newAllWebs.push_back(allWebs[i]);
	}

	newAllWebs.push_back(rightWeb);
	allWebs = newAllWebs;

	return 0;
}

void splitWeb(Graph<int>& g, std::vector<webInfo>& allWebs, int bestWebID) {    /*   webInfo e allWebs tem de ser implementada por quem faz o parser */
    // Modificação do grafo original pois apenas é aplicada uma das abordagens (splitting ou spilling) por cada grafo

	return;
}
