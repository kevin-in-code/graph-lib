
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
        char* buffer;

        static constexpr std::size_t BufferSize = 4096;

    public:
        GraphLoader(const std::string& filename)
        {
            /**
                NB: Certain versions of g++ fail if the buffer passed into pubsetbuf
                    is part of the same memory allocation.  Thus buffer is allocated
                    separately here.
            */
            this->buffer = new char[BufferSize];
            stream.rdbuf()->pubsetbuf(buffer, BufferSize);
            stream.open(filename);
        }

        ~GraphLoader()
        {
            delete[] this->buffer;
        }

        bool isOpen() const
        {
            return stream.is_open();
        }

        Graph* loadAdjacencyMatrix(char delim, bool directed);

        Graph* loadAdjacencyList(char delim, bool directed);

        Graph* loadDIMACS();

        Graph* loadAttributedDIMACS();

        static Graph* loadLinearDIMACS(const std::string dimacs);

        std::vector<Graph*> loadLinearDIMACS();
    };

}
