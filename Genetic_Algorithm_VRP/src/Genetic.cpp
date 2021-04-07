#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <cmath>
#include <map>

#include "Genetic.h"
#include "PathFinder.h"
#include "Timing.h"
#include "Util.h"

bool Road::Parse(const std::string& roadString)
{
	// Find important part
	size_t openingBracket = roadString.find('(');
	size_t closingBracket = roadString.find(')', openingBracket);

	// Find first letter (non-whitespace) & end of city
	size_t begin = Util::FindNextNonWhitespace(roadString, openingBracket);
	size_t comma = roadString.find(',', begin);
	City1 = roadString.substr(begin, comma - begin);

	// Find next first letter & end of city
	begin = Util::FindNextNonWhitespace(roadString, comma);
	comma = roadString.find(',', begin);
	City2 = roadString.substr(begin, comma - begin);

	// Find first digit
	begin = Util::FindNextNonWhitespace(roadString, comma);
	Distance = std::stoi(roadString.substr(begin, closingBracket - begin));

	return true;
}

GeneticAlgorithm::GeneticAlgorithm()
	: mDistances(nullptr)
	, mNumCities(0)
	, mPopulationSize(300)
	, mIterations(200)
	, mMutationRate(0.1)
	, mBestSolutions(mIterations, std::vector<int>(mNumCities))
{
}

GeneticAlgorithm::~GeneticAlgorithm()
{
	if (mDistances != nullptr)
	{
		for (int i = 0; i < mNumCities; i++)
		{
			delete[] mDistances[i];
		}
		delete[] mDistances;
	}
}

void GeneticAlgorithm::SolveVRP()
{
	std::vector<std::vector<int>> population = InitPopulation();

	std::vector<int> routeLength;
	for (int i = 0; i < mPopulationSize; i++) 
	{
		routeLength.push_back(EvaluateFitness(population[i]));
	}

	// TODO: Multithreading
	for (int j = 0; j < mIterations; j++) 
	{

		population = CreateNewGeneration(population, routeLength);
		population = Mutate(population);

		std::vector<int> routeLength;
		for (int i = 0; i < (int)population.size(); i++) 
		{
			routeLength.push_back(EvaluateFitness(population[i]));
		}

		mBestSolutions[j] = SaveBest(population, routeLength);	// Save the best of each iteration
	}
}

bool GeneticAlgorithm::ReadFile(std::string path, bool calculateMissingRoutes)
{
	std::vector<Road> roads;
	std::map<std::string, int> cityMap;
	int cityCounter = 0;

	// Load important data from file
	std::ifstream file(path);
	std::string line;
	while (getline(file, line))
	{
		size_t commentIndex = line.find('%');
		if (commentIndex == 0U)	// Check if whole line is comment
		{
			continue;
		}

		if (Util::StartsWith(line, "road"))		// Load road if line starts with road
		{
			size_t end = std::string::npos;
			size_t offset = 0U;
			while ((end = line.find('.', offset)) != std::string::npos	// Find end of road, offset is used if multiple roads are in one line
				   && end < commentIndex								// Check if found end is not after comment
				   )
			{
				roads.push_back(Road());
				roads[roads.size() - 1].Parse(line.substr(offset, end));	// Parse road string
				offset = end + 1;
			}
		}
		else if (Util::StartsWith(line, "city"))	// Load city
		{
			size_t begin = line.find('(') + 1;
			size_t end = line.find(',');
			std::string city = line.substr(begin, end - begin);
			cityMap.insert(std::pair<std::string, int>(city, cityCounter++));
			mCities.push_back(city);
		}
	}
	file.close();

	// Create array
	mNumCities = cityCounter;
	mDistances = new int* [mNumCities];
	for (int i = 0; i < mNumCities; i++)
	{
		mDistances[i] = new int[mNumCities];
		for (int j = 0; j < mNumCities; j++)
		{
			mDistances[i][j] = i == j ? 0 : -1;
		}
	}

	for (const auto& road : roads)
	{
		int index1 = cityMap.at(road.City1);
		int index2 = cityMap.at(road.City2);

		mDistances[index1][index2] = road.Distance;
		mDistances[index2][index1] = road.Distance;
	}

	//PrintDistances();
	if (calculateMissingRoutes)
	{
		// Create adjMatrix for PathFinder
		PathFinder graph;
		for (int i = 0; i < mNumCities; i++)
		{
			PathFinder::VPII a;
			for (int j = 0; j < mNumCities; j++)
			{
				if (mDistances[i][j] != 0 && mDistances[i][j] != -1)
				{
					a.push_back(PathFinder::PII(j, mDistances[i][j]));
				}
			}
			graph.Graph.push_back(a);
		}

		// Find and write shortest routes
		for (int i = 0; i < mNumCities; i++)
		{
			std::vector<int> dist = graph.ShortestPath(i);
			for (int j = 0; j < mNumCities; j++)
			{
				mDistances[i][j] = dist[j];
			}
		}

		PrintDistances();
	}
	// Debug print
	//std::cout << "Roads:" << std::endl;
	//for (const auto& road : roads)
	//{
	//	std::cout << road.City1 << "," << road.City2 << "," << std::to_string(road.Distance) << ";" << std::endl;
	//}
	//std::cout << "--------------------------" << std::endl;
	//std::cout << "Cities:" << std::endl;
	//for (const auto& city : cities)
	//{
	//	std::cout << city.first << ";" << std::endl;
	//}
	//std::cout << "--------------------------" << std::endl;

	return true;
}

void GeneticAlgorithm::PrintDistances()
{
	std::cout << "Distances:" << std::endl;
	std::cout << "  \t";
	for (int i = 0; i < mNumCities; i++)
	{
		std::cout << char('A' + i) << " \t";
	}
	std::cout << std::endl;
	for (int i = 0; i < mNumCities; i++)
	{
		std::cout << char('A' + i) << " \t";
		for (int j = 0; j < mNumCities; j++)
		{
			std::cout << std::to_string(mDistances[i][j]) << " \t";
		}
		std::cout << std::endl;
	}
}

std::vector<std::vector<int>> GeneticAlgorithm::InitPopulation() 
{
	std::vector<std::vector<int>> population(mPopulationSize, std::vector<int>(mNumCities));
	// std::vector<int>(routeVehicle1|blank|routeVehicle2|blank|routeVehicle3|blank|routeVehicle4|blank|routeVehicle5)	-> siehe Discord Paper
	// Wichtig: gültige initiale Population erzeugen, sodass alle 5 Fahrzeuge ungefähr die gleiche Anzahl an Städten befahren

	std::default_random_engine generator(std::random_device{}());

	int city;
	for (int i = 0; i < mPopulationSize; i++)
	{

		std::vector<int> place;
		for (int k = 0; k < mNumCities; k++) 
		{
			place.push_back(k);
		}

		int s = mNumCities;
		for (int j = 0; j < mNumCities; j++) 
		{


			std::uniform_int_distribution<int> distribution(0, s - 1);

			city = distribution(generator);
			population[i][j] = place[city];

			int temp = place[s - 1];
			place[city] = temp;
			place.pop_back();
			s = s - 1;

		}
	}

	return population;
}

int GeneticAlgorithm::EvaluateFitness(const std::vector<int>& populationRoute) const
{
	// Mittelwert der zurückgelegten Distanz der Trucks berechnen = Summe der einzelnen Truck-Distanzen/5
	// Fitness ist Gewichtung1 (z.B. 0.5) * Summe der einzelnen Truck-Distanzen)/5 + Gewichtung2 (z.B. 0.5) * Gesamtstrecke aller Trucks
	// Fitness Wert soll möglichst klein sein
	// TODO: Wenn -1 ausgelesen wird -> durch INT_MAX ersetzen
	int s = populationRoute.size();

	int routeLength = 0;
	for (int i = 1; i < s; i++) 
	{
		routeLength += mDistances[populationRoute[i - 1]][populationRoute[i]];
	}
	routeLength += mDistances[populationRoute[s - 1]][populationRoute[0]];

	return routeLength;
}

//std::vector<double> choseRange(std::vector<int> fitness, int ep) {
//
//	// ???
//	// TODO: dont use this
//
//	std::vector<int>::const_iterator fitIter = fitness.cbegin();
//	int minFit = *fitIter;
//	for (; fitIter != fitness.cend(); fitIter++) {
//		if (*fitIter < minFit) {
//			minFit = *fitIter;
//		}
//	}
//
//	//cout << "Minimun distance: " << minFit << endl;
//
//	std::vector<double> trueFitness;
//	double total = 0;
//	for (fitIter = fitness.cbegin(); fitIter != fitness.cend(); fitIter++) {
//		double ratio = double(*fitIter) / double(minFit);
//		double exponent = -pow(ratio, ep);
//		total += exp(exponent);	// das ergebnis von exp() ist zwischen 0 und 1
//		trueFitness.push_back(total);
//	}
//
//	std::vector<double> range(fitness.size());
//	for (int i = 0; i < (int)fitness.size(); i++) {
//		range[i] = trueFitness[i] / total;
//	}
//
//	return range;	// range[i] ist zwischen 0 und 1 und umso höher die Distanz zum minimalen Fitness Wert war, desto höher ist range[i]
//}

std::vector<int> GeneticAlgorithm::Crossover(const std::vector<int>& father, std::vector<int> mother) 
{
	if (father.size() != mother.size()) 
	{
		std::cout << "Fuck! Length is different." << std::endl;
	}

	int s = father.size();	// Kind ist immer so groß wie der Vater, auch wenn Vater und Mutter unterschiedlich groß sind
	std::default_random_engine generator(std::random_device{}());
	std::uniform_int_distribution<int> distribution(0, s - 1);

	int p1, p2;
	p1 = distribution(generator);
	p2 = distribution(generator);

	// Generiere zwei zufällige Werte, die nicht gleich sein dürfen und nicht größer als der Vater sein dürfen
	while (p1 == p2) 
	{
		p2 = distribution(generator);
	}

	if (p1 > p2) 
	{
		int temp = p1;
		p1 = p2;
		p2 = temp;
	}

	std::vector<int> child(s);

	// Alles was wir vom Vater nehmen, nehmen wir nicht von der Mutter (daher löschen wir es)
	for (int i = p1; i <= p2; i++) 
	{
		for (int j = 0; j < (int)mother.size(); j++) 
		{
			if (mother[j] == father[i]) 
			{
				mother.erase(mother.begin() + j);
				break;
			}
		}
	}

	// Kind mit Mutter und Vater befüllen
	std::vector<int>::const_iterator mIter = mother.cbegin();
	for (int i = 0; i < s; i++) 
	{
		if (i >= p1 && i <= p2) 
		{
			child[i] = father[i];
		}
		else 
		{
			child[i] = *mIter;
			if (mIter != mother.cend()) 
			{
				mIter++;
			}
		}

	}

	//cout << "p1: " << p1 << endl;
	//cout << "p2: " << p2 << endl;

	return child;	// Kind ist geboren
}

std::vector<std::vector<int>> GeneticAlgorithm::CreateNewGeneration(const std::vector<std::vector<int>>& population, const std::vector<int>& fitness) 
{
	// TODO: Sortiere Population nach Fitness Value und wähle zufällig Vater + Mutter aus der besseren Hälfte

	std::vector<double> range;	// TODO: get rid of stupid range

	int routeNb = population.size();

	std::default_random_engine generator(std::random_device{}());

	std::vector<std::vector<int>> childGen;
	std::uniform_real_distribution<double> distribution(0, 1);
	for (int i = 0; i < routeNb; i++) 
	{
		double randNb;
		randNb = distribution(generator);	// Zufallszahl zwischen 0 und 1

		// Wähle zufällig einen Vater, der in einem gewissen Bereich liegt
		int parFather = 0;
		for (int j = 1; j < (int)range.size(); j++)
		{
			if (randNb <= range[0]) 
			{
				parFather = 0;
				break;
			}
			else if (randNb > range[j - 1] && randNb <= range[j])
			{
				parFather = j;
				break;
			}
		}

		randNb = distribution(generator);

		// Wähle zufällig eine Mutter, die in einem gewissen Bereich liegt
		int parMother = 0;
		for (int j = 1; j < (int)range.size(); j++)
		{
			if (randNb <= range[0]) 
			{
				parMother = 0;
				break;
			}
			else if (randNb > range[j - 1] && randNb <= range[j])
			{
				parMother = j;
				break;
			}
		}

		// Vater und Mutter müssen unterschiedlich sein
		while (parFather == parMother)
		{
			randNb = distribution(generator);
			for (int j = 1; j < (int)range.size(); j++)
			{
				if (randNb <= range[0]) 
				{
					parMother = 0;
					break;
				}
				else if (randNb > range[j - 1] && randNb <= range[j])
				{
					parMother = j;
					break;
				}
			}
		}

		std::vector<int> child = Crossover(population[parFather], population[parMother]);
		childGen.push_back(child);	// Das soeben geborene Kind ist Teil der neuen Generation

	}

	std::vector<int>::const_iterator fitIter = fitness.cbegin();
	int minFit = *fitIter;
	int index = 0;
	for (; fitIter != fitness.cend(); fitIter++) 
	{
		if (*fitIter < minFit) 
		{
			minFit = *fitIter;
			index = fitIter - fitness.cbegin();
		}
	}
	childGen.push_back(population[index]);	// Das beste alte Ding ist ebenfalls Teil der neuen Generation
	// Damit wird unsere Population immer um 1 größer

	return childGen;
}

std::vector<std::vector<int>> GeneticAlgorithm::Mutate(std::vector<std::vector<int>> population) 
{
	// Mutationsfunktion
	std::default_random_engine generator(std::random_device{}());
	std::uniform_real_distribution<double> dis(0, 1);
	std::uniform_int_distribution<int> disInt(0, mNumCities - 1);

	for (int i = 0; i < (int)population.size(); i++) 
	{
		double r = dis(generator);
		if (r >= mMutationRate) 
		{
			int first, second;
			first = disInt(generator);
			second = disInt(generator);

			while (first == second) 
			{
				second = disInt(generator);
			}

			int temp = population[i][first];
			population[i][first] = population[i][second];
			population[i][second] = temp;
		}
	}

	return population;
}

std::vector<int> GeneticAlgorithm::SaveBest(const std::vector<std::vector<int>>& population, const std::vector<int>& fitness) 
{
	// Bestes Ding abspeichern
	std::vector<int>::const_iterator fitIter = fitness.cbegin();
	int minFit = *fitIter;
	int index = 0;
	for (; fitIter != fitness.cend(); fitIter++) 
	{
		if (*fitIter < minFit) 
		{
			minFit = *fitIter;
			index = fitIter - fitness.cbegin();
		}
	}

	return population[index];
}

const std::vector<int>& GeneticAlgorithm::GetBest() const
{
	auto currentBestIt = mBestSolutions.begin();
	int currentBest = INT32_MAX;

	// Find the best (= smallest) Fitness Value
	for (auto it = mBestSolutions.begin(); it != mBestSolutions.end(); it++)
	{
		int fitness = EvaluateFitness(*it);
		if (fitness < currentBest)
		{
			currentBest = fitness;
			currentBestIt = it;
		}
	}

	return *currentBestIt;
}

void GeneticAlgorithm::PrintOutput(const std::vector<int>& solution) const
{
	//// Output
	//// Gesamtdistanz aller Fahrzeuge: Distanz
	//// Vehicle 1 (Gesamtdistanz): StadtA -> StadtB -> StadtC -> etc. ... -> StadtA
	//// Vehicle 2 (Gesamtdistanz): StadtA -> StadtX -> StadtY -> etc. ... -> StadtA
	//// Vehicle 3 (Gesamtdistanz): etc.
	//// etc.

	std::string output = "Gesamtdistanz aller Fahrzeuge: ";
	std::vector<std::string> vehicleStrings;
	int completeDistance = 0;
	int vehicleDistance = 0;
	for (size_t i = 0; i <= solution.size(); i++)
	{
		if (i == solution.size() || solution[i] == sBlank)
		{
			// Build Output String (add vehicle details whenever a blank is discovered or the end is reached)
			vehicleDistance += mDistances[solution[i - 1]][mDepot];
			completeDistance += vehicleDistance;
			std::string frontInfo = "Vehicle " + std::to_string(vehicleStrings.size()) + "(" + std::to_string(vehicleDistance) + "): ";
			vehicleStrings[vehicleStrings.size() - 1].insert(0, frontInfo);
			vehicleStrings[vehicleStrings.size() - 1] += " -> " + mCities[mDepot];
			vehicleDistance = 0;
		}
		else
		{
			if (vehicleDistance == 0)
			{
				// Increase travelled distance and add starting city
				vehicleDistance += mDistances[mDepot][solution[i]];
				vehicleStrings.push_back(mCities[mDepot]);
			}
			else
			{
				// Increase travelled distance
				vehicleDistance += mDistances[solution[i - 1]][solution[i]];
			}
			// Add visited city
			vehicleStrings[vehicleStrings.size() - 1] += " -> " + mCities[solution[i]];
		}
	}

	output += std::to_string(completeDistance) + "\n";

	// Print Output-String to Console
	for (const auto& vehicle : vehicleStrings)
	{
		output += vehicle + "\n";
	}
	std::cout << output << std::endl;
}