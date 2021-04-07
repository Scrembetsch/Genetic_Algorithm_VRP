#include <vector>
#include <random>

struct Road
{
	std::string City1;
	std::string City2;
	int			Distance;

	bool Parse(const std::string& roadString);
};

class GeneticAlgorithm
{
public:
	GeneticAlgorithm();
	~GeneticAlgorithm();

	bool ReadFile(std::string path, bool calculateMissingRoutes);
	void SolveVRP();
	std::vector<std::vector<int>> InitPopulation();
	int EvaluateFitness(const std::vector<int>& populationRoute);
	std::vector<std::vector<int>> CreateNewGeneration(const std::vector<std::vector<int>>& population, const std::vector<int>& fitness);
	std::vector<int> Crossover(const std::vector<int>& father, std::vector<int> mother);
	std::vector<std::vector<int>> Mutate(std::vector<std::vector<int>> population);
	std::vector<int> SaveBest(const std::vector<std::vector<int>>& population, const std::vector<int>& fitness);
	void PrintOutput(std::vector<std::vector<int>> bestSolutions);

	int**	mDistances;
	int		mNumCities;				// Number of cities
	int		mPopulationSize = 300;	// Initial population size
	int		mIterations = 200;		// Number of iterations 	
	double	mMutationRate = 0.1;	// Probability of mutation

	std::vector<std::string>		mCities;
	std::vector<std::vector<int>>	mBestSolutions;

private:
	int OptimatizeRoutes();
	int FindShortestPath(int start, int current, int destination, int currentDistance);
	void PrintDistances();
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