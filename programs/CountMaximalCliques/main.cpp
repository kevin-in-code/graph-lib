
/**
 * CountingCliqueReceiver
 * This program is a basic example of clique enumeration applied to text graph file.
 *
 * @author Kevin A. Naudé
 * @version 1.1
 */

#include <iostream>
#include <string>
#include <string.h>
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
	if ((argc < 4) || ((strcmp(argv[1], "tomita") != 0) && (strcmp(argv[1], "naude") != 0)))
	{
		std::cout << "usage: program (tomita|naude) (am|al) filename" << std::endl;
		std::cout << "  e.g. program tomita al graph.al.csv" << std::endl;
		std::cout << std::endl;
		std::cout << " tomita     use Tomita et al. pivot selection" << std::endl;
		std::cout << " naude      use Naudé's pivot selection" << std::endl;
		std::cout << " am         file is adjacency matrix in CSV format" << std::endl;
		std::cout << " al         file is adjacency list in CSV format" << std::endl;
		std::cout << " filename   file containing the graph in the specified format" << std::endl;
	}
	else
	{
		std::string filename(argv[3]);
		GraphLoader loader(filename);
		Graph* graph = nullptr;

		if (!loader.isOpen())
		{
			std::cout << "file \"" << argv[3] << "\" could not be opened" << std::endl;
		}
		else
		{
			if (strcmp(argv[2], "am") == 0)
			{
				graph = loader.loadAdjacencyMatrix(',', false);
			}
			else
				if (strcmp(argv[2], "al") == 0)
				{
					graph = loader.loadAdjacencyList(',', false);
				}
				else
				{
					std::cout << "argument \"" << argv[2] << "\" is not a recognised graph format" << std::endl;
				}

			if (graph)
			{
				if (strcmp(argv[1], "tomita") == 0)
				{
					test(&AllCliques_Tomita, graph);
				}
				else
					if (strcmp(argv[1], "naude") == 0)
					{
						test(&AllCliques_Naude, graph);
					}
					else
					{
						std::cout << "argument \"" << argv[1] << "\" is not a recognised maximal clique enumerator" << std::endl;
					}

				delete graph;
			}
		}
	}
}
