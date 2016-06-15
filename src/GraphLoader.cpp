
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
			while (std::getline(lstream, entry, delim))
			{
				if (column == numVertices)
				{
					g->addVertex(0);
					numVertices++;
				}
				int value = std::stoi(entry);
				if (value != 0)
				{
					if (directed || (!g->hasEdge(row, column)))
					{
						if (directed)
							g->addArc(row, column, 0);
						else
							g->addEdge(row, column, 0);
					}
				}
				column++;
			}
			row++;
		}

		return g;
	}
}
