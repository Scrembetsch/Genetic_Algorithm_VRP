#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <random>
#include <omp.h>
#include <thread>

#include "ArgumentParser.h"
#include "Genetic.h"
#include "GraphDrawer.h"
#include "Timing.h"

#ifdef _WIN32
const std::string sPrefix = "../";
#else
const std::string sPrefix;
#endif // _WIN32

const std::string sInputFile = "Data/Romania.txt";
//const std::string sInputFile = "Data/US.txt";

int NumThreads = 4;
bool VisualMode = false;

void LoadArguments(int argc, char** argv)
{
	ArgumentParser parser(argc, argv);

	NumThreads = parser.GetInt("-t", "--threads", std::thread::hardware_concurrency());
	VisualMode = parser.CheckIfExists("-v", "--visual");

	std::cout << "Using Threads: " << NumThreads << std::endl << std::endl;
}

int main(int argc, char** argv)
{
	LoadArguments(argc, argv);

	std::vector<GeneticAlgorithm*> algos;

	// Create default object and read file one time, then copy
	algos.push_back(new GeneticAlgorithm());
	algos[0]->ReadFile(sPrefix + sInputFile, true);

	// Copy object with parsed input
	for (int i = 1; i < NumThreads; i++)
	{
		algos.push_back(new GeneticAlgorithm(*algos[0]));
	}

#pragma omp parallel for
	for (int i = 0; i < NumThreads; i++)
	{
		// Start needed threads
		//std::cout << "Started Thread " << std::to_string(i) << std::endl;
		algos[i]->SolveVRP();
	}

	// Find best solution in all started threads
	int bestFitness = INT32_MAX;
	int bestIndex = 0;
	for (int i = 0; i < NumThreads; i++)
	{
		int fitness = algos[i]->EvaluateFitness(algos[i]->GetBest());
		if (fitness < bestFitness)
		{
			bestFitness = fitness;
			bestIndex = i;
		}
	}

	const std::vector<int>& solution = algos[bestIndex]->GetBest();

	// Output - Test
	//algo.mDepot = 0;
	//std::vector<int> solution = { 1,2,GeneticAlgorithm::sBlank, 10, 11,12,14,GeneticAlgorithm::sBlank, 13,15,7,8 };
	// Print best solution
	std::cout << "Best solution found in Thread: " << bestIndex << std::endl;
	algos[bestIndex]->PrintOutput(solution);

	// Dummy solution
	//std::vector<int> solution;
	//for (int i = 0; i < algos[0].mNumCities; i++)
	//{
	//	solution.push_back(i);
	//}
	//std::random_device rd;
	//std::mt19937 g(rd());
	//std::shuffle(solution.begin(), solution.end(), g);
	//int bestIndex = 0;
	//solution.insert(solution.begin() + 1, GeneticAlgorithm::sBlank);
	//solution.insert(solution.begin() + 5, GeneticAlgorithm::sBlank);
	//solution.insert(solution.begin() + 10, GeneticAlgorithm::sBlank);
	//solution.insert(solution.begin() + 14, GeneticAlgorithm::sBlank);
	//solution.insert(solution.begin() + 18, GeneticAlgorithm::sBlank);

	if (VisualMode)
	{
#ifdef _WIN64
		GraphDrawer graph;

		std::vector<std::pair<float, float>> cityLocations;
		for (const auto& city : algos[bestIndex]->mCities)
		{
			cityLocations.push_back(std::make_pair(city.X, city.Y));
		}
		graph.SetPoints(cityLocations);
		graph.SetDepot(solution[0]);
		graph.RescalePoints();

		std::vector<std::vector<int>> routes;
		int route = 0;
		routes.push_back(std::vector<int>());
		routes[0].push_back(solution[0]);
		for (size_t i = 1; i < solution.size(); i++)
		{
			if (solution[i] == GeneticAlgorithm::sBlank)
			{
				routes[route].push_back(solution[0]);
				route++;
				routes.push_back(std::vector<int>());
				routes[route].push_back(solution[0]);
			}
			else
			{
				routes[route].push_back(solution[i]);
			}
		}
		routes[route].push_back(solution[0]);
		graph.SetRoutes(routes);
		graph.Draw();
#endif
	}

	for (size_t i = 0; i < algos.size(); i++)
	{
		delete algos[i];
	}
	algos.clear();
	return 0;

	// TODO: Delete this old stuff

	//int cityNb = 42;	// number of cities
	//int route = 300;	// population size
	//int ep = 5;
	//int iteration = 200;

	//std::string dataPath = "../Data/dantzig42_d.txt";

	////std::vector<std::vector<int>> distance = inTxt(dataPath, cityNb);

	////std::vector<std::vector<int>> population = initPath(route, cityNb);

	//std::vector<std::vector<int>>::const_iterator pRow;
	//std::vector<int>::const_iterator pCol;

	//// Output
	//// Gesamtdistanz aller Fahrzeuge: Distanz
	//// Vehicle 1 (Gesamtdistanz): StadtA -> StadtB -> StadtC -> etc. ... -> StadtA
	//// Vehicle 2 (Gesamtdistanz): StadtA -> StadtX -> StadtY -> etc. ... -> StadtA
	//// Vehicle 3 (Gesamtdistanz): etc.
	//// etc.
	//std::ofstream outputP("../Output/population.txt");
	//for (pRow = population.cbegin(); pRow != population.cend(); pRow++) {
	//	for (pCol = (*pRow).cbegin(); pCol != (*pRow).cend(); pCol++) {
	//		outputP << *pCol << " ";
	//	}
	//	outputP << std::endl;
	//}

	//std::vector<int> routeLength;
	//for (int i = 0; i < route; i++) {
	//	routeLength.push_back(fitness(population[i], distance));
	//}

	//std::vector<double> range = choseRange(routeLength, ep);
	//outputP << std::endl;
	//outputP << "range:" << std::endl;
	//std::vector<double>::const_iterator rIter = range.cbegin();
	//for (; rIter != range.cend(); rIter++) {
	//	outputP << *rIter << " ";
	//}

	//std::vector<std::vector<int>> mini(iteration, std::vector<int>(cityNb));

	//for (int j = 0; j < iteration; j++) {

	//	population = mutate(population, range, routeLength);	// eigentlich crossover
	//	population = change(population, 0.06, cityNb);			// eigentlich mutation

	//	std::vector<int> routeLength;
	//	for (int i = 0; i < (int)population.size(); i++) {
	//		routeLength.push_back(fitness(population[i], distance));
	//	}

	//	mini[j] = saveMin(population, routeLength);	// den besten von jeder Iteration abspeichern

	//	std::vector<double> range = choseRange(routeLength, ep);

	//	std::vector<std::vector<int>>::const_iterator spRow;
	//	std::vector<int>::const_iterator spCol;
	//	std::ofstream outputGen("./output/generation.txt");
	//	for (spRow = population.cbegin(); spRow != population.cend(); spRow++) {
	//		for (spCol = (*spRow).cbegin(); spCol != (*spRow).cend(); spCol++) {
	//			outputGen << *spCol << " ";
	//		}
	//		outputGen << std::endl;
	//	}
	//}

	//std::vector<int> finalLength;
	//for (int i = 0; i < iteration; i++) {
	//	finalLength.push_back(fitness(mini[i], distance));
	//}

	//std::vector<int>::const_iterator fitIter = finalLength.cbegin();
	//int minFit = *fitIter;
	//// Besten Fitness Value ( = Länge) raussuchen und ausgeben
	//for (; fitIter != finalLength.cend(); fitIter++) {
	//	if (*fitIter < minFit) {
	//		minFit = *fitIter;
	//	}
	//}
	//std::cout << "Minimum length: " << minFit << std::endl;

	//// TODO: Output der von den Vehicles zurückgelegten Strecken

	//return 0;
}

