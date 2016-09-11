
#include <Graph_ErdosRenyi.hpp>

namespace kn
{
    namespace ErdosRenyi
    {
        Graph* Gnp(Random& r, uint32_t n, double p, const AttributeModel* vertexAttributes, const AttributeModel* edgeAttributes)
        {
            uint32_t numVertAttributes = vertexAttributes ? (uint32_t)vertexAttributes->count() : 0;
            uint32_t numEdgeAttributes = edgeAttributes ? (uint32_t)edgeAttributes->count() : 0;
            Graph* g = new Graph(vertexAttributes, edgeAttributes);
            for (uint32_t v = 0; v < n; v++)
            {
                g->addVertex(r.nextUInt(numVertAttributes));
            }

            for (uint32_t v = 0; v < n; v++)
            {
                for (uint32_t u = 0; u < v; u++)
                {
                    if (r.nextDoubleCO() < p)
                    {
                        g->addEdge(u, v, r.nextUInt(numEdgeAttributes));
                    }
                }
            }

            return g;
        }

        void shuffleN(Random& r, std::vector<Graph::Pair>& vec, std::size_t n)
        {
            uint32_t m = (uint32_t)vec.size();
            for (std::size_t c = 0; c < n; c++)
            {
                uint32_t k = r.nextUInt(m);
                if (k != 0)
                {
                    Graph::Pair t = vec[c];
                    vec[c] = vec[c + k];
                    vec[c + k] = t;
                }
                m--;
            }
        }

        Graph* Gnm(Random& r, uint32_t n, uint32_t m, const AttributeModel* vertexAttributes, const AttributeModel* edgeAttributes)
        {
            uint32_t numVertAttributes = vertexAttributes? (uint32_t)vertexAttributes->count() : 0;
            uint32_t numEdgeAttributes = edgeAttributes ? (uint32_t)edgeAttributes->count() : 0;
            Graph* g = new Graph(vertexAttributes, edgeAttributes);
            for (uint32_t v = 0; v < n; v++)
            {
                g->addVertex(r.nextUInt(numVertAttributes));
            }

            std::vector<Graph::Pair> availableEdges = g->listOfAbsentEdges();
            shuffleN(r, availableEdges, m);

            for (std::size_t p = 0; p < m; p++)
            {
                Graph::Pair pair = availableEdges[p];
                g->addEdge(pair.u, pair.v, r.nextUInt(numEdgeAttributes));
            }

            return g;
        }

    }
}
