#include <assert.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <cmath>
#include <map>
#include <stdlib.h>

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

bool City::Parse(const std::string& cityString)
{
	// Find important part
	size_t openingBracket = cityString.find('(');
	size_t closingBracket = cityString.find(')', openingBracket);

	// Find first letter (non-whitespace) & end of city
	size_t begin = Util::FindNextNonWhitespace(cityString, openingBracket);
	size_t comma = cityString.find(',', begin);
	Name = cityString.substr(begin, comma - begin);

	// Find next first letter & end of city
	begin = Util::FindNextNonWhitespace(cityString, comma);
	comma = cityString.find(',', begin);
	X = std::stof(cityString.substr(begin, comma - begin));

	// Find first digit
	begin = Util::FindNextNonWhitespace(cityString, comma);
	Y = std::stof(cityString.substr(begin, closingBracket - begin));

	return true;
}

const int GeneticAlgorithm::sBlank = -42;
const int GeneticAlgorithm::sVehicles = 5;

GeneticAlgorithm::GeneticAlgorithm()
	: mDistances(nullptr)
	, mNumCities(0)
	, mPopulationSize(300)
	, mIterations(10)
	, mMutationRate(0.1)
	, mBestSolutions(mIterations, std::vector<int>(mNumCities))
{
}

GeneticAlgorithm::GeneticAlgorithm(const GeneticAlgorithm& ga)
	: mDistances(nullptr)
	, mNumCities(ga.mNumCities)
	, mPopulationSize(ga.mPopulationSize)
	, mIterations(ga.mIterations)
	, mMutationRate(ga.mMutationRate)
	, mCities(ga.mCities)
	, mBestSolutions(mIterations, std::vector<int>(mNumCities))

{
	mDistances = new int* [mNumCities];
	for (int i = 0; i < mNumCities; i++)
	{
		mDistances[i] = new int[mNumCities];
		for (int j = 0; j < mNumCities; j++)
		{
			mDistances[i][j] = ga.mDistances[i][j];
		}
	}
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
			size_t end = std::string::npos;
			size_t offset = 0U;
			while ((end = line.find(").", offset)) != std::string::npos	// Find end of road, offset is used if multiple roads are in one line
				   && end < commentIndex								// Check if found end is not after comment
				   )
			{
				mCities.push_back(City());
				mCities[mCities.size() - 1].Parse(line.substr(offset, end));	// Parse city string
				offset = end + 1;
				cityMap.insert(std::make_pair(mCities[mCities.size() - 1].Name, cityCounter++));
			}
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

	//PrintCities();
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

		//PrintDistances();
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

void GeneticAlgorithm::PrintDistances() const
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

void GeneticAlgorithm::PrintCities() const
{
	std::cout << "Cities:" << std::endl;
	for (const auto& city : mCities)
	{
		std::cout << city.Name << "\t" << city.X << ", " << city.Y << std::endl;
	}
}

bool GeneticAlgorithm::ValidateRoute(const std::vector<int>& route, bool assertOnError) const
{
	if (route[0] == sBlank)
	{
		std::cout << "ERROR: Depot is Blank!" << std::endl;
		assert(!assertOnError);
		return false;
	}

	if (route[1] == sBlank)
	{
		std::cout << "ERROR: First Route is empty!" << std::endl;
		assert(!assertOnError);
		return false;
	}

	if (route[route.size() - 1] == sBlank)
	{
		std::cout << "ERROR: Last Route is empty!" << std::endl;
		assert(!assertOnError);
		return false;
	}

	for (size_t i = 1; i < route.size(); i++)
	{
		if (route[i - 1] == route[i])
		{
			std::cout << "ERROR: Same id found two consecutive times! Id: " << std::to_string(route[i]) << std::endl;
			assert(!assertOnError);
			return false;
		}
	}

	std::vector<int> doubleEntries(mNumCities, 0);
	int numBlanks = 0;
	for (size_t i = 0; i < route.size(); i++)
	{
		if (route[i] != sBlank)
		{
			doubleEntries[route[i]]++;
		}
		else
		{
			numBlanks++;
		}
	}
	for (size_t i = 0; i < mNumCities; i++)
	{
		if (doubleEntries[i] > 1)
		{
			std::cout << "ERROR: Same id found two times! Id: " << std::to_string(i) << std::endl;
			assert(!assertOnError);
			return false;
		}
		if (doubleEntries[i] == 0)
		{
			std::cout << "ERROR: Id not found! Id: " << std::to_string(i) << ", City: "<< mCities[i].Name << std::endl;
			assert(!assertOnError);
			return false;
		}
	}

	if (numBlanks != sVehicles - 1)
	{
		std::cout << "ERROR: Number of blanks does not match! Expected: " << std::to_string(sVehicles - 1) << ", Actual: " << std::to_string(numBlanks) << std::endl;

		assert(!assertOnError);
		return false;
	}
	return true;
}

std::vector<std::vector<int>> GeneticAlgorithm::InitPopulation()
{
	std::vector<std::vector<int>> population(mPopulationSize, std::vector<int>(mNumCities + (sVehicles - 1)));// Cities + blanks
	// std::vector<int>(routeVehicle1|blank|routeVehicle2|blank|routeVehicle3|blank|routeVehicle4|blank|routeVehicle5)	-> siehe Discord Paper
	// Wichtig: gültige initiale Population erzeugen, sodass alle 5 Fahrzeuge ungefähr die gleiche Anzahl an Städten befahren   <--- Anzahl der Städte pro Route ist random weil sie nicht wirklich Einfluss auf die tatsächliche Länge der Strecke hat (Irena)

	std::default_random_engine generator(std::random_device{}());

	int city;
	for (int i = 0; i < mPopulationSize; i++)
	{
		// Add all cities to array
		std::vector<int> place;
		for (int k = 0; k < mNumCities; k++)
		{
			place.push_back(k);
		}
		// Add blanks to array
		for (int k = 0; k < sVehicles - 1; k++)
		{
			place.push_back(sBlank);
		}

		// Swap cities and blanks randomly
		int s = mNumCities + sVehicles - 1;
		for (int j = 0; j < mNumCities + sVehicles - 1; j++)
		{
			std::uniform_int_distribution<int> distribution(0, s - 1);

			city = distribution(generator);
			population[i][j] = place[city];

			//int temp = place[s - 1];
			place[city] = place[s - 1];
			place.pop_back();
			s = s - 1;
		}

		// Check if no base station is set and if routes of first and last vehicle is empty
		if (population[i].front() == sBlank || population[i][1] == sBlank || population[i].back() == sBlank)
		{
			// Reset loop and generate a new version for this individual
			--i;
			continue;
		}

		bool hasEmptyRoutes = false;
		// Check if there are blanks next to each other
		for (size_t j = 1; j < population[i].size(); j++)//start on second index to prevent out of bounds when checking previus element
		{
			// If both previous and current element are blanks
			if (population[i][j] == sBlank && population[i][j - 1] == sBlank)
			{
				// Mark as invalid and kill this loop
				hasEmptyRoutes = true;
				break;
			}
		}

		if (hasEmptyRoutes)
		{
			// Reset loop and generate a new version for this individual
			--i;
			continue;
		}
	}

	return population;
}

int GeneticAlgorithm::EvaluateFitness(const std::vector<int>& populationRoute) const
{
	if (populationRoute[0] == sBlank || populationRoute[1] == sBlank)
	{
		return INT32_MAX;
	}
	float weight1 = 0.5;
	float weight2 = 0.5;

	std::vector<int> routeDistances;

	int s = populationRoute.size();
	int routeLength = 0;
	int routePartLength = 0;
	int currentDistance = 0;
	int averageTruckDistance = 0;

	int distanceDifference = 0;
	int averageDistanceDifference = 0;
	int addCorrected = 0;

	for (int i = 1; i < (s - 1); i++)
	{
		if (populationRoute[i] != sBlank)
		{
			currentDistance = mDistances[populationRoute[i]][populationRoute[i - 1]];
			routeLength += currentDistance;
			routePartLength += currentDistance;
		}
		else
		{
			if (populationRoute[i + 1] == sBlank)
			{
				return INT32_MAX;
			}

			if (populationRoute[i - 1] != sBlank && populationRoute[i + 1] != sBlank)
			{
				currentDistance = mDistances[populationRoute[i - 1]][populationRoute[0]];
				routeLength += currentDistance;
				routePartLength += currentDistance;

				routeDistances.push_back(routePartLength);

				currentDistance = mDistances[populationRoute[0]][populationRoute[i + 1]];
				routeLength += currentDistance;
				routePartLength = currentDistance;
			}
		}
	}
	routeDistances.push_back(routePartLength);
	averageTruckDistance = routeLength / sVehicles;

	for (int distance : routeDistances)
	{
		distanceDifference += std::abs(averageTruckDistance - distance);
	}

	averageDistanceDifference = distanceDifference / sVehicles;
	addCorrected = averageDistanceDifference * 10;

	return (weight1 * routeLength) + (weight2 * addCorrected);
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

int* GeneticAlgorithm::createInversionSequence(std::vector<int> individual)
{
	int* inversionSequence = new int[individual.size()];

	for (size_t i = 0; i < individual.size(); i++)
	{
		int counter = 0;

		for (size_t j = 0; j < individual.size(); j++)
		{
			if (individual[j] == int(i + 1))
				break;

			if (individual[j] > int(i + 1))
				counter++;
		}
		inversionSequence[i] = counter;
	}

	return inversionSequence;
}

std::vector<int> GeneticAlgorithm::recreateNumbers(int* inversionSequence, int size)
{
	int* positions = new int[size];
	std::vector<int> square(size);

	for (int i = (size - 1); i >= 0; i--)
	{
		int additionValue = 1;
		int currentValue = inversionSequence[i];
		positions[i] = currentValue;

		while (i + additionValue <= (size - 1))
		{
			if (positions[i + additionValue] >= positions[i])
			{
				positions[i + additionValue]++;
			}

			additionValue++;
		}
	}

	for (int i = 0; i < size; i++)
	{
		int insertPosition = positions[i];
		square[insertPosition] = (i + 1);
	}

	delete[] positions;

	return square;
}

std::vector<int> GeneticAlgorithm::Crossover(std::vector<int> father, std::vector<int> mother)
{
	//if (father.size() != mother.size())
	//{
	//	std::cout << "Fuck! Length is different." << std::endl;
	//}

	//int s = father.size();	// Kind ist immer so groß wie der Vater, auch wenn Vater und Mutter unterschiedlich groß sind
	//std::default_random_engine generator(std::random_device{}());
	//std::uniform_int_distribution<int> distribution(0, s - 1);

	//int p1, p2;
	//p1 = distribution(generator);
	//p2 = distribution(generator);

	//// Generiere zwei zufällige Werte, die nicht gleich sein dürfen und nicht größer als der Vater sein dürfen
	//while (p1 == p2)
	//{
	//	p2 = distribution(generator);
	//}

	//if (p1 > p2)
	//{
	//	int temp = p1;
	//	p1 = p2;
	//	p2 = temp;
	//}

	//std::vector<int> child(s);

	//// Alles was wir vom Vater nehmen, nehmen wir nicht von der Mutter (daher löschen wir es)
	//for (int i = p1; i <= p2; i++)
	//{
	//	for (int j = 0; j < (int)mother.size(); j++)
	//	{
	//		if (mother[j] == father[i])
	//		{
	//			mother.erase(mother.begin() + j);
	//			break;
	//		}
	//	}
	//}

	//// Kind mit Mutter und Vater befüllen
	//std::vector<int>::const_iterator mIter = mother.cbegin();
	//for (int i = 0; i < s; i++)
	//{
	//	if (i >= p1 && i <= p2)
	//	{
	//		child[i] = father[i];
	//	}
	//	else
	//	{
	//		child[i] = *mIter;
	//		if (mIter != mother.cend())
	//		{
	//			mIter++;
	//		}
	//	}

	//}

	////cout << "p1: " << p1 << endl;
	////cout << "p2: " << p2 << endl;

	//return child;	// Kind ist geboren



	int s = father.size();

	std::vector<int> child(s);

	//to replace the blanks with mNumCities, mNumCities+1,...
	//because the algorithm works only in a number sequence where each number is unique
	int fatherOffset = 0;
	int motherOffset = 0;

	for (int i=0; i < s; i++)
	{
		if (father[i] == sBlank)
		{
			father[i] = mNumCities + fatherOffset;
			fatherOffset++;
		}
		if (mother[i] == sBlank)
		{
			mother[i] = mNumCities + motherOffset;
			motherOffset++;
		}
	}

	int* inversionSequenceP1;
	inversionSequenceP1 = createInversionSequence(father);

	int* inversionSequenceP2;
	inversionSequenceP2 = createInversionSequence(mother);

	int min = s * 0.2;
	int max = s * 0.8;

	std::default_random_engine generator(std::random_device{}());
	std::uniform_int_distribution<int> distribution(min, max);

	int crossoverPoint = distribution(generator) + 1;
	int* inversionSequenceChild = new int[s];

	std::copy(inversionSequenceP1, inversionSequenceP1 + crossoverPoint, inversionSequenceChild);
	std::copy(inversionSequenceP2 + crossoverPoint, inversionSequenceP2 + s, inversionSequenceChild + crossoverPoint);

	child = recreateNumbers(inversionSequenceChild, s);

	delete[] inversionSequenceP1;
	delete[] inversionSequenceP2;
	delete[] inversionSequenceChild;

	for (int i = 0; i < s; i++)
	{
		if (child[i] >= mNumCities)
		{
			child[i] = sBlank;
		}
	}

	return child;
}

void GeneticAlgorithm::sort(std::vector<std::vector<int>>& population, std::vector<int>& fitness, int l, int r)
{
	int i = l;
	int j = r;
	int bufferFitness;
	std::vector<int> bufferPopulation;
	int pivot = fitness[(l + r) / 2];
	while (i <= j) {
		while (fitness[i] < pivot)
		{
			i++;
		}
		while (fitness[j] > pivot)
		{
			j--;
		}

		if (i <= j)
		{
			bufferFitness = fitness[i];
			fitness[i] = fitness[j];
			fitness[j] = bufferFitness;
			bufferPopulation = population[i];
			population[i] = population[j];
			population[j] = bufferPopulation;
			i++;
			j--;
		}
	}
	if (l < j)
		sort(population, fitness, l, j);
	if (i < r)
		sort(population, fitness, i, r);
}

std::vector<std::vector<int>> GeneticAlgorithm::CreateNewGeneration(std::vector<std::vector<int>> population, std::vector<int> fitness)
{
	//// TODO: Sortiere Population nach Fitness Value und wähle zufällig Vater + Mutter aus der besseren Hälfte

	//std::vector<double> range;	// TODO: get rid of stupid range

	//int routeNb = population.size();

	//std::default_random_engine generator(std::random_device{}());

	//std::vector<std::vector<int>> childGen;
	//std::uniform_real_distribution<double> distribution(0, 1);
	//for (int i = 0; i < routeNb; i++)
	//{
	//	double randNb;
	//	randNb = distribution(generator);	// Zufallszahl zwischen 0 und 1

	//	// Wähle zufällig einen Vater, der in einem gewissen Bereich liegt
	//	int parFather = 0;
	//	for (int j = 1; j < (int)range.size(); j++)
	//	{
	//		if (randNb <= range[0])
	//		{
	//			parFather = 0;
	//			break;
	//		}
	//		else if (randNb > range[j - 1] && randNb <= range[j])
	//		{
	//			parFather = j;
	//			break;
	//		}
	//	}

	//	randNb = distribution(generator);

	//	// Wähle zufällig eine Mutter, die in einem gewissen Bereich liegt
	//	int parMother = 0;
	//	for (int j = 1; j < (int)range.size(); j++)
	//	{
	//		if (randNb <= range[0])
	//		{
	//			parMother = 0;
	//			break;
	//		}
	//		else if (randNb > range[j - 1] && randNb <= range[j])
	//		{
	//			parMother = j;
	//			break;
	//		}
	//	}

	//	// Vater und Mutter müssen unterschiedlich sein
	//	while (parFather == parMother)
	//	{
	//		randNb = distribution(generator);
	//		for (int j = 1; j < (int)range.size(); j++)
	//		{
	//			if (randNb <= range[0])
	//			{
	//				parMother = 0;
	//				break;
	//			}
	//			else if (randNb > range[j - 1] && randNb <= range[j])
	//			{
	//				parMother = j;
	//				break;
	//			}
	//		}
	//	}

	std::default_random_engine generator(std::random_device{}());
	std::uniform_real_distribution<double> distribution(0, population.size()/2);


	std::vector<std::vector<int>> childGen;
	sort(population, fitness, 0, fitness.size() - 1);

	std::vector<std::vector<int>> new_population;
	for (unsigned int i = 0; i < population.size() / 2; i++)
	{

		int randomNum1 = distribution(generator);
		int randomNum2 = distribution(generator);
		if (randomNum1 == randomNum2)
		{
			i--;
			continue;
		}
		new_population.push_back(population[i]);

		std::vector<int> father = population[randomNum1];
		std::vector<int> mother = population[randomNum2];

		std::vector<int> child = Crossover(father, mother);

		new_population.push_back(child);
	}
	population = new_population;
	new_population.clear();

	return population;
}

std::vector<std::vector<int>> GeneticAlgorithm::Mutate(std::vector<std::vector<int>> population)
{
	// Mutation-Function
	std::default_random_engine generator(std::random_device{}());
	std::uniform_real_distribution<double> dis(0, 1);
	// Maximum Array Size = numCities + 4 blanks (to seperate the 5 vehicles)
	std::uniform_int_distribution<int> disInt(0, mNumCities + sVehicles - 2);
	int size = int(population.size());
	for (int i = 0; i < size; i++)
	{
		double r = dis(generator);
		if (r <= mMutationRate)
		{
			int first, second;
			first = disInt(generator);
			second = disInt(generator);

			// Don't swap the depot with a blank
			while ((first == 0 || first == (size - 1)) && population[i][second] == sBlank)
			{
				second = disInt(generator);
			}

			// Don't swap the depot with a blank
			while ((second == 0 || second == (size - 1)) && population[i][first] == sBlank)
			{
				first = disInt(generator);
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

	ValidateRoute(solution, false);

	std::string output = "Gesamtdistanz aller Fahrzeuge: ";
	std::vector<std::string> vehicleStrings(sVehicles);
	int completeDistance = 0;
	int vehicleDistance = 0;
	int vehicleCounter = 0;

	for (size_t i = 1; i <= solution.size(); i++)
	{
		if (i == solution.size() || solution[i] == sBlank)
		{
			if (vehicleDistance == 0)
			{
				if (vehicleCounter == sVehicles)
				{
					break;
				}
				vehicleStrings[vehicleCounter] = "Vehicle " + std::to_string(vehicleCounter + 1) + "(" + std::to_string(0) + ") ";
			}
			else
			{
				// Build Output String (add vehicle details whenever a blank is discovered or the end is reached)
				vehicleDistance += mDistances[solution[i - 1]][solution[0]];
				completeDistance += vehicleDistance;
				std::string frontInfo = "Vehicle " + std::to_string(vehicleCounter + 1) + "(" + std::to_string(vehicleDistance) + "): ";
				vehicleStrings[vehicleCounter].insert(0, frontInfo);
				vehicleStrings[vehicleCounter] += " -> " + mCities[solution[0]].Name;
				vehicleDistance = 0;
			}
			vehicleCounter++;
		}
		else
		{
			if (vehicleDistance == 0)
			{
				// Increase traveled distance and add starting city
				vehicleDistance += mDistances[solution[0]][solution[i]];
				vehicleStrings[vehicleCounter] = (mCities[solution[0]].Name);
			}
			else
			{
				// Increase traveled distance
				vehicleDistance += mDistances[solution[i - 1]][solution[i]];
			}
			vehicleStrings[vehicleCounter] += " -> " + mCities[solution[i]].Name;
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