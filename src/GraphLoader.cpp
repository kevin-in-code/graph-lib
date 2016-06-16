
#include <GraphLoader.hpp>
#include <algorithm>
#include <istream>
#include <iomanip>
#include <sstream>
#include <limits>
#include <vector>
#include <unordered_map>


namespace kn
{
    Graph* GraphLoader::loadAdjacencyMatrix(char delim, bool directed)
    {
        Graph* g = new Graph();

        std::string line;
        std::size_t numVertices = 0;
        std::size_t row = 0;
        while (std::getline(stream, line))
        {
            std::stringstream lstream(line);
            std::string entry;
            std::size_t column = 0;
            bool none = true;
            while (std::getline(lstream, entry, delim))
            {
                none = false;
                if (column == numVertices)
                {
                    g->addVertex(0);
                    numVertices++;
                }
                int value = std::stoi(entry);
                if (value != 0)
                {
                    if (directed)
                        g->addArc(row, column, 0);
                    else
                    if (!g->hasEdge(row, column))
                    {
                        g->addEdge(row, column, 0);
                    }
                }
                column++;
            }
            if (none) break;
            row++;
        }

        return g;
    }

    Graph* GraphLoader::loadAdjacencyList(char delim, bool directed)
    {
        Graph* g = new Graph();

        std::string line;
        std::size_t numVertices = 0;
        while (std::getline(stream, line))
        {
            std::stringstream lstream(line);
            std::string entry;
            if (std::getline(lstream, entry, delim))
            {
                std::size_t source = (std::size_t) std::stoi(entry) - 1;
                while (source >= numVertices)
                {
                    g->addVertex(0);
                    numVertices++;
                }
                while (std::getline(lstream, entry, delim))
                {
                    std::size_t dest = (std::size_t) std::stoi(entry) - 1;
                    while (dest >= numVertices)
                    {
                        g->addVertex(0);
                        numVertices++;
                    }
                    if (directed)
                        g->addArc(source, dest, 0);
                    else
                    if (!g->hasEdge(source, dest))
                    {
                        g->addEdge(source, dest, 0);
                    }
                }
            }
            else
                break;
        }

        return g;
    }
}

