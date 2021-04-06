#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <random>

#include"genetic.h"

using namespace std;
using std::vector;


int main(int argc, char * argv[]) {

	int cityNb = 42;
	int route = 300;
	int ep = 5;
	int iteration = 200;

	string dataPath = "../Data/dantzig42_d.txt";

	vector<vector<int>> distance = inTxt(dataPath, cityNb);

	vector<vector<int>> population = initPath(route, cityNb);

	vector<vector<int>>::const_iterator pRow;
	vector<int>::const_iterator pCol;

	ofstream outputP("../Output/population.txt");
	for (pRow = population.cbegin(); pRow != population.cend(); pRow++) {
		for (pCol = (*pRow).cbegin(); pCol != (*pRow).cend(); pCol++) {
			outputP << *pCol << " ";
		}
		outputP << endl;
	}

	vector<int> routeLength;
	for (int i = 0; i < route; i++) {
		routeLength.push_back(fitness(population[i], distance));
	}

	vector<double> range = choseRange(routeLength, ep);
	outputP << endl;
	outputP << "range:" << endl;
	vector<double>::const_iterator rIter = range.cbegin();
	for (; rIter != range.cend(); rIter++) {
		outputP << *rIter << " ";
	}

	vector<vector<int>> mini(iteration, vector<int>(cityNb));

	for (int j = 0; j < iteration; j++) {

		population = mutate(population, range, routeLength);
		population = change(population, 0.06, cityNb);

		vector<int> routeLength;
		for (int i = 0; i < population.size(); i++) {
			routeLength.push_back(fitness(population[i], distance));
		}

		mini[j] = saveMin(population, routeLength);

		vector<double> range = choseRange(routeLength, ep);

		vector<vector<int>>::const_iterator spRow;
		vector<int>::const_iterator spCol;
		ofstream outputGen("./output/generation.txt");
		for (spRow = population.cbegin(); spRow != population.cend(); spRow++) {
			for (spCol = (*spRow).cbegin(); spCol != (*spRow).cend(); spCol++) {
				outputGen << *spCol << " ";
			}
			outputGen << endl;
		}
	}

	vector<int> finalLength;
	for (int i = 0; i < iteration; i++) {
		finalLength.push_back(fitness(mini[i], distance));
	}

	vector<int>::const_iterator fitIter = finalLength.cbegin();
	int minFit = *fitIter;
	int index = 0;
	for (; fitIter != finalLength.cend(); fitIter++) {
		if (*fitIter < minFit) {
			minFit = *fitIter;
			index = fitIter - finalLength.cbegin();
		}
	}
	cout << "Minimum length: " << minFit << endl;

	return 0;
}

