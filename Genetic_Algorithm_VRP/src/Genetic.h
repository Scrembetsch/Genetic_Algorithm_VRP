#include <vector>
#include <random>


using namespace std;


vector<vector<int>> inTxt(string path, int cityNb);

vector<vector<int>> initPath(int route, int nb);

int fitness(const vector<int>& sroute, const vector<vector<int>>& length);

vector<double> choseRange(vector<int> fitness, int ep);	// TODO: delete

vector<int> inheritance(const vector<int>& father, vector<int> mother);

vector<vector<int>> mutate(const vector<vector<int>>& population, const vector<double>& range, const vector<int>& fitness);

vector<vector<int>> change(vector<vector<int>> population, double changeRate, int cityNb);

vector<int> saveMin(const vector<vector<int>>& population, const vector<int>& fitness);