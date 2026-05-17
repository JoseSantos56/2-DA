#ifndef SPLITTING_H
#define SPLITTING_H

#include "Graph.h"
#include "parser.h"
#include <vector>


int chooseWeb(Graph<int>& g);       // Greedy escolher a web que têm amior innterseção de webs
int findLargestGap(const Web& web);                // De perferencia o allWebs tem como indice no vetor o ID do vertex/Web
bool websIntersection(const Web& web1, const Web& web2);
int splitGap(Graph<int>& g, std::vector<Web>& allWebs, const int bestWebId, const int bestGapId);       // Dividir no Gap
int splitMiddle(Graph<int>& g, std::vector<Web>& allWebs, const int bestWebId);
AllocationResult splitWeb(Graph<int>& g, std::vector<Web>& allWebs, const int n, const int k);     // Splittar a web

#endif //SPLITTING_H