#include "PathFinder.h"

#include<set>

std::vector<int> PathFinder::ShortestPath(int start)
{
    // Assume that the distance from source_node to other nodes is infinite
    // in the beginning, i.e initialize the distance vector to a max value
    const int INF = 9999999;
    std::vector<int> dist(Graph.size(), INF);
    std::set<PII> set_length_node;

    // Distance from starting vertex to itself is 0
    dist[start] = 0;
    set_length_node.insert(PII(0, start));

    while (!set_length_node.empty())
    {

        PII top = *set_length_node.begin();
        set_length_node.erase(set_length_node.begin());

        int source_node = top.second;

        for (const auto& it : Graph[source_node])
        {

            int adj_node = it.first;
            int length_to_adjnode = it.second;

            // Edge relaxation
            if (dist[adj_node] > length_to_adjnode + dist[source_node])
            {

                // If the distance to the adjacent node is not INF, means the pair <dist, node> is in the set
                // Remove the pair before updating it in the set.
                if (dist[adj_node] != INF)
                {
                    set_length_node.erase(set_length_node.find(PII(dist[adj_node], adj_node)));
                }
                dist[adj_node] = length_to_adjnode + dist[source_node];
                set_length_node.insert(PII(dist[adj_node], adj_node));
            }
        }
    }

    return dist;
}