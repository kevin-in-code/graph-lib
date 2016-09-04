
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
    uint64_t size = 0;

    virtual void onClique(const Graph& graph, const IntegerSet& vertices)
    {
        count++;
    }

    virtual void onApply()
    {
        size++;
    }
};

typedef void(*CliqueEnumerator)(const Graph* g, CliqueReceiver* cr);

void test(CliqueEnumerator ce, const Graph& graph)
{
    CountingCliqueReceiver cr;
    StopWatch sw;
    sw.start();
    ce(&graph, &cr);
    sw.stop();

    double seconds = sw.elapsedSeconds();
    std::cout << cr.count << " cliques, " << seconds << " seconds, " << cr.size << " recursive calls" << std::endl;
}

int main(int argc, const char* argv[])
{
    if ((argc < 4) || ((strcmp(argv[1], "tomita") != 0) && (strcmp(argv[1], "naude") != 0) && (strcmp(argv[1], "segundo") != 0)))
    {
        std::cout << "usage: program algorithm format filename" << std::endl;
        std::cout << "  e.g. program tomita dimacs graph.dimacs.txt" << std::endl;
        std::cout << std::endl;
        std::cout << " tomita     use Tomita et al. pivot selection" << std::endl;
        std::cout << " naude      use Naude's pivot selection" << std::endl;
        std::cout << " segundo    use Segundo et al. pivot selection" << std::endl;
        std::cout << " am         file is adjacency matrix in CSV format" << std::endl;
        std::cout << " al         file is adjacency list in CSV format" << std::endl;
        std::cout << " dimacs     file is in DIMACS ascii format" << std::endl;
        std::cout << " dimacs-b   file is in DIMACS binary format" << std::endl;
        std::cout << " dimacs-at  file is in DIMACS format with attribute extensions" << std::endl;
        std::cout << " dimacs-lin file contains multiple graphs in linear DIMACS format" << std::endl;
        std::cout << " filename   file containing the graph in the specified format" << std::endl;
    }
    else
    {
        std::string filename(argv[3]);
        GraphLoader loader(filename);
        std::vector<Graph> graphs;
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
            if (strcmp(argv[2], "dimacs") == 0)
            {
                graph = loader.loadDIMACS();
            }
            else
            if (strcmp(argv[2], "dimacs-b") == 0)
            {
                graph = loader.loadDIMACSB();
            }
            else
                if (strcmp(argv[2], "dimacs-at") == 0)
            {
                graph = loader.loadAttributedDIMACS();
            }
            else
            if (strcmp(argv[2], "dimacs-lin") == 0)
            {
                loader.loadLinearDIMACS(graphs);
            }
            else
            {
                std::cout << "argument \"" << argv[2] << "\" is not a recognised graph format" << std::endl;
            }

            if (graph)
            {
                graphs.push_back(std::move(*graph));
                delete graph;
            }

            if (graphs.size())
            {
                CliqueEnumerator ce = nullptr;

                if (strcmp(argv[1], "tomita") == 0)
                {
                    ce = &AllCliques_Tomita;
                }
                else
                if (strcmp(argv[1], "naude") == 0)
                {
                    ce = &AllCliques_Naude;
                }
                else
                if (strcmp(argv[1], "segundo") == 0)
                {
                    ce = &AllCliques_Segundo;
                }
                else
                {
                    std::cout << "argument \"" << argv[1] << "\" is not a recognised maximal clique enumerator" << std::endl;
                }

                if (ce)
                {
                    for (std::size_t c = 0; c < graphs.size(); c++) {
                        test(ce, graphs[c]);
                    }
                }
            }
        }
    }
}
