#ifndef SPLITTING_H
#define SPLITTING_H

#include "Graph.h"
#include <vector>

/* ========= Structs utilizadas apenas para fase inicial de desenvolvimento ==========*/

struct interval {
    int start;
    int end;
};

struct webInfo {
    int id;
    std::vector<interval> intervals;
};

/* ===================================== Até aqui ===================================== */

int chooseWeb(Graph<int>& g);       // Greedy escolher a web que têm amior innterseção de webs
int findLargestGap(const webInfo& web);                // De perferencia o allWebs tem como indice no vetor o ID do vertex/Web
bool websIntersection(const webInfo& web1, const webInfo& web2);
int splitGap(Graph<int>& g, std::vector<webInfo>& allWebs, const int bestWebId, const int bestGapId);
int splitMiddle(Graph<int>& g, std::vector<webInfo>& allWebs, const int bestWebId);
int splitWeb(Graph<int>& g, std::vector<webInfo>& allWebs, const int k);     // Splittar nos intervalos onde a variável não é usada o maior tempo possível

#endif //SPLITTING_H