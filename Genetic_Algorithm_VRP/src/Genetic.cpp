#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <cmath>
#include <map>

#include "Genetic.h"
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

bool GeneticAlgorithm::ReadFile(std::string path)
{
	std::vector<Road> roads;
	std::map<std::string, int> cities;
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
			cities.insert(std::pair<std::string, int>(line.substr(begin, end - begin), cityCounter++));
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
		int index1 = cities.at(road.City1);
		int index2 = cities.at(road.City2);

		mDistances[index1][index2] = road.Distance;
		mDistances[index2][index1] = road.Distance;
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
	//std::cout << "Distances:" << std::endl;
	//for (int i = 0; i < mNumCities; i++)
	//{
	//	for (int j = 0; j < mNumCities; j++)
	//	{
	//		std::cout << std::to_string(mDistances[i][j]) << " \t";
	//	}
	//	std::cout << std::endl;
	//}
	return true;
}

bool GeneticAlgorithm::CalculateMissingRoutes()
{
	// TODO: Needs to be done if normal attempt provides no results
	return true;
}

std::vector<std::vector<int>> inTxt(std::string path, int cityNb) {
	// TODO: parse our file
	// Knotenpunkt ist die Stadt mit den meisten Straßenverbindungen, alternativ die Stadt die am zentralsten liegt
	// Zwischenspeichern wie das danzig24 file (siehe Discord), wobei keine Straßenverbindungen mit -1 gekennzeichnet sind
	std::ifstream infile(path);

	std::string line;
	std::vector<std::vector<int>> distance(cityNb, std::vector<int>(cityNb));

	int lineCount = 0;
	int tempDis = 0;


	while (getline(infile, line)) {

		int nb = 0;

		std::istringstream iss(line);
		while (iss >> tempDis && nb < cityNb) {

			distance[lineCount][nb] = tempDis;
			nb = nb + 1;

		}

		lineCount = lineCount + 1;

	}

	return distance;
}

std::vector<std::vector<int>> initPath(int route, int nb) {

	std::vector<std::vector<int>> population(route, std::vector<int>(nb));
	// std::vector<int>(routeVehicle1|blank|routeVehicle2|blank|routeVehicle3|blank|routeVehicle4|blank|routeVehicle5)	-> siehe Discord Paper
	// Wichtig: gültige initiale Population erzeugen, sodass alle 5 Fahrzeuge ungefähr die gleiche Anzahl an Städten befahren

	std::default_random_engine generator(std::random_device{}());

	int city;
	for (int i = 0; i < route; i++) {

		std::vector<int> place;
		for (int k = 0; k < nb; k++) {
			place.push_back(k);
		}

		int s = nb;
		for (int j = 0; j < nb; j++) {


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

int fitness(const std::vector<int>& sroute, const std::vector<std::vector<int>>& length) {
	// Mittelwert der zurückgelegten Distanz der Trucks berechnen = Summe der einzelnen Truck-Distanzen/5
	// Fitness ist Gewichtung1 (z.B. 0.5) * Summe der einzelnen Truck-Distanzen)/5 + Gewichtung2 (z.B. 0.5) * Gesamtstrecke aller Trucks
	// Fitness Wert soll möglichst klein sein
	// TODO: Wenn -1 ausgelesen wird -> durch INT_MAX ersetzen
	int s = sroute.size();

	int routeLength = 0;
	for (int i = 1; i < s; i++) {
		routeLength += length[sroute[i - 1]][sroute[i]];
	}
	routeLength += length[sroute[s - 1]][sroute[0]];

	return routeLength;
}

std::vector<double> choseRange(std::vector<int> fitness, int ep) {

	// ???
	// TODO: dont use this

	std::vector<int>::const_iterator fitIter = fitness.cbegin();
	int minFit = *fitIter;
	for (; fitIter != fitness.cend(); fitIter++) {
		if (*fitIter < minFit) {
			minFit = *fitIter;
		}
	}

	//cout << "Minimun distance: " << minFit << endl;

	std::vector<double> trueFitness;
	double total = 0;
	for (fitIter = fitness.cbegin(); fitIter != fitness.cend(); fitIter++) {
		double ratio = double(*fitIter) / double(minFit);
		double exponent = -pow(ratio, ep);
		total += exp(exponent);	// das ergebnis von exp() ist zwischen 0 und 1
		trueFitness.push_back(total);
	}

	std::vector<double> range(fitness.size());
	for (int i = 0; i < (int)fitness.size(); i++) {
		range[i] = trueFitness[i] / total;
	}

	return range;	// range[i] ist zwischen 0 und 1 und umso höher die Distanz zum minimalen Fitness Wert war, desto höher ist range[i]
}

std::vector<int> inheritance(const std::vector<int>& father, std::vector<int> mother) {

	if (father.size() != mother.size()) {
		std::cout << "Fuck! Length is different." << std::endl;
	}

	int s = father.size();	// Kind ist immer so groß wie der Vater, auch wenn Vater und Mutter unterschiedlich groß sind
	std::default_random_engine generator(std::random_device{}());
	std::uniform_int_distribution<int> distribution(0, s - 1);

	int p1, p2;
	p1 = distribution(generator);
	p2 = distribution(generator);

	// Generiere zwei zufällige Werte, die nicht gleich sein dürfen und nicht größer als der Vater sein dürfen
	while (p1 == p2) {
		p2 = distribution(generator);
	}

	if (p1 > p2) {
		int temp = p1;
		p1 = p2;
		p2 = temp;
	}

	std::vector<int> child(s);

	// Alles was wir vom Vater nehmen, nehmen wir nicht von der Mutter (daher löschen wir es)
	for (int i = p1; i <= p2; i++) {
		for (int j = 0; j < (int)mother.size(); j++) {
			if (mother[j] == father[i]) {
				mother.erase(mother.begin() + j);
				break;
			}
		}
	}

	// Kind mit Mutter und Vater befüllen
	std::vector<int>::const_iterator mIter = mother.cbegin();
	for (int i = 0; i < s; i++) {
		if (i >= p1 && i <= p2) {
			child[i] = father[i];
		}
		else {
			child[i] = *mIter;
			if (mIter != mother.cend()) {
				mIter++;
			}
		}

	}

	//cout << "p1: " << p1 << endl;
	//cout << "p2: " << p2 << endl;

	return child;	// Kind ist geboren
}

std::vector<std::vector<int>> mutate(const std::vector<std::vector<int>>& population, const std::vector<double>& range, const std::vector<int>& fitness) {

	// TODO: Sortiere Population nach Fitness Value und wähle zufällig Vater + Mutter aus der besseren Hälfte

	int routeNb = population.size();

	std::default_random_engine generator(std::random_device{}());

	std::vector<std::vector<int>> childGen;
	std::uniform_real_distribution<double> distribution(0, 1);
	for (int i = 0; i < routeNb; i++) {
		double randNb;
		randNb = distribution(generator);	// Zufallszahl zwischen 0 und 1

		// Wähle zufällig einen Vater, der in einem gewissen Bereich liegt
		int parFather = 0;
		for (int j = 1; j < (int)range.size(); j++){
			if (randNb <= range[0]) {
				parFather = 0;
				break;
			}
			else if (randNb > range[j - 1] && randNb <= range[j]){
				parFather = j;
				break;
			}
		}

		randNb = distribution(generator);

		// Wähle zufällig eine Mutter, die in einem gewissen Bereich liegt
		int parMother = 0;
		for (int j = 1; j < (int)range.size(); j++){
			if (randNb <= range[0]) {
				parMother = 0;
				break;
			}
			else if (randNb > range[j - 1] && randNb <= range[j]){
				parMother = j;
				break;
			}
		}

		// Vater und Mutter müssen unterschiedlich sein
		while (parFather == parMother){
			randNb = distribution(generator);
			for (int j = 1; j < (int)range.size(); j++){
				if (randNb <= range[0]) {
					parMother = 0;
					break;
				}
				else if (randNb > range[j - 1] && randNb <= range[j]){
					parMother = j;
					break;
				}
			}
		}

		std::vector<int> child = inheritance(population[parFather], population[parMother]);
		childGen.push_back(child);	// Das soeben geborene Kind ist Teil der neuen Generation

	}

	std::vector<int>::const_iterator fitIter = fitness.cbegin();
	int minFit = *fitIter;
	int index = 0;
	for (; fitIter != fitness.cend(); fitIter++) {
		if (*fitIter < minFit) {
			minFit = *fitIter;
			index = fitIter - fitness.cbegin();
		}
	}
	childGen.push_back(population[index]);	// Das beste alte Ding ist ebenfalls Teil der neuen Generation
	// Damit wird unsere Population immer um 1 größer

	return childGen;
}

std::vector<std::vector<int>> change(std::vector<std::vector<int>> population, double changeRate, int cityNb) {

	// Mutationsfunktion
	std::default_random_engine generator(std::random_device{}());
	std::uniform_real_distribution<double> dis(0, 1);
	std::uniform_int_distribution<int> disInt(0, cityNb - 1);

	for (int i = 0; i < (int)population.size(); i++) {
		double r = dis(generator);
		if (r >= changeRate) {
			int first, second;
			first = disInt(generator);
			second = disInt(generator);

			while (first == second) {
				second = disInt(generator);
			}

			int temp = population[i][first];
			population[i][first] = population[i][second];
			population[i][second] = temp;
		}
	}

	return population;
}

std::vector<int> saveMin(const std::vector<std::vector<int>>& population, const std::vector<int>& fitness) {

	// Bestes Ding abspeichern
	std::vector<int>::const_iterator fitIter = fitness.cbegin();
	int minFit = *fitIter;
	int index = 0;
	for (; fitIter != fitness.cend(); fitIter++) {
		if (*fitIter < minFit) {
			minFit = *fitIter;
			index = fitIter - fitness.cbegin();
		}
	}

	return population[index];
}