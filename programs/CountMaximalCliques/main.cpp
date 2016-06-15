
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
#include <StopWatch.hpp>
#include <GraphLoader.hpp>

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

void test(CliqueEnumerator ce, Graph* graph)
{
	CountingCliqueReceiver cr;
	StopWatch sw;
	sw.start();
	ce(graph, &cr);
	sw.stop();

	double seconds = sw.elapsedSeconds();
	std::cout << cr.count << " " << seconds << std::endl;
}

int main(int argc, const char* argv[])
{
	if ((argc < 3) || ((strcmp(argv[1], "tomita") != 0) && (strcmp(argv[1], "naude") != 0)))
	{
		std::cout << "usage: program (tomita|naude) filename" << std::endl;
		std::cout << "  e.g. program tomita graph.csv" << std::endl;
		std::cout << std::endl;
		std::cout << " tomita     use Tomita et al. pivot selection" << std::endl;
		std::cout << " naude      use Naud\202's pivot selection" << std::endl;
		std::cout << " filename   file containting the graph adjacency matrix in CSV format" << std::endl;
	}
	else
	{
		std::string filename(argv[2]);
		GraphLoader loader(filename);

		Graph* graph = loader.loadAdjacencyMatrix(',', false);

		if (strcmp(argv[1], "tomita") == 0)
		{
			test(&AllCliques_Tomita, graph);
		}
		else
		{
			test(&AllCliques_Naude, graph);
		}

		delete graph;
	}
}
