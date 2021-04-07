#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <random>
#include <omp.h>
#include <thread>

#include "Genetic.h"
#include "Timing.h"

#ifdef _WIN32
const std::string sPrefix = "../";
#else
const std::string sPrefix;
#endif // _WIN32

const std::string sInputFile = "Data/Romania.txt";
//const std::string sInputFile = "Data/US.txt";

int NumThreads = 4;

void LoadArguments(int argc, char** argv)
{
    bool threadsProvided = false;

    for (int i = 1; i < argc; i++)
    {
        std::string arg(argv[i]);
        if (arg == "--threads")
        {
            if ((i + 1) < argc)
            {
                NumThreads = std::stoi(argv[i + 1]);
                threadsProvided = true;
            }
        }
    }
    if (!threadsProvided)
    {
        NumThreads = std::thread::hardware_concurrency();
    }
	std::cout << "Using Threads: " << NumThreads << std::endl;
}

int main(int argc, char** argv)
{
	LoadArguments(argc, argv);

	std::vector<GeneticAlgorithm> algos;

	// Create default object and read file one time, then copy
	algos.push_back(GeneticAlgorithm());
	algos[0].ReadFile(sPrefix + sInputFile, true);
	
	// Copy object with parsed input
	for (int i = 1; i < NumThreads; i++)
	{
		algos.push_back(GeneticAlgorithm(algos[0]));
	}

#pragma omp parallel for
	for (int i = 0; i < NumThreads; i++)
	{
		// Start needed threads
		std::cout << "Started Thread " << std::to_string(i) << std::endl;
		algos[i].SolveVRP();
	}

	// Find best solution in all started threads
	int bestFitness = INT32_MAX;
	int bestIndex = 0;
	for (int i = 0; i < NumThreads; i++)
	{
		int fitness = algos[i].EvaluateFitness(algos[i].GetBest());
		if (fitness < bestFitness)
		{
			bestFitness = fitness;
			bestIndex = i;
		}
	}

	const std::vector<int>& solution = algos[bestIndex].GetBest();

	// Output - Test
	//algo.mDepot = 0;
	//std::vector<int> solution = { 1,2,GeneticAlgorithm::sBlank, 10, 11,12,14,GeneticAlgorithm::sBlank, 13,15,7,8 };
	// Print best solution
	algos[bestIndex].PrintOutput(solution);

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

