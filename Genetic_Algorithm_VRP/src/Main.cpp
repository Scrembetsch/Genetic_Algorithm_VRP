#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <random>

#include"genetic.h"

int main() {

	// TODO: multithreading

	int cityNb = 42;	// number of cities
	int route = 300;	// population size
	int ep = 5;
	int iteration = 200;

	std::string dataPath = "../Data/dantzig42_d.txt";

	std::vector<std::vector<int>> distance = inTxt(dataPath, cityNb);

	std::vector<std::vector<int>> population = initPath(route, cityNb);

	std::vector<std::vector<int>>::const_iterator pRow;
	std::vector<int>::const_iterator pCol;

	// Output
	// Gesamtdistanz aller Fahrzeuge: Distanz
	// Vehicle 1 (Gesamtdistanz): StadtA -> StadtB -> StadtC -> etc. ... -> StadtA
	// Vehicle 2 (Gesamtdistanz): StadtA -> StadtX -> StadtY -> etc. ... -> StadtA
	// Vehicle 3 (Gesamtdistanz): etc.
	// etc.
	std::ofstream outputP("../Output/population.txt");
	for (pRow = population.cbegin(); pRow != population.cend(); pRow++) {
		for (pCol = (*pRow).cbegin(); pCol != (*pRow).cend(); pCol++) {
			outputP << *pCol << " ";
		}
		outputP << std::endl;
	}

	std::vector<int> routeLength;
	for (int i = 0; i < route; i++) {
		routeLength.push_back(fitness(population[i], distance));
	}

	std::vector<double> range = choseRange(routeLength, ep);
	outputP << std::endl;
	outputP << "range:" << std::endl;
	std::vector<double>::const_iterator rIter = range.cbegin();
	for (; rIter != range.cend(); rIter++) {
		outputP << *rIter << " ";
	}

	std::vector<std::vector<int>> mini(iteration, std::vector<int>(cityNb));

	for (int j = 0; j < iteration; j++) {

		population = mutate(population, range, routeLength);	// eigentlich crossover
		population = change(population, 0.06, cityNb);			// eigentlich mutation

		std::vector<int> routeLength;
		for (int i = 0; i < (int)population.size(); i++) {
			routeLength.push_back(fitness(population[i], distance));
		}

		mini[j] = saveMin(population, routeLength);	// den besten von jeder Iteration abspeichern

		std::vector<double> range = choseRange(routeLength, ep);

		std::vector<std::vector<int>>::const_iterator spRow;
		std::vector<int>::const_iterator spCol;
		std::ofstream outputGen("./output/generation.txt");
		for (spRow = population.cbegin(); spRow != population.cend(); spRow++) {
			for (spCol = (*spRow).cbegin(); spCol != (*spRow).cend(); spCol++) {
				outputGen << *spCol << " ";
			}
			outputGen << std::endl;
		}
	}

	std::vector<int> finalLength;
	for (int i = 0; i < iteration; i++) {
		finalLength.push_back(fitness(mini[i], distance));
	}

	std::vector<int>::const_iterator fitIter = finalLength.cbegin();
	int minFit = *fitIter;
	// Besten Fitness Value ( = Länge) raussuchen und ausgeben
	for (; fitIter != finalLength.cend(); fitIter++) {
		if (*fitIter < minFit) {
			minFit = *fitIter;
		}
	}
	std::cout << "Minimum length: " << minFit << std::endl;

	// TODO: Output der von den Vehicles zurückgelegten Strecken

	return 0;
}

