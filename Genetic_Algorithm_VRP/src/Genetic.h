#include <vector>
#include <random>

std::vector<std::vector<int>> inTxt(std::string path, int cityNb);

std::vector<std::vector<int>> initPath(int route, int nb);

int fitness(const std::vector<int>& sroute, const std::vector<std::vector<int>>& length);

std::vector<double> choseRange(std::vector<int> fitness, int ep);	// TODO: delete

std::vector<int> inheritance(const std::vector<int>& father, std::vector<int> mother);

std::vector<std::vector<int>> mutate(const std::vector<std::vector<int>>& population, const std::vector<double>& range, const std::vector<int>& fitness);

std::vector<std::vector<int>> change(std::vector<std::vector<int>> population, double changeRate, int cityNb);

std::vector<int> saveMin(const std::vector<std::vector<int>>& population, const std::vector<int>& fitness);