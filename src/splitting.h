#ifndef SPLITTING_H
#define SPLITTING_H
#include "Graph.h"
#include <utility>

int chooseWeb(Graph<int>& g);       // Greedy escolher a web que têm amior innterseção de webs
std::pair<int,int> splitWeb();      // Splittar nos intervalos onde a variavel não é usado o maior tempo possivel

#endif //SPLITTING_H