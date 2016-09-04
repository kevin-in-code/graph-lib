
#include <GraphLoader.hpp>
#include <algorithm>
#include <istream>
#include <iomanip>
#include <sstream>
#include <limits>
#include <vector>
#include <unordered_map>
#include <cctype>
#include <cstdio>
#include <iostream>

namespace kn
{
    void GraphLoader::loadAdjacencyMatrix(Graph& g, char delim, bool directed)
    {
        g.clear();

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
                    g.addVertex(0);
                    numVertices++;
                }
                int value = std::stoi(entry);
                if (value != 0)
                {
                    if (directed)
                        g.addArc(row, column, 0);
                    else
                    if (!g.hasEdge(row, column))
                    {
                        g.addEdge(row, column, 0);
                    }
                }
                column++;
            }
            if (none) break;
            row++;
        }
    }

    Graph* GraphLoader::loadAdjacencyMatrix(char delim, bool directed)
    {
        Graph* g = new Graph();
        loadAdjacencyMatrix(*g, delim, directed);
        return g;
    }

    void GraphLoader::loadAdjacencyList(Graph& g, char delim, bool directed)
    {
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
                    g.addVertex(0);
                    numVertices++;
                }
                while (std::getline(lstream, entry, delim))
                {
                    std::size_t dest = (std::size_t) std::stoi(entry) - 1;
                    while (dest >= numVertices)
                    {
                        g.addVertex(0);
                        numVertices++;
                    }
                    if (directed)
                        g.addArc(source, dest, 0);
                    else
                    if (!g.hasEdge(source, dest))
                    {
                        g.addEdge(source, dest, 0);
                    }
                }
            }
            else
                break;
        }
    }

    Graph* GraphLoader::loadAdjacencyList(char delim, bool directed)
    {
        Graph* g = new Graph();
        loadAdjacencyList(*g, delim, directed);
        return g;
    }

    void GraphLoader::loadDIMACS(Graph& g)
    {
        std::string line;
        std::size_t numVertices = 0;
        while (std::getline(stream, line))
        {
            std::stringstream lstream(line);
            
            std::string key = "";
            lstream >> key;
            
            if (key != "")
            {
                if (key == "e")
                {
                    std::size_t source, dest;
                    lstream >> source;
                    lstream >> dest;
                    source--;
                    dest--;
                    while (source >= numVertices)
                    {
                        g.addVertex(0);
                        numVertices++;
                    }
                    while (dest >= numVertices)
                    {
                        g.addVertex(0);
                        numVertices++;
                    }
                    if (!g.hasEdge(source, dest))
                    {
                        g.addEdge(source, dest, 0);
                    }
                }
            }
            else
                break;
        }
    }

    Graph* GraphLoader::loadDIMACS()
    {
        Graph* g = new Graph();
        loadDIMACS(*g);
        return g;
    }

    void GraphLoader::loadDIMACSB(Graph& g)
    {
        int preambleSize = 0;

        // read header
        stream >> preambleSize;
        stream.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // skip end-to-line

        // skip preamble
        stream.seekg(preambleSize, std::ios_base::cur);

        // read data
        std::vector<unsigned char> row;
        row.reserve(256);
        std::size_t i = 0;
        for (i = 0; ; i++)
        {
            std::size_t rowSize = (i + 8) / 8;
            row.clear();
            row.resize(rowSize, 0);

            stream.read((char*) &row[0], rowSize);
            if (!stream) break;

            // we now have data for another vertex
            g.addVertex(0);

            for (std::size_t j = 0; j <= i; j++)
            {
                unsigned char bit = (unsigned char)(1 << (7 - (j & 7)));
                std::size_t index = j >> 3;

                if ((row[index] & bit) == bit)
                {
                    g.addEdge(i, j, 0);
                }
            }
        }

        // note: the originally published loader would fail if the graph size was not present in the preamble
    }

    Graph* GraphLoader::loadDIMACSB()
    {
        Graph* g = new Graph();
        loadDIMACSB(*g);
        return g;
    }

    void GraphLoader::loadAttributedDIMACS(Graph& g)
    {
        std::string line;
        std::size_t numVertices = 0;
        while (std::getline(stream, line))
        {
            std::stringstream lstream(line);

            std::string key = "";
            lstream >> key;

            if (key != "")
            {
                if (key == "v")
                {
                    std::size_t attr;
                    lstream >> attr;
                    if (!lstream) attr = 0;

                    g.addVertex(attr);
                    numVertices++;
                }
                else
                if (key == "e")
                {
                    std::size_t source, dest;
                    std::size_t attr;
                    lstream >> source;
                    lstream >> dest;
                    lstream >> attr;
                    if (!lstream) attr = 0;
                    source--;
                    dest--;
                    while (source >= numVertices)
                    {
                        g.addVertex(0);
                        numVertices++;
                    }
                    while (dest >= numVertices)
                    {
                        g.addVertex(0);
                        numVertices++;
                    }
                    if (!g.hasEdge(source, dest))
                    {
                        g.addEdge(source, dest, attr);
                    }
                }
            }
            else
                break;
        }
    }

    Graph* GraphLoader::loadAttributedDIMACS()
    {
        Graph* g = new Graph();
        loadAttributedDIMACS(*g);
        return g;
    }

    void GraphLoader::loadLinearDIMACS(Graph& g, const std::string dimacs)
    {
        std::size_t numVertices = 0;
        std::stringstream lstream(dimacs);

        std::string key = "";
        while (lstream)
        {
            key = "";
            lstream >> key;

            if (key != "")
            {
                if (key == "v")
                {
                    std::size_t attr;
                    lstream >> attr;
                    if (!lstream) attr = 0;

                    g.addVertex(attr);
                    numVertices++;
                }
                else
                if (key == "e")
                {
                    std::size_t source, dest;
                    std::size_t attr = 0;
                    lstream >> source;
                    lstream >> dest;
                    lstream >> std::ws;
                    if (std::isdigit(lstream.peek())) lstream >> attr;
                    source--;
                    dest--;
                    while (source >= numVertices)
                    {
                        g.addVertex(0);
                        numVertices++;
                    }
                    while (dest >= numVertices)
                    {
                        g.addVertex(0);
                        numVertices++;
                    }
                    if (!g.hasEdge(source, dest))
                    {
                        g.addEdge(source, dest, attr);
                    }
                }
                else
                    break;
            }
        }
    }

    Graph* GraphLoader::loadLinearDIMACS(const std::string dimacs)
    {
        Graph* g = new Graph();
        loadLinearDIMACS(*g, dimacs);
        return g;
    }

    void GraphLoader::loadLinearDIMACS(std::vector<Graph>& graphs, bool append)
    {
        if (!append) graphs.clear();
        std::string line;
        while (std::getline(stream, line))
        {
            std::stringstream lstream(line);
            lstream >> std::ws;
            if (lstream)
            {
                Graph g;
                GraphLoader::loadLinearDIMACS(g, line);
                graphs.push_back(std::move(g));
            }
        }
    }
}

