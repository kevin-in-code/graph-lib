
/**
 * Main_TCS_2015
 * This program is a reimplementation of the code used in the experimental section of my 2015 TCS article.
 * It is slightly more general than the original code, but it implements and tests the same algorithms.
 *
 * @author Kevin A. Naudé
 * @version 1.1
 */

#include <iostream>
#include <string>
#include <algorithm>
#include <BitStructures.hpp>
#include <Graph.hpp>
#include <CliqueEnumeration.hpp>
#include <SmallFunctions.hpp>
#include <MersenneTwister.hpp>
#include <Graph_ErdosRenyi.hpp>
#include <StopWatch.hpp>

using namespace kn;

class CountingCliqueReceiver : public CliqueReceiver
{
public:
	uint64_t count = 0;

	virtual void onClique(const Graph& graph, const IntegerSet& vertices)
	{
		count++;
	}
};

typedef void(*CliqueEnumerator)(const Graph* g, CliqueReceiver* cr);

void testCase(Random& r, CliqueEnumerator ce, uint32_t n, uint32_t d, uint32_t maxCount, bool diffuseError)
{
	uint32_t count = (d >= 65) ? 30 : (d >= 35) ? 100 : 500;
	if (maxCount > 0) count = (std::min)(count, maxCount);

	double goalDensity = d / 100.0;
	double error = 0.0;
	double density = goalDensity;
	uint32_t maxEdges = n * (n - 1) / 2;

	uint32_t m = (uint32_t)roundEven(maxEdges * (density - error));
	density = (double)m / maxEdges;
	error = density - goalDensity;

	for (uint32_t c = 0; c < count; c++)
	{
		Graph* g = ErdosRenyi::Gnm(r, n, m, nullptr, nullptr);
		CountingCliqueReceiver cr;
		StopWatch sw;
		sw.start();
		ce(g, &cr);
		sw.stop();
		delete g;

		double seconds = sw.elapsedSeconds();
		std::cout << n << "," << d << "," << (c + 1) << "," << cr.count << "," << seconds << std::endl;

		if (diffuseError)
		{
			m = (uint32_t)roundEven(maxEdges * (density - error));
			density = (double)m / maxEdges;
			error = density - goalDensity;
		}
	}
}

void test(CliqueEnumerator ce, uint32_t seed, uint32_t maxCount, bool diffuseError)
{
	MersenneTwister random(seed);
	for (uint32_t n = 40; n <= 150; n += 5)
	{
		for (uint32_t d = 5; d <= 95; d += 5)
		{
			if ((n > 90) && (d >= 95)) break;
			if ((n > 100) && (d >= 85)) break;

			testCase(random, ce, n, d, maxCount, diffuseError);
		}
	}
}

int main(int argc, const char* argv[])
{
	if ((argc < 3) || ((strcmp(argv[1], "tomita") != 0) && (strcmp(argv[1], "naude") != 0)))
	{
		std::cout << "usage: program (tomita|naude) seed [maxcount [diffuse]]" << std::endl;
		std::cout << "  e.g. program tomita 123" << std::endl;
		std::cout << std::endl;
		std::cout << " tomita     use Tomita et al. pivot selection" << std::endl;
		std::cout << " naude      use Naud\202's pivot selection" << std::endl;
		std::cout << " seed       integer, seed for Mersenne Twiser initialisation" << std::endl;
		std::cout << " maxcount   integer, maximum sample size in any test case" << std::endl;
		std::cout << " diffuse    yes or no, apply error diffusion to discretely selected densities" << std::endl;
	}
	else
	{
		uint32_t seed = (uint32_t) atol(argv[2]);
		uint32_t maxCount = 0;
		bool distributeError = false;

		if (argc >= 4) maxCount = (uint32_t) atol(argv[3]);
		if (argc >= 5)
		{
			distributeError = (strcmp(argv[4], "yes") == 0) || (strcmp(argv[4], "y") == 0);
		}

		if (strcmp(argv[1], "tomita") == 0)
		{
			test(&AllCliques_Tomita, seed, maxCount, distributeError);
		}
		else
		{
			test(&AllCliques_Naude, seed, maxCount, distributeError);
		}
	}
}
