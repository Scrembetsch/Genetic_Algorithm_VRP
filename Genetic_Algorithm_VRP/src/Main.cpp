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

//const std::string sInputFile = "Data/Romania.txt";
const std::string sInputFile = "Data/US.txt";

int NumThreads = 4;
bool VisualMode = false;

void LoadArguments(int argc, char** argv)
{
	ArgumentParser parser(argc, argv);

	NumThreads = parser.GetInt("-t", "--threads", std::thread::hardware_concurrency());	// Get number of allowed threads, default is maximum number of threads
	VisualMode = parser.CheckIfExists("-v", "--visual");	// Check if visual output should be shown (Only available on Windows x64)

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
	Timing::getInstance()->startComputation();
#pragma omp parallel for
	for (int i = 0; i < NumThreads; i++)
	{
		// Start needed threads
		//std::cout << "Started Thread " << std::to_string(i) << std::endl;
		algos[i]->SolveVRP();
	}
	Timing::getInstance()->stopComputation();

	std::cout << "Calculated " << algos[0]->mIterations << " iterations in ";
	Timing::getInstance()->print(true);
	std::cout << std::endl;

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

	int* solution = algos[bestIndex]->GetBest();

	std::cout << "Best solution found in Thread: " << bestIndex << std::endl;
	algos[bestIndex]->PrintOutput(solution);

	if (VisualMode)
	{
#ifdef _WIN64
		// Set all graph values and draw graph
		GraphDrawer graph;

		// Convert all cities with their coordinates
		std::vector<std::pair<float, float>> cityLocations;
		for (const auto& city : algos[bestIndex]->mCities)
		{
			cityLocations.push_back(std::make_pair(city.X, city.Y));
		}
		graph.SetPoints(cityLocations);
		graph.SetDepot(solution[0]);
		graph.RescalePoints();

		// Convert all routes of all vehicles
		std::vector<std::vector<int>> routes;
		int route = 0;
		routes.push_back(std::vector<int>());
		routes[0].push_back(solution[0]);
		for (size_t i = 1; i < algos[bestIndex]->mRouteSize; i++)
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
#else
		std::cout << "Visual mode not supported on this platform!" << std::endl;
#endif
	}

	// Cleanup
	for (size_t i = 0; i < algos.size(); i++)
	{
		delete algos[i];
	}
	algos.clear();
	Timing::getInstance()->clear();
	return 0;
}

