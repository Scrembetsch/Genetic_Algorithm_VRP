#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <cmath>

#include "genetic.h"

using namespace std;
using std::vector;

vector<vector<int>> inTxt(string path, int cityNb) {
	// TODO: parse our file
	// Knotenpunkt ist die Stadt mit den meisten Stra�enverbindungen, alternativ die Stadt die am zentralsten liegt
	// Zwischenspeichern wie das danzig24 file (siehe Discord), wobei keine Stra�enverbindungen mit -1 gekennzeichnet sind
	ifstream infile(path);

	string line;
	vector<vector<int>> distance(cityNb, vector<int>(cityNb));

	int lineCount = 0;
	int tempDis = 0;


	while (getline(infile, line)) {

		int nb = 0;

		istringstream iss(line);
		while (iss >> tempDis && nb < cityNb) {

			distance[lineCount][nb] = tempDis;
			nb = nb + 1;

		}

		lineCount = lineCount + 1;

	}

	return distance;
}

vector<vector<int>> initPath(int route, int nb) {

	vector<vector<int>> population(route, vector<int>(nb));
	// vector<int>(routeVehicle1|blank|routeVehicle2|blank|routeVehicle3|blank|routeVehicle4|blank|routeVehicle5)	-> siehe Discord Paper
	// Wichtig: g�ltige initiale Population erzeugen, sodass alle 5 Fahrzeuge ungef�hr die gleiche Anzahl an St�dten befahren 

	default_random_engine generator(std::random_device{}());

	int city;
	for (int i = 0; i < route; i++) {

		vector<int> place;
		for (int k = 0; k < nb; k++) {
			place.push_back(k);
		}

		int s = nb;
		for (int j = 0; j < nb; j++) {


			uniform_int_distribution<int> distribution(0, s - 1);

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

int fitness(const vector<int>& sroute, const vector<vector<int>>& length) {
	// Mittelwert der zur�ckgelegten Distanz der Trucks berechnen = Summe der einzelnen Truck-Distanzen/5
	// Fitness ist Gewichtung1 (z.B. 0.5) * Summe der einzelnen Truck-Distanzen)/5 + Gewichtung2 (z.B. 0.5) * Gesamtstrecke aller Trucks
	// Fitness Wert soll m�glichst klein sein
	// TODO: Wenn -1 ausgelesen wird -> durch INT_MAX ersetzen
	int s = sroute.size();

	int routeLength = 0;
	for (int i = 1; i < s; i++) {
		routeLength += length[sroute[i - 1]][sroute[i]];
	}
	routeLength += length[sroute[s - 1]][sroute[0]];

	return routeLength;
}

vector<double> choseRange(vector<int> fitness, int ep) {

	// ???
	// TODO: dont use this

	vector<int>::const_iterator fitIter = fitness.cbegin();
	int minFit = *fitIter;
	for (; fitIter != fitness.cend(); fitIter++) {
		if (*fitIter < minFit) {
			minFit = *fitIter;
		}
	}

	//cout << "Minimun distance: " << minFit << endl;

	vector<double> trueFitness;
	double total = 0;
	for (fitIter = fitness.cbegin(); fitIter != fitness.cend(); fitIter++) {
		double ratio = double(*fitIter) / double(minFit);
		double exponent = -pow(ratio, ep);
		total += exp(exponent);	// das ergebnis von exp() ist zwischen 0 und 1
		trueFitness.push_back(total);
	}

	vector<double> range(fitness.size());
	for (int i = 0; i < (int)fitness.size(); i++) {
		range[i] = trueFitness[i] / total;
	}

	return range;	// range[i] ist zwischen 0 und 1 und umso h�her die Distanz zum minimalen Fitness Wert war, desto h�her ist range[i]
}

vector<int> inheritance(const vector<int>& father, vector<int> mother) {

	if (father.size() != mother.size()) {
		cout << "Fuck! Length is different." << endl;
	}

	int s = father.size();	// Kind ist immer so gro� wie der Vater, auch wenn Vater und Mutter unterschiedlich gro� sind
	default_random_engine generator(std::random_device{}());
	uniform_int_distribution<int> distribution(0, s - 1);

	int p1, p2;
	p1 = distribution(generator);
	p2 = distribution(generator);

	// Generiere zwei zuf�llige Werte, die nicht gleich sein d�rfen und nicht gr��er als der Vater sein d�rfen
	while (p1 == p2) {
		p2 = distribution(generator);
	}

	if (p1 > p2) {
		int temp = p1;
		p1 = p2;
		p2 = temp;
	}

	vector<int> child(s);

	// Alles was wir vom Vater nehmen, nehmen wir nicht von der Mutter (daher l�schen wir es)
	for (int i = p1; i <= p2; i++) {
		for (int j = 0; j < (int)mother.size(); j++) {
			if (mother[j] == father[i]) {
				mother.erase(mother.begin() + j);
				break;
			}
		}
	}

	// Kind mit Mutter und Vater bef�llen
	vector<int>::const_iterator mIter = mother.cbegin();
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

vector<vector<int>> mutate(const vector<vector<int>>& population, const vector<double>& range, const vector<int>& fitness) {

	// TODO: Sortiere Population nach Fitness Value und w�hle zuf�llig Vater + Mutter aus der besseren H�lfte 

	int routeNb = population.size();

	default_random_engine generator(std::random_device{}());

	vector<vector<int>> childGen;
	uniform_real_distribution<double> distribution(0, 1);
	for (int i = 0; i < routeNb; i++) {
		double randNb;
		randNb = distribution(generator);	// Zufallszahl zwischen 0 und 1

		// W�hle zuf�llig einen Vater, der in einem gewissen Bereich liegt
		int parFather = 0;
		for (int j = 1; j < range.size(); j++){
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

		// W�hle zuf�llig eine Mutter, die in einem gewissen Bereich liegt
		int parMother = 0;
		for (int j = 1; j < range.size(); j++){
			if (randNb <= range[0]) {
				parMother = 0;
				break;
			}
			else if (randNb > range[j - 1] && randNb <= range[j]){
				parMother = j;
				break;
			}
		}

		// Vater und Mutter m�ssen unterschiedlich sein
		while (parFather == parMother){
			randNb = distribution(generator);
			for (int j = 1; j < range.size(); j++){
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

		vector<int> child = inheritance(population[parFather], population[parMother]);
		childGen.push_back(child);	// Das soeben geborene Kind ist Teil der neuen Generation

	}

	vector<int>::const_iterator fitIter = fitness.cbegin();
	int minFit = *fitIter;
	int index = 0;
	for (; fitIter != fitness.cend(); fitIter++) {
		if (*fitIter < minFit) {
			minFit = *fitIter;
			index = fitIter - fitness.cbegin();
		}
	}
	childGen.push_back(population[index]);	// Das beste alte Ding ist ebenfalls Teil der neuen Generation
	// Damit wird unsere Population immer um 1 gr��er

	return childGen;
}

vector<vector<int>> change(vector<vector<int>> population, double changeRate, int cityNb) {

	// Mutationsfunktion
	default_random_engine generator(std::random_device{}());
	uniform_real_distribution<double> dis(0, 1);
	uniform_int_distribution<int> disInt(0, cityNb - 1);

	for (int i = 0; i < population.size(); i++) {
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

vector<int> saveMin(const vector<vector<int>>& population, const vector<int>& fitness) {

	// Bestes Ding abspeichern
	vector<int>::const_iterator fitIter = fitness.cbegin();
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