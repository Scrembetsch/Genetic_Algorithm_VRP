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
	std::vector<std::vector<int>> InitPopulation();
	int EvaluateFitness(const std::vector<int>& populationRoute) const;
	std::vector<std::vector<int>> CreateNewGeneration(std::vector<std::vector<int>> population, std::vector<int> fitness);
	int* createInversionSequence(std::vector<int> individual);
	std::vector<int> recreateNumbers(int* inversionSequence, int size);
	std::vector<int> Crossover(std::vector<int> father, std::vector<int> mother);
	void sort(std::vector<std::vector<int>>& population, std::vector<int>& fitness, int l, int r);
	std::vector<std::vector<int>> Mutate(std::vector<std::vector<int>> population);
	std::vector<int> SaveBest(const std::vector<std::vector<int>>& population, const std::vector<int>& fitness);
	const std::vector<int>& GetBest() const;
	void PrintOutput(const std::vector<int>& solution) const;

	int**	mDistances;				// All distances between cities
	int		mNumCities;				// Number of cities
	int		mPopulationSize;		// Initial population size
	int		mIterations;			// Number of iterations
	double	mMutationRate;			// Probability of mutation

	std::vector<City>				mCities;
	std::vector<std::vector<int>>	mBestSolutions;

private:
	void PrintDistances();
	void PrintCities();
};

// TODO: Delete old functions

// std::vector<std::vector<int>> inTxt(std::string path, int cityNb);

// std::vector<std::vector<int>> initPath(int route, int nb);

// int fitness(const std::vector<int>& sroute, const std::vector<std::vector<int>>& length);

// std::vector<double> choseRange(std::vector<int> fitness, int ep);	// TODO: delete

// std::vector<int> inheritance(const std::vector<int>& father, std::vector<int> mother);

// std::vector<std::vector<int>> mutate(const std::vector<std::vector<int>>& population, const std::vector<double>& range, const std::vector<int>& fitness);

// std::vector<std::vector<int>> change(std::vector<std::vector<int>> population, double changeRate, int cityNb);

// std::vector<int> saveMin(const std::vector<std::vector<int>>& population, const std::vector<int>& fitness);