#include "Graph.h"
#include "splitting.h"
#include "coloring.h"

#include <vector>


int chooseWeb(Graph<int>& g) {          // Greedy
    int bestDegree = -1;
    int bestWebId = -1;

    for (auto w: g.getVertexSet()) {        // A web com maior degree é escolhida
        int indegree = w->getIndegree();

        if (indegree > bestDegree) {
            bestDegree = indegree;
            bestWebId = w->getInfo();
        }
    }

    return bestWebId;      // Retorna -1 caso não tenha webs
}

int findLargestGap(const WebInfo& web) {		// Escolhe o maior gap da web selecionada
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

bool websIntersection(const WebInfo& web1, const WebInfo& web2) {		// Rever mais tarde por causa dos sinais
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


int splitGap(Graph<int>& g, std::vector<WebInfo>& allWebs, const int bestWebId, const int bestGapId) {
	const WebInfo& old = allWebs[bestWebId];

	// Descobrir os ids e os intervalos para cada nova web
	WebInfo leftWeb;
	int leftId = old.id;

	leftWeb.id = leftId;
	for (int i = 0; i < bestGapId + 1; i++){
		leftWeb.intervals.push_back(old.intervals[i]);
	}

	WebInfo rightWeb;
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
	std::vector<WebInfo> newAllWebs;		// vetor para adicionar as novas webs
	for (size_t i = 0; i < allWebs.size(); i++) {
		if ((int)i == leftId) newAllWebs.push_back(leftWeb);
		else newAllWebs.push_back(allWebs[i]);
	}

	newAllWebs.push_back(rightWeb);
	allWebs = newAllWebs;

	return 0;
}

int splitMiddle(Graph<int>& g, std::vector<WebInfo>& allWebs, const int bestWebId) {
	WebInfo old = allWebs[bestWebId];

	int startOld = old.intervals[0].start;
	int endOld = old.intervals[0].end;

	int middle = (endOld - startOld) / 2 + startOld;		// Evita overflow

	// Descobrir os ids e os intervalos para cada nova web
	WebInfo leftWeb;
	int leftId = old.id;

	leftWeb.id = leftId;

	leftWeb.intervals.push_back({startOld, middle});

	WebInfo rightWeb;
	int rightId = g.getNumVertex();

	rightWeb.id = rightId;

	rightWeb.intervals.push_back({middle + 1, endOld});

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
	std::vector<WebInfo> newAllWebs;		// vetor para adicionar as novas webs
	for (size_t i = 0; i < allWebs.size(); i++) {
		if ((int)i == leftId) newAllWebs.push_back(leftWeb);
		else newAllWebs.push_back(allWebs[i]);
	}

	newAllWebs.push_back(rightWeb);
	allWebs = newAllWebs;

	return 0;
}


int splitWeb(Graph<int>& g, std::vector<WebInfo>& allWebs, const int n, const int k) {    /*   WebInfo e allWebs tem de ser implementada por quem faz o parser */

    // Modificação do grafo original pois apenas é aplicada uma das abordagens (splitting ou spilling) por cada grafo
	AllocationResult result = basicColoring(g, k);

	if (result.success) {		// Verificação incial
		return 0;
	}

	for (int tries = 0; tries < n; tries++) {

		int bestWebId = 0;
		if ((bestWebId = chooseWeb(g)) == -1) return -1;		// Erro se não tiver webs

		WebInfo& web = allWebs[bestWebId];
		int bestGapId = 0;
		if ((bestGapId = findLargestGap(web)) != -1) {
			splitGap(g, allWebs, bestWebId, bestGapId);
		}

		else {
			splitMiddle(g, allWebs, bestWebId);
		}

		result = basicColoring(g, k);
		if (result.success) {
			return 0;
		}
	}

	return -1;
}
