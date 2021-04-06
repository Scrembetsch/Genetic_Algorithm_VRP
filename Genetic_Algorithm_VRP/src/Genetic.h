#include <vector>
#include <random>


using namespace std;


vector<vector<int>> inTxt(string path, int cityNb);

vector<vector<int>> initPath(int route, int nb);

int fitness(vector<int> sroute, vector<vector<int>> length);

vector<double> choseRange(vector<int> fitness, int ep);

vector<int> inheritance(vector<int> father, vector<int> mother);

vector<vector<int>> mutate(vector<vector<int>> population, vector<double> range, vector<int> fitness);

vector<vector<int>> change(vector<vector<int>> population, double changeRate, int cityNb);

vector<int> saveMin(vector<vector<int>> population, vector<int> fitness);