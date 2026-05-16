#ifndef DA_CUSTOMALLOCATOR_H
#define DA_CUSTOMALLOCATOR_H

#include "Graph.h"
#include <set>
#include <vector>

struct ColoringState {
    std::vector<int> colors;             //  -1 se não estiver colorido
    std::vector<bool> spilled;
    std::vector<bool> processed;
    std::vector<std::set<bool>> adjColors;
    std::vector<int> saturation;
    std::set<bool> unprocessed;
};

int chooseNextWeb(Graph<int>& g, const ColoringState& state);       // Com base em DSatur usando o vertice de maior saturação
bool isColorAvailable(ColoringState& state, const int webId, const int color);
void updateSaturation(Graph<int>& g, ColoringState& state, const int webId, const int color);         // Update da saturação dos vizinhos do vértice
bool backtrackDSatur(Graph<int>& g, const int k, ColoringState& state, int spillsLeft);           // Algoritmo DSatur + backtracking (Escolher o vértice com maior saturação, combinando um aboradegem de backtracking)
ColoringState initState(Graph<int>& g, int v);              // Incialização do parametro state
bool greedyDSatur(Graph<int>& g, int k, std::vector<int>& colors, std::vector<bool>& spilled);          // Abordagem hibrida greedy com decisão baseada em DSatur
bool customAllocate(Graph<int>& g, int k, std::vector<int>& colors, std::vector<bool>& spilled);        // Algoritmo final junta todas as funções

#endif //DA_CUSTOMALLOCATOR_H