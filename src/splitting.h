#ifndef SPLITTING_H
#define SPLITTING_H

#include "Graph.h"
#include <vector>

/* ========= Structs utilizadas apenas para fase inicial de desenvolvimento ==========*/

struct Interval {
    int start;
    int end;
};

struct WebInfo {
    int id;
    std::vector<Interval> intervals;
};

/* ===================================== Até aqui ===================================== */

int chooseWeb(Graph<int>& g);       // Greedy escolher a web que têm amior innterseção de webs
int findLargestGap(const WebInfo& web);                // De perferencia o allWebs tem como indice no vetor o ID do vertex/Web
bool websIntersection(const WebInfo& web1, const WebInfo& web2);
int splitGap(Graph<int>& g, std::vector<WebInfo>& allWebs, const int bestWebId, const int bestGapId);       // Dividir no Gap
int splitMiddle(Graph<int>& g, std::vector<WebInfo>& allWebs, const int bestWebId);
int splitWeb(Graph<int>& g, std::vector<WebInfo>& allWebs, const int n, const int k);     // Splittar a web

#endif //SPLITTING_H