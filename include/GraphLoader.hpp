
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

		bool isOpen() const
		{
			return stream.is_open();
		}

		Graph* loadAdjacencyMatrix(char delim, bool directed);

		Graph* loadAdjacencyList(char delim, bool directed);
	};

}
