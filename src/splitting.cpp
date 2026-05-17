#include "Graph.h"
#include "splitting.h"
#include "coloring.h"

#include <vector>


int chooseWeb(Graph<int>& g) {          // Greedy
    int bestDegree = -1;
    int bestWebId = -1;

    for (auto w: g.getVertexSet()) {        // A web com maior degree é escolhida
        int indegree = (int)w->getAdj().size();

        if (indegree > bestDegree) {
            bestDegree = indegree;
            bestWebId = w->getInfo();
        }
    }

    return bestWebId;      // Retorna -1 caso não tenha webs
}

int findLargestGap(const Web& web) {		// Escolhe o maior gap da web selecionada

	// Em caso de empate escolhemos o primeiro gap
	if (web.lines.size() < 2) return -1;

	int bestGapId = -1;
	int largestGapSize = -1;

	auto prevIt = web.lines.begin();			// Mínimo deste intervalo
	auto curIt = prevIt;						// Máximo deste intervalo
	++curIt;

	for (int id = 0; curIt != web.lines.end(); id++) {
		int gap = *curIt - *prevIt - 1;

		if (gap > largestGapSize) {
			largestGapSize = gap;
			bestGapId = id;
		}
		++prevIt;
		++curIt;
	}

	return bestGapId;		// Return -1 em caso de não ter gaps
}

bool websIntersection(const Web& web1, const Web& web2) {
	if (web1.lines.empty() || web2.lines.empty()) return false;

	// Caso não se toquem
	if (*web1.lines.rbegin() < *web2.lines.begin() || *web2.lines.rbegin() < *web1.lines.begin()) return false;

	int line = -1;
	if (*web1.lines.rbegin() == *web2.lines.begin()) line = *web1.lines.rbegin();
	else if (*web2.lines.rbegin() == *web1.lines.begin()) line = *web1.lines.begin();

	if (line != -1) {
		if ((web1.hasUseAt(line) && web2.hasDefAt(line)) || (web2.hasUseAt(line) && web1.hasDefAt(line))) return false;
	}
	return true;
}


int splitGap(Graph<int>& g, std::vector<Web>& allWebs, const int bestWebId, const int bestGapId) {

	Web* oldptr = nullptr;
	for (auto& w : allWebs) {				// Encontrar web
		if (w.id == bestWebId) {
			oldptr = &w;
			break;
		}
	}

	if (oldptr == nullptr) return -1;		// caso não encontre

	const Web old = *oldptr;			// Cópia pra não alterar allWebs

	Web leftWeb(old.id, old.variable);
	Web rightWeb(g.getNumVertex(), old.variable);

	auto it = old.lines.begin();
	std::advance(it,bestGapId + 1);		// Avançar o iterador até a posição inicial da direita

	for (auto lit = old.lines.begin(); lit != it; ++lit) {
		leftWeb.lines.insert(*lit);
		if (old.defsLines.count(*lit)) leftWeb.defsLines.insert(*lit);
		if (old.usesLines.count(*lit)) leftWeb.usesLines.insert(*lit);
	}

	for (auto rit = it; rit != old.lines.end(); ++rit) {
		rightWeb.lines.insert(*rit);
		if (old.defsLines.count(*rit)) rightWeb.defsLines.insert(*rit);
		if (old.usesLines.count(*rit)) rightWeb.usesLines.insert(*rit);
	}

	g.removeVertex(bestWebId);
	g.addVertex(leftWeb.id);
	g.addVertex(rightWeb.id);

	for (const auto& other : allWebs) {
		if(other.id == bestWebId) continue;
		if (g.findVertex(other.id) == nullptr) continue;

		if (websIntersection(leftWeb, other)) {
			g.addEdge(leftWeb.id, other.id, 1);
			g.addEdge(other.id,leftWeb.id, 1);
		}

		if (websIntersection(rightWeb, other)) {
			g.addEdge(rightWeb.id, other.id, 1);
			g.addEdge(other.id,rightWeb.id, 1);
		}
	}

	for (auto& w : allWebs)
		if (w.id == bestWebId) {
			w = leftWeb;
			break;
		}
	allWebs.push_back(rightWeb);

	return 0;
}

int splitMiddle(Graph<int>& g, std::vector<Web>& allWebs, const int bestWebId) {

	Web* oldptr = nullptr;
	for (auto& w : allWebs) {				// Encontrar web
		if (w.id == bestWebId) {
			oldptr = &w;
			break;
		}
	}

	if (oldptr == nullptr) return -1;		// caso não encontre

	const Web old = *oldptr;			// Cópia pra não alterar allWebs

	if (old.lines.size() < 2) return -1;		// Verificar se tem pelo menos 2 linhas para poder dar split

	Web leftWeb(old.id, old.variable);
	Web rightWeb(g.getNumVertex(), old.variable);

	auto it = old.lines.begin();
	std::advance(it,old.lines.size() / 2);		// Avançar o iterador até a posição inicial da direita

	for (auto lit = old.lines.begin(); lit != it; ++lit) {
		leftWeb.lines.insert(*lit);
		if (old.defsLines.count(*lit)) leftWeb.defsLines.insert(*lit);
		if (old.usesLines.count(*lit)) leftWeb.usesLines.insert(*lit);
	}

	for (auto rit = it; rit != old.lines.end(); ++rit) {
		rightWeb.lines.insert(*rit);
		if (old.defsLines.count(*rit)) rightWeb.defsLines.insert(*rit);
		if (old.usesLines.count(*rit)) rightWeb.usesLines.insert(*rit);
	}

	g.removeVertex(bestWebId);
	g.addVertex(leftWeb.id);
	g.addVertex(rightWeb.id);

	for (const auto& other : allWebs) {
		if(other.id == bestWebId) continue;
		if (g.findVertex(other.id) == nullptr) continue;

		if (websIntersection(leftWeb, other)) {
			g.addEdge(leftWeb.id, other.id, 1);
			g.addEdge(other.id,leftWeb.id, 1);
		}

		if (websIntersection(rightWeb, other)) {
			g.addEdge(rightWeb.id, other.id, 1);
			g.addEdge(other.id,rightWeb.id, 1);
		}
	}

	for (auto& w : allWebs)
		if (w.id == bestWebId) {
			w = leftWeb;
			break;
		}
	allWebs.push_back(rightWeb);

	return 0;
}


AllocationResult splitWeb(Graph<int>& g, std::vector<Web>& allWebs, const int n, const int k) {    /*   WebInfo e allWebs tem de ser implementada por quem faz o parser */

    // Modificação do grafo original pois apenas é aplicada uma das abordagens (splitting ou spilling) por cada grafo
	AllocationResult result = basicColoring(g, k);

	if (result.success) return result;		// Verificação incial

	for (int tries = 0; tries < n; tries++) {

		int bestWebId = 0;
		if ((bestWebId = chooseWeb(g)) == -1) break;		// Erro se o grafo estiver vazio

		Web* web = nullptr;
		for (auto& w : allWebs) {				// Encontrar web
			if (w.id == bestWebId) {
				web = &w;
				break;
			}
		}

		if (web == nullptr) break;		// caso não encontre

		int bestGapId = 0;
		if ((bestGapId = findLargestGap(*web)) != -1) {
			splitGap(g, allWebs, bestWebId, bestGapId);
		}

		else {
			int ret = splitMiddle(g, allWebs, bestWebId);
			if (ret != 0) break;
		}

		result = basicColoring(g, k);
		if (result.success) {
			return result;
		}
	}

	// Caso falhe settar tudo a 0 e dar spill (mandar tudo para a meméria) e reportar erro

	result.success = false;
	result.registersUsed = 0;
	result.spilledWebs.clear();
	result.webToRegister.clear();
	for (auto* v : g.getVertexSet()) {
		int id  = v->getInfo();
		result.webToRegister[id] = -1;
		result.spilledWebs.push_back(id);
	}

	std::cerr << "Warning: register allocation with " << k << " registers and up to " << n << " splits is not possible.\n";

	return result;
}
