#ifndef WEB_BUILDER_H
#define WEB_BUILDER_H

#include "Graph.h"
#include "parser.h"
#include <vector>
#include <map>

/**
 * @brief Builds webs from live ranges and constructs the interference graph.
 *
 * A web is the union of live ranges of the same variable that overlap
 * at any program point. Two webs interfere if they are simultaneously
 * live at any program point.
 */
class WebBuilder {
public:
    /**
     * @brief Merges live ranges into webs for each variable.
     *
     * Live ranges of the same variable are merged into a single web
     * if they share any program line. Uses a greedy union approach.
     *
     * Time complexity: O(R^2 * L) where R = number of ranges, L = lines per range.
     *
     * @param ranges  All live ranges parsed from input
     * @return        Vector of webs (each with a unique ID)
     */
    static std::vector<Web> buildWebs(const std::vector<LiveRange>& ranges);

    /**
     * @brief Builds the interference graph from a set of webs.
     *
     * Adds one vertex per web. Adds a bidirectional edge between two webs
     * if they interfere (i.e., are simultaneously live at some point).
     *
     * Special case: a web A starting at line X (definition) and web B
     * ending at line X (use) do NOT interfere at that point.
     *
     * Time complexity: O(W^2 * L) where W = number of webs, L = lines per web.
     *
     * @param webs  Vector of webs
     * @param g     Output interference graph (Graph<int>, node = web ID)
     */
    static void buildInterferenceGraph(const std::vector<Web>& webs, Graph<int>& g);
};

#endif // WEB_BUILDER_H
