
#pragma once

/**
* GraphLoader.hpp
* Purpose: Load graphs and digraphs from files.
*
* @author Kevin A. Naudé
* @version 1.1
*/

#include <string>
#include <fstream>
#include <Graph.hpp>

namespace kn
{

	class GraphLoader
	{
	private:
		std::ifstream stream;
		char buffer[4096];

	public:
		GraphLoader(const std::string& filename)
		{
			stream.rdbuf()->pubsetbuf(buffer, sizeof(buffer));
			stream.open(filename);
		}

		Graph* loadAdjacencyMatrix(char delim, bool directed);
	};

}
