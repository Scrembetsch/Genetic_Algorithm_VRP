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
#include <algorithm>

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

// Initialize Genetic Algorithm
GeneticAlgorithm::GeneticAlgorithm()
	: mDistances(nullptr)
	, mNumCities(0)
	, mRouteSize(mNumCities + (sVehicles - 1))
	, mPopulationSize(500)
	, mIterations(100000)
	, mMutationRate(0.5)
	, mBestSolution()
{
}

GeneticAlgorithm::GeneticAlgorithm(const GeneticAlgorithm& ga)
	: mDistances(nullptr)
	, mNumCities(ga.mNumCities)
	, mRouteSize(ga.mRouteSize)
	, mPopulationSize(ga.mPopulationSize)
	, mIterations(ga.mIterations)
	, mMutationRate(ga.mMutationRate)
	, mCities(ga.mCities)
	, mBestSolution(ga.mBestSolution)

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
	delete[] mBestSolution;
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
	int** population = InitPopulation();

	int* routeLength = new int[mPopulationSize];
	for (int i = 0; i < mPopulationSize; i++)
	{
		routeLength[i] = EvaluateFitness(population[i]);
	}
	mBestSolution = SaveBest(population, routeLength);	// Save best from initial population

	for (int j = 0; j < mIterations; j++)
	{
		population = CreateNewGeneration(population, routeLength);
		population = Mutate(population);
		int* routeLength = new int[mPopulationSize];
		for (int i = 0; i < mPopulationSize; i++)
		{
			routeLength[i] = EvaluateFitness(population[i]);
		}

		int* temp = SaveBest(population, routeLength);	// Save the best of each iteration
		if (j == 0 || EvaluateFitness(temp) < EvaluateFitness(mBestSolution))
		{
			delete[] mBestSolution;
			mBestSolution = temp;
		}
		else
		{
			delete[] temp;
		}
		delete[] routeLength;
	}

	delete[] routeLength;
	for (int i = 0; i < mPopulationSize; i++)
	{
		delete[] population[i];
	}
	delete[] population;
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
		if (commentIndex == 0U)	// Check if a comment exists in this line
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

	// Save variables
	mNumCities = cityCounter;
	mRouteSize = mNumCities + (sVehicles - 1);

	// Created adj. matrix
	mDistances = new int* [mNumCities];
	for (int i = 0; i < mNumCities; i++)
	{
		mDistances[i] = new int[mNumCities];
		for (int j = 0; j < mNumCities; j++)
		{
			mDistances[i][j] = i == j ? 0 : -1;
		}
	}

	// Store values in adj. matrix
	for (const auto& road : roads)
	{
		int index1 = cityMap.at(road.City1);
		int index2 = cityMap.at(road.City2);

		mDistances[index1][index2] = road.Distance;
		mDistances[index2][index1] = road.Distance;
	}

	// Calculate missing routes using a path finder
	// Is done to have less work in crossover and mutate
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
	}
	return true;
}

void GeneticAlgorithm::PrintDistances() const
{
	// Print distances
	std::cout << "Distances:" << std::endl;
	std::cout << "  \t";
	for (int i = 0; i < mNumCities; i++)
	{
		std::cout << char('A' + i) << " \t";	// Print placeholder city names
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
	// Print Cities
	std::cout << "Cities:" << std::endl;
	for (const auto& city : mCities)
	{
		std::cout << city.Name << "\t" << city.X << ", " << city.Y << std::endl;
	}
}

bool GeneticAlgorithm::ValidateRoute(int* route, bool assertOnError) const
{
	// Is used to find invalid routes

	std::vector<int> temp(mRouteSize);
	for (int i = 0; i < mRouteSize; i++)
	{
		temp[i] = route[i];
	}
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

	if (route[mRouteSize - 1] == sBlank)
	{
		std::cout << "ERROR: Last Route is empty!" << std::endl;
		assert(!assertOnError);
		return false;
	}

	for (int i = 1; i < mRouteSize; i++)
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
	for (int i = 0; i < mRouteSize; i++)
	{
		if (route[i] != sBlank)
		{
			if (route[i] >= mNumCities)
			{
				std::cout << "ERROR: Route contains invalid Id! Id: " << std::to_string(route[i]) << std::endl;
				assert(!assertOnError);
				return false;
			}
			doubleEntries[route[i]]++;
		}
		else
		{
			numBlanks++;
		}
	}
	for (int i = 0; i < mNumCities; i++)
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

int** GeneticAlgorithm::InitPopulation()
{
	int** population = new int*[mPopulationSize];
	for (int i = 0; i < mPopulationSize; i++)
	{
		population[i] = new int[mRouteSize];
	}
	// std::vector<int>(baseStation|routeVehicle1|blank|routeVehicle2|blank|routeVehicle3|blank|routeVehicle4|blank|routeVehicle5|...)	-> https://www.researchgate.net/publication/220743156_Vehicle_Routing_Problem_Doing_It_The_Evolutionary_Way
	// Creates valid population (valid: base station set & no route empty) - number of cities per route can vary (distance between 2 cities on two sides of the country can be bigger than the distance between 5 close cities -> let Darwin do his thing)
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
		if (population[i][0] == sBlank || population[i][1] == sBlank || population[i][mRouteSize - 1] == sBlank)
		{
			// Reset loop and generate a new version for this individual
			--i;
			continue;
		}

		bool hasEmptyRoutes = false;
		// Check if there are blanks next to each other
		for (int j = 1; j < mRouteSize; j++)// Start on second index to prevent out of bounds when checking previous element
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

//Calculates the Fitness of a given Route with the overall Distance of this Route and the average difference in Distance between the Trucks.
//These Values get multiplied by specific Fitness Weights and are added together. The smaller the resulting number, the better the given route.

int GeneticAlgorithm::EvaluateFitness(int* populationRoute) const
{
	//Check for invalid Route. Set to max Fitness value
	if (populationRoute[0] == sBlank || populationRoute[1] == sBlank)
	{
		return INT32_MAX;
	}

	//Weights for the Fitness Calculation
	float weight1 = 0.3;	// Weight of overall distance
	float weight2 = 0.7;	// Weight of average distance

	std::vector<int> routeDistances;

	int s = mRouteSize;
	int routeLength = 0;
	int routePartLength = 0;
	int currentDistance = 0;
	int averageTruckDistance = 0;

	int distanceDifference = 0;
	int averageDistanceDifference = 0;
	int addCorrected = 0;

	//Iterate all Points in Route
	for (int i = 1; i < (s - 1); i++)
	{
		//When Current and next Point not Blank. Calculate the Distance between them
		if (populationRoute[i] != sBlank)
		{
			if (populationRoute[i - 1] != sBlank)
			{
				currentDistance = mDistances[populationRoute[i]][populationRoute[i - 1]];
				//Add currently calculated Distance to the Distance of all Routs together
				routeLength += currentDistance;
				//Add currently calculated Distance to the Distance for this Route
				routePartLength += currentDistance;
			}
		}
		else
		{
			//Check if Route is Valid
			if (populationRoute[i + 1] == sBlank)
			{
				return INT32_MAX;
			}

			//Check if there are no two blanks after one another
			if (populationRoute[i - 1] != sBlank && populationRoute[i + 1] != sBlank)
			{
				//Calculate the Distance back to the Start
				currentDistance = mDistances[populationRoute[i - 1]][populationRoute[0]];
				routeLength += currentDistance;
				routePartLength += currentDistance;

				routeDistances.push_back(routePartLength);

				//Prepare for next Truck on this Route
				currentDistance = mDistances[populationRoute[0]][populationRoute[i + 1]];
				routeLength += currentDistance;
				routePartLength = currentDistance;
			}
		}
	}
	routeDistances.push_back(routePartLength);

	//Calculate average difference in Distance between the Trucks
	averageTruckDistance = routeLength / sVehicles;
	for (int distance : routeDistances)
	{
		distanceDifference += std::abs(averageTruckDistance - distance);
	}
	averageDistanceDifference = distanceDifference / sVehicles;

	//Correct average Distance to be the same size as the route Length
	addCorrected = averageDistanceDifference * 10;

	return (weight1 * routeLength) + (weight2 * addCorrected);
}

// Creates an array in which the number at a certain index indicates
// how many numbers in the converted array (which were left of the number
// of the index position) were greater than the number itself, formula source: https://user.ceng.metu.edu.tr/~ucoluk/research/publications/tspnew.pdf
int* GeneticAlgorithm::createInversionSequence(int* individual)
{
	int* inversionSequence = new int[mRouteSize];

	for (int i = 0; i < mRouteSize; i++)
	{
		int counter = 0;

		for (int j = 0; j < mRouteSize; j++)
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

// Inverse function of "createInversionSequence()"
int* GeneticAlgorithm::recreateNumbers(int* inversionSequence, int size)
{
	int* positions = new int[size];
	int* square = new int[size];

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

int* GeneticAlgorithm::Crossover(int* father, int* mother)
{
	int s = mRouteSize;

	int* child = nullptr;

	int fatherOffset = 0;
	int motherOffset = 0;

	//to replace the blanks with mNumCities, mNumCities+1,...
	//because the algorithm works only in a number sequence where each number is unique
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
		if (mother[i] == father[0])
		{
			mother[i] = mother[0];
			mother[0] = father[0];
		}
	}

	//create inversion sequence of father
	int* inversionSequenceP1;
	inversionSequenceP1 = createInversionSequence(father);

	//create inversion sequence of mother
	int* inversionSequenceP2;
	inversionSequenceP2 = createInversionSequence(mother);

	//crossover point somewhere between 20% and 80%
	int min = s * 0.2;
	int max = s * 0.8;

	std::default_random_engine generator(std::random_device{}());
	std::uniform_int_distribution<int> distribution(min, max);

	int crossoverPoint = distribution(generator) + 1;
	int* inversionSequenceChild = new int[s];

	//create child based on crossover point
	std::copy(inversionSequenceP1, inversionSequenceP1 + crossoverPoint, inversionSequenceChild);
	std::copy(inversionSequenceP2 + crossoverPoint, inversionSequenceP2 + s, inversionSequenceChild + crossoverPoint);

	//create usable child sequence
	child = recreateNumbers(inversionSequenceChild, s);

	delete[] inversionSequenceP1;
	delete[] inversionSequenceP2;
	delete[] inversionSequenceChild;

	int reshuffleBlanks = 0;

	int childSize = mRouteSize;

	for (int i = 0; i < childSize; i++)
	{
		child[i]--;
		if (child[i] >= mNumCities)
		{
			child[i] = sBlank; //reset route delimiters to sBlank
			if (child[i - 1] == sBlank || i < 2) //avoid empty routes
			{
				for (int j = i; j < childSize - 1; j++)
				{
					child[j] = child[j + 1];
				}
				childSize--;
				i--;
				reshuffleBlanks++;
			}
		}
	}

	while (child[childSize - 1] == sBlank)
	{
		childSize--;
		reshuffleBlanks++;
	}

	int i = 2;
	while (reshuffleBlanks > 0)
	{
		if (i != (childSize - 1) && child[i] != sBlank && child[i - 1] != sBlank)
		{

			for (int j = childSize; j > i; j--)
			{
				child[j] = child[j - 1];
			}
			childSize++;
			child[i] = sBlank;
			reshuffleBlanks--;
		}
		i++;
	}

	//ValidateRoute(child, true);

	return child;
}

//sort the population based on the associated fitness values via quick sort
void GeneticAlgorithm::sort(int** population, int* fitness, int l, int r)
{
	int i = l;
	int j = r;
	int bufferFitness;
	int* bufferPopulation;
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

int** GeneticAlgorithm::CreateNewGeneration(int** population, int* fitness)
{
	//set the random generator to pick only from the better half of the population
	std::default_random_engine generator(std::random_device{}());
	std::uniform_int_distribution<int> distribution(0, mPopulationSize/2);

	//sort the population
	sort(population, fitness, 0, mPopulationSize - 1);

	int** new_population = new int* [mPopulationSize];
	for (int i = 0; i < mPopulationSize; i++)
	{
		new_population[i] = new int[mRouteSize];
	}

	for (int i = 0; i < mPopulationSize / 2; i++) //take fathers and mothers from the better half of the population
	{
		int randomNum1 = distribution(generator);
		int randomNum2 = distribution(generator);
		if (randomNum1 == randomNum2)
		{
			i--;
			continue;
		}

		std::copy(population[i], population[i] + mRouteSize, new_population[i]);

		//create father sequence
		int* father = new int[mRouteSize];
		std::copy(population[randomNum1], population[randomNum1] + mRouteSize, father);

		//create mother sequence
		int* mother = new int[mRouteSize];
		std::copy(population[randomNum2], population[randomNum2] + mRouteSize, mother);

		//get child sequence
		int* child = Crossover(father, mother);

		delete[] new_population[i + mPopulationSize / 2];
		new_population[i + mPopulationSize / 2] = child;

		delete[] father;
		delete[] mother;
	}
	for (int i = 0; i < mPopulationSize; i++)
	{
		delete[] population[i];
	}
	delete[] population;

	return new_population;
}

bool GeneticAlgorithm::CheckSwap(int* route, int first, int second) const
{
	if (route[second] == sBlank)
	{
		if (first == 0 || first == 1 || first == mRouteSize - 1)
		{
			return false;
		}
		if (route[first - 1] == sBlank || route[first + 1] == sBlank)
		{
			return false;
		}
	}
	return true;
}

int** GeneticAlgorithm::Mutate(int** population)
{
	// Mutation-Function
	std::default_random_engine generator(std::random_device{}());
	std::uniform_real_distribution<double> dis(0, 1);
	// Maximum Array Size = numCities + 4 blanks (to separate the 5 vehicles)
	std::uniform_int_distribution<int> disInt(0, mNumCities + sVehicles - 2);
	int size = mRouteSize;
	for (int i = 0; i < size; i++)
	{
		double r = dis(generator);
		if (r <= mMutationRate)
		{
			int first, second;
			first = disInt(generator);
			second = disInt(generator);

			// Don't swap the depot, the first or the last city with a blank
			while (!CheckSwap(population[i], first, second))
			{
				second = disInt(generator);
			}

			// Don't swap the depot, the first or the last city  with a blank
			while (!CheckSwap(population[i], second, first))
			{
				first = disInt(generator);
			}
			std::swap(population[i][first], population[i][second]);
		}
	}

	return population;
}

int* GeneticAlgorithm::SaveBest(int** population, int* fitness)
{
	// Save the best solution of the current population
	int* fitIter = fitness;
	int minFit = *fitIter;
	int index = 0;
	for (; fitIter != fitness + mPopulationSize; fitIter++)
	{
		if (*fitIter < minFit)
		{
			minFit = *fitIter;
			index = fitIter - fitness;
		}
	}
	int* best = new int[mRouteSize];
	std::copy(population[index], population[index] + mRouteSize, best);
	return best;
}

int* GeneticAlgorithm::GetBest() const
{
	return mBestSolution;
}

void GeneticAlgorithm::PrintOutput(int* solution) const
{
	// Output
	// Total distance of all vehicles: <Total distance of all 5 vehicles>|<Average distance of all 5 vehicles>
	// Vehicle 1 <Total distance of vehicle 1>|<Distance difference to average distance>: Depot -> CityA -> CityB -> etc. ... -> Depot
	// Vehicle 2 <Total distance of vehicle 2>|<Distance difference to average distance>: Depot-> StadtX -> StadtY -> etc. ... -> Depot
	// Vehicle 3 <Total distance of vehicle 3>|<Distance difference to average distance>: etc.
	// etc.

	ValidateRoute(solution, false);

	std::string output = "Gesamtdistanz aller Fahrzeuge: ";
	std::vector<std::string> vehicleStrings(sVehicles);
	std::vector<int> vehicleDistances(sVehicles);
	int completeDistance = 0;
	int vehicleDistance = 0;
	int vehicleCounter = 0;

	for (int i = 1; i <= mRouteSize; i++)
	{
		if (i == mRouteSize || solution[i] == sBlank)
		{
			if (vehicleDistance == 0)
			{
				if (vehicleCounter == sVehicles)
				{
					break;
				}
				vehicleStrings[vehicleCounter] = "Vehicle " + std::to_string(vehicleCounter + 1) + "(" + std::to_string(0) + ") ";
				vehicleDistances[vehicleCounter] = 0;
			}
			else
			{
				// Build Output String (add vehicle details whenever a blank is discovered or the end is reached)
				vehicleDistance += mDistances[solution[i - 1]][solution[0]];
				completeDistance += vehicleDistance;
				std::string frontInfo = "Vehicle " + std::to_string(vehicleCounter + 1) + "(" + std::to_string(vehicleDistance) + "): ";
				vehicleDistances[vehicleCounter] = vehicleDistance;
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
	int averageDistance = completeDistance / sVehicles;
	output += std::to_string(completeDistance) + "|" + std::to_string(averageDistance) + "\n";

	// Print Output-String to Console
	for (int i = 0; i < sVehicles; i++)
	{
		vehicleStrings[i].insert(vehicleStrings[i].find(')'), "|" + std::to_string(std::abs(averageDistance - vehicleDistances[i])));
	}
	for (const auto& vehicle : vehicleStrings)
	{
		output += vehicle + "\n";
	}
	std::cout << output << std::endl;
}