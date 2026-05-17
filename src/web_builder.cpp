/**
 * @file web_builder.cpp
 * @brief Builds webs and interference graph from live ranges.
 */
#include "web_builder.h"
#include <algorithm>

// ============================================================
// Build webs from live ranges
// ============================================================

/**
 * @brief Merges live ranges of the same variable into webs.
 *
 * Algorithm:
 *   For each variable, collect all its live ranges.
 *   Use a greedy union: start a new web with the first range,
 *   then for each subsequent range of the same variable,
 *   if it shares any line with an existing web, merge into it.
 *   Otherwise start a new web.
 *   Repeat until no more merges occur (transitive closure).
 */
std::vector<Web> WebBuilder::buildWebs(const std::vector<LiveRange>& ranges) {
    // Group ranges by variable
    std::map<std::string, std::vector<const LiveRange*>> byVariable;
    for (const auto& r : ranges) {
        byVariable[r.variable].push_back(&r);
    }

    std::vector<Web> webs;
    int nextId = 0;

    for (auto& [variable, varRanges] : byVariable) {
        // Start with one web per range, then merge
        std::vector<Web> varWebs;
        for (const auto* r : varRanges) {
            Web w(nextId++, variable);
            w.addRange(*r);
            varWebs.push_back(w);
        }

        // Merge webs that share lines (greedy, repeat until stable)
        bool merged = true;
        while (merged) {
            merged = false;
            for (size_t i = 0; i < varWebs.size(); i++) {
                for (size_t j = i + 1; j < varWebs.size(); j++) {
                    // Check if they share any line
                    bool share = false;
                    for (int line : varWebs[i].lines) {
                        if (varWebs[j].lines.count(line)) {
                            share = true;
                            break;
                        }
                    }
                    if (share) {
                        // Merge j into i
                        varWebs[i].lines.insert(
                            varWebs[j].lines.begin(),
                            varWebs[j].lines.end()
                        );
                        varWebs[i].defsLines.insert(
                            varWebs[j].defsLines.begin(),
                            varWebs[j].defsLines.end()
                        );
                        varWebs[i].useslines.insert(
                            varWebs[j].usesLines.begin(),
                            varWebs[j].usesLines.end()
                        );
                        varWebs.erase(varWebs.begin() + j);
                        merged = true;
                        break;
                    }
                }
                if (merged) break;
            }
        }

        // Re-assign IDs sequentially after merging
        for (auto& w : varWebs) {
            w.id = (int)webs.size();
            webs.push_back(w);
        }
    }

    return webs;
}

// ============================================================
// Build interference graph
// ============================================================

/**
 * @brief Builds interference graph from webs.
 *
 * Two webs interfere if they are simultaneously live at any point.
 * Special case: web A defined at line X and web B last used at line X
 * do NOT interfere (definition kills the previous value).
 */
void WebBuilder::buildInterferenceGraph(const std::vector<Web>& webs, Graph<int>& g) {
    // Add one vertex per web
    for (const auto& w : webs) {
        g.addVertex(w.id);
    }

    // Check each pair of webs for interference
    for (size_t i = 0; i < webs.size(); i++) {
        for (size_t j = i + 1; j < webs.size(); j++) {
            const Web& a = webs[i];
            const Web& b = webs[j];

            if (a.lines.empty() || b.lines.empty()) continue;

            // Quick bounds check
            if (*a.lines.rbegin() < *b.lines.begin() ||
                *b.lines.rbegin() < *a.lines.begin()) {
                continue;
            }

            // Find common lines
            std::vector<int> common;
            std::set_intersection(
                a.lines.begin(), a.lines.end(),
                b.lines.begin(), b.lines.end(),
                std::back_inserter(common)
            );

            if (common.empty()) continue;

            // Check special case: if the ONLY common line is one where
            // one web starts with a definition and the other ends with a use,
            // they do NOT interfere at that point.
            bool interfere = false;
            for (int line : common) {
                bool aDefHere = a.hasDefAt(line);
                bool bUseHere = b.hasUseAt(line);
                bool bDefHere = b.hasDefAt(line);
                bool aUseHere = a.hasUseAt(line);

                // They don't interfere at this line only if:
                // (A is defined here AND B ends its use here) OR
                // (B is defined here AND A ends its use here)
                bool noInterferenceAtLine =
                    (aDefHere && bUseHere) || (bDefHere && aUseHere);

                if (!noInterferenceAtLine) {
                    interfere = true;
                    break;
                }
            }

            if (interfere) {
                g.addBidirectionalEdge(a.id, b.id, 1.0);
            }
        }
    }
}
