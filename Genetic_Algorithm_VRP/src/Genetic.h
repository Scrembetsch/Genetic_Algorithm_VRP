#include <vector>
#include <random>

struct Road
{
	std::string City1;
	std::string City2;
	int			Distance;

	bool Parse(const std::string& roadString);
};

struct City
{
	std::string Name;
	float X;
	float Y;

	bool Parse(const std::string& cityString);
};

class GeneticAlgorithm
{
public:
	// Init needs to be done in cpp to compile on Linux
	static const int sBlank;
	static const int sVehicles;

	GeneticAlgorithm();
	GeneticAlgorithm(const GeneticAlgorithm& ga);
	~GeneticAlgorithm();

	bool ReadFile(std::string path, bool calculateMissingRoutes);
	void SolveVRP();
	int** InitPopulation();
	int EvaluateFitness(int* populationRoute) const;
	int** CreateNewGeneration(int** population, int* fitness);
	int* createInversionSequence(int* individual);
	int* recreateNumbers(int* inversionSequence, int size);
	int* Crossover(int* father, int* mother);
	void sort(int** population, int* fitness, int l, int r);
	bool CheckSwap(int* router, int first, int second) const;
	int** Mutate(int** population);
	int* SaveBest(int** population, int* fitness);
	int* GetBest() const;
	void PrintOutput(int* solution) const;

	int**	mDistances;				// All distances between cities
	int		mNumCities;				// Number of cities
	int		mRouteSize;				// Length of Route array
	int		mPopulationSize;		// Initial population size
	int		mIterations;			// Number of iterations
	double	mMutationRate;			// Probability of mutation

	std::vector<City>				mCities;
	int*							mBestSolution;

private:
	void PrintDistances() const;
	void PrintCities() const;
	bool ValidateRoute(int* route, bool assertOnError) const;
};