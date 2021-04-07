#include<iostream>
#include<vector>

// Code from https://algotree.org/algorithms/single_source_shortest_path/dijkstras_shortest_path_c++/

class PathFinder
{
public:
    using PII = std::pair<int, int>;
    using VPII = std::vector<PII>;
    using VVPII = std::vector<VPII>;

    VVPII Graph;

    std::vector<int> ShortestPath(int start);
};

