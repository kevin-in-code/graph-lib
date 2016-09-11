
/**
 * CountingCliqueReceiver
 * This program is a basic example of clique enumeration applied to text graph file.
 *
 * @author Kevin A. Naudé
 * @version 1.1
 */

#include <iostream>
#include <iomanip>
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

struct CliqueEnumerationMethod
{
    std::string handle;
    std::string name;
    CliqueEnumerator enumerator;
};

std::vector<CliqueEnumerationMethod> Methods = {
    CliqueEnumerationMethod{ "tomita-et-al", "Tomita et al.", &AllCliques_Tomita },
    CliqueEnumerationMethod{ "naude", "Naude", &AllCliques_Naude }
};

bool validMethod(const char* name)
{
    for (auto it = Methods.begin(); it != Methods.end(); it++)
    {
        if (it->handle == name) return true;
    }
    return false;
}

std::string formatText(std::string name, int width)
{
    std::stringstream ss;
    ss << std::setw(width) << std::left << name;
    return ss.str();
}

int main(int argc, const char* argv[])
{
    if ((argc < 3) || !validMethod(argv[1]))
    {
        std::cout << "usage: program algorithm [format] filename" << std::endl;
        std::cout << "  e.g. program tomita dimacs graph.dimacs.txt" << std::endl;
        std::cout << std::endl;
        for (auto it = Methods.begin(); it != Methods.end(); it++)
        {
            std::cout << " " << formatText(it->handle, 15) << "use method of " << it->name << std::endl;
        }
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
                std::string goal = argv[1];

                for (std::size_t m = 0; m < Methods.size(); m++)
                {
                    CliqueEnumerationMethod cm = Methods[m];

                    if ((goal != "") && (goal != cm.handle)) continue;

                    for (std::size_t c = 0; c < graphs.size(); c++) {
                        test(cm.enumerator, graphs[c]);
                    }
                }
            }
        }
    }
}
