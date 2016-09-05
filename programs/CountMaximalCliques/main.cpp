
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

#ifdef NDEBUG
    typedef CliqueReceiver TheCliqueReceiver;
#else
    typedef PrettyPrintCliqueReceiver TheCliqueReceiver;
#endif

typedef void(*CliqueEnumerator)(const Graph* g, CliqueReceiver* cr);

void test(CliqueEnumerator ce, const Graph& graph)
{
    TheCliqueReceiver cr;
    StopWatch sw;
    sw.start();
    ce(&graph, &cr);
    sw.stop();

    double seconds = sw.elapsedSeconds();
    std::cout << cr.cliqueCount() << " cliques, " << seconds << " seconds, " << cr.recursionCount() << " recursive calls" << std::endl;
}

bool strEndsWith(const char* str, const char* ending)
{
    std::size_t strLength = strlen(str);
    std::size_t endingLength = strlen(ending);
    return (strLength >= endingLength && (0 == strcmp(str + strLength - endingLength, ending)));
}

int main(int argc, const char* argv[])
{
    if ((argc < 3) || ((strcmp(argv[1], "tomita-et-al") != 0) && (strcmp(argv[1], "naude") != 0) && (strcmp(argv[1], "segundo-et-al") != 0)))
    {
        std::cout << "usage: program algorithm [format] filename" << std::endl;
        std::cout << "  e.g. program tomita dimacs graph.dimacs.txt" << std::endl;
        std::cout << std::endl;
        std::cout << " tomita-et-al   use Tomita et al. pivot selection" << std::endl;
        std::cout << " naude          use Naude's pivot selection" << std::endl;
        std::cout << " segundo-et-al  use Segundo et al. pivot selection" << std::endl;
        std::cout << std::endl;
        std::cout << " am             file is adjacency matrix in CSV format" << std::endl;
        std::cout << " al             file is adjacency list in CSV format" << std::endl;
        std::cout << " dimacs         file is in DIMACS ascii format" << std::endl;
        std::cout << " dimacs-b       file is in DIMACS binary format" << std::endl;
        std::cout << " dimacs-at      file is in DIMACS format with attribute extensions" << std::endl;
        std::cout << " dimacs-lin     file contains multiple graphs in linear DIMACS format" << std::endl;
        std::cout << " filename       file containing the graph in the specified format" << std::endl;
    }
    else
    {
        const char* cFilename = argv[2 + ((argc > 3) ? 1 : 0)];
        std::string filename(cFilename);
        std::string format((argc > 3)? argv[2] : "");
        GraphLoader loader(filename);
        std::vector<Graph> graphs;
        Graph* graph = nullptr;

        if (format.length() == 0)
        {
            if (strEndsWith(cFilename, ".am") || strEndsWith(cFilename, ".am.txt") || strEndsWith(cFilename, ".am.csv"))
                format = "am";
            else
            if (strEndsWith(cFilename, ".al") || strEndsWith(cFilename, ".al.txt") || strEndsWith(cFilename, ".al.csv"))
                format = "al";
            else
            if (strEndsWith(cFilename, ".clq") || strEndsWith(cFilename, ".clq.txt") || strEndsWith(cFilename, ".dimacs.txt"))
                format = "dimacs";
            else
            if (strEndsWith(cFilename, ".clq.b") || strEndsWith(cFilename, ".dimacs.b"))
                format = "dimacs-b";
            else
            if (strEndsWith(cFilename, ".dimacs-at.txt"))
                format = "dimacs-at";
            else
            if (strEndsWith(cFilename, ".dimacs-lin.txt"))
                format = "dimacs-lin";
        }

        if (format.length() == 0)
        {
            std::cout << "file format \"" << format << "\" is not understood" << std::endl;
        }
        else
        if (!loader.isOpen())
        {
            std::cout << "file \"" << filename << "\" could not be opened" << std::endl;
        }
        else
        {
            if (format == "am")
            {
                graph = loader.loadAdjacencyMatrix(',', false);
            }
            else
            if (format == "al")
            {
                graph = loader.loadAdjacencyList(',', false);
            }
            else
            if (format == "dimacs")
            {
                graph = loader.loadDIMACS();
            }
            else
            if (format == "dimacs-b")
            {
                graph = loader.loadDIMACSB();
            }
            else
            if (format == "dimacs-at")
            {
                graph = loader.loadAttributedDIMACS();
            }
            else
            if (format == "dimacs-lin")
            {
                loader.loadLinearDIMACS(graphs);
            }

            if (graph)
            {
                graphs.push_back(std::move(*graph));
                delete graph;
            }

            if (graphs.size())
            {
                CliqueEnumerator ce = nullptr;

                if (strcmp(argv[1], "tomita-et-al") == 0)
                {
                    ce = &AllCliques_Tomita;
                }
                else
                if (strcmp(argv[1], "naude") == 0)
                {
                    ce = &AllCliques_Naude;
                }
                else
                if (strcmp(argv[1], "segundo-et-al") == 0)
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
