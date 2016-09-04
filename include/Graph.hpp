
#pragma once

/**
 * Graph.hpp
 * Purpose: An class for the representation of attributed graphs and digraphs.
 *
 * @author Kevin A. Naudé
 * @version 1.1
 */

#include <algorithm>
#include <assert.h>
#include <string>
#include <limits>
#include <vector>
#include <unordered_map>
#include <memory>
#include <BitStructures.hpp>
#include <AttributeModel.hpp>
#include <Matrix.hpp>

namespace kn
{

    class Graph
    {
    public:
        typedef std::size_t VertexID;
        typedef std::size_t EdgeID;
        typedef std::size_t AttrID;

        struct Edge
        {
            EdgeID id;
            VertexID u;
            VertexID v;
            AttrID attrID;
            bool undirected;
        };

        struct Vertex
        {
            VertexID id;
            std::size_t outDegree;
            std::size_t inDegree;
            AttrID attrID;
        };

        struct Pair
        {
            VertexID u;
            VertexID v;

            Pair() {}

            Pair(VertexID u, VertexID v)
            {
                this->u = u;
                this->v = v;
            }
        };

        class VertexIterator;
        class EdgeIterator;
        class Walker;

    private:
        struct EdgeInfo : Edge
        {
            EdgeInfo* prevToDestination;
            EdgeInfo* nextToDestination;

            EdgeInfo* prevFromSource;
            EdgeInfo* nextFromSource;
        };

        struct VertexInfo : Vertex
        {
            EdgeInfo* destinationEdges;
            EdgeInfo* sourceEdges;
        };

    private:
        const AttributeModel* vertexAttributes;
        const AttributeModel* edgeAttributes;

        std::vector<VertexInfo> vertices;
        std::unordered_map<VertexID, std::size_t> vertexIDtoIndex;
        std::unordered_map<EdgeID, VertexID> edgeIDtoSourceID;

        VertexID nextVertexID;
        EdgeID nextEdgeID;

        void deleteEdge(EdgeInfo* e);
        void removeEdgeHelper(VertexID sourceID, VertexID destinationID);

        void insertEdge(EdgeID id, VertexID sourceID, VertexID destinationID, AttrID attrID, bool undirected);

        void vertexAdjacency(VertexID id, IntegerSet& row);

    public:
        Graph();
        Graph(const AttributeModel* vertexAttributeModel, const AttributeModel* edgeAttributeModel);

        Graph(const Graph& other) : Graph(other, false)
        {
        }

        Graph(const Graph& other, bool complement);

        Graph(const Graph& other, const std::vector<VertexID>& permutation);

        virtual ~Graph();

        Graph(Graph&& other);
        Graph& operator=(Graph&& other);
        Graph& operator=(const Graph& other);

        void clear()
        {
            vertices.clear();
            vertexIDtoIndex.clear();
            edgeIDtoSourceID.clear();

            nextVertexID = 0;
            nextEdgeID = 0;
        }

        const AttributeModel* getVertexAttributeModel() const
        {
            return vertexAttributes;
        }

        const AttributeModel* getEdgeAttributeModel() const
        {
            return edgeAttributes;
        }

        std::unique_ptr<IntegerSet> vertexAdjacency(VertexID id)
        {
            std::unique_ptr<IntegerSet> adj(new IntegerSet(vertices.size()));
            vertexAdjacency(id, *adj);
            return adj;
        }

        std::unique_ptr<std::vector<IntegerSet>> adjacency();

        template <typename T>
        void constructAdjacencyMatrix(Matrix<T>& m) const
        {
            m.reshape(vertices.size(), vertices.size());
            for (std::size_t u = 0; u < vertices.size(); u++)
            {
                for (std::size_t v = 0; v < vertices.size(); v++)
                {
                    if (hasArc(vertices[u].id, vertices[v].id))
                    {
                        m.setValue(u, v, 1);
                    }
                    else
                    {
                        m.setValue(u, v, 0);
                    }
                }
            }
        }

        std::size_t countVertices() const
        {
            return vertices.size();
        }

        std::size_t countEdges() const
        {
            return edgeIDtoSourceID.size();
        }

        VertexIterator vertexIterator() const
        {
            return VertexIterator(this);
        }

        EdgeIterator exitingEdgeIterator(VertexID id) const
        {
            std::size_t index = vertexIDtoIndex.at(id);
            const VertexInfo* u = &vertices[index];
            return EdgeIterator(u->sourceEdges, true);
        }

        EdgeIterator enteringEdgeIterator(VertexID id) const
        {
            std::size_t index = vertexIDtoIndex.at(id);
            const VertexInfo* v = &vertices[index];
            return EdgeIterator(v->destinationEdges, false);
        }

        bool validVertexID(VertexID id) const
        {
            return (vertexIDtoIndex.find(id) != vertexIDtoIndex.end());
        }

        VertexID getVertexID(std::size_t index) const
        {
            if (index < vertices.size())
            {
                return vertices[index].id;
            }
            else
            {
                return 0;
            }
        }

        bool getVertex(VertexID id, Vertex& v) const
        {
            if (!validVertexID(id))
            {
                // This line prevents a spurious compiler warning.
                v.id = v.outDegree = v.inDegree = v.attrID = 0;
                return false;
            }

            v = vertices[vertexIDtoIndex.at(id)];
            return true;
        }

        bool getVertexByIndex(std::size_t index, Vertex& v) const
        {
            if (index >= vertices.size())
            {
                // This line prevents a spurious compiler warning.
                v.id = v.outDegree = v.inDegree = v.attrID = 0;
                return false;
            }

            v = vertices[index];
            return true;
        }

        bool getEdge(EdgeID id, Edge& e) const;

        bool getEdge(VertexID sourceID, VertexID destinationID, Edge& e) const;

        VertexID addVertex(AttrID attrID)
        {
            VertexID id = nextVertexID++;

            std::size_t index = vertices.size();
            VertexInfo v;
            v.id = id;
            v.outDegree = 0;
            v.inDegree = 0;
            v.sourceEdges = nullptr;
            v.destinationEdges = nullptr;
            v.attrID = attrID;
            vertices.push_back(v);

            vertexIDtoIndex.insert(std::make_pair(id, index));

            return id;
        }

        bool removeVertex(VertexID id);

        bool removeEdge(EdgeID id)
        {
            Edge e;
            if (getEdge(id, e))
                return removeEdge(e.u, e.v);
            else
                return false;
        }

        bool removeEdge(VertexID sourceID, VertexID destinationID);

        bool hasArc(VertexID sourceID, VertexID destinationID) const;

        bool hasEdge(VertexID sourceID, VertexID destinationID) const;

        EdgeID addArc(VertexID sourceID, VertexID destinationID, AttrID attrID)
        {
            if (!validVertexID(sourceID) || !validVertexID(destinationID))
                return 0;
            else
            {
                EdgeID id = nextEdgeID++;
                insertEdge(id, sourceID, destinationID, attrID, false);
                edgeIDtoSourceID.insert(std::make_pair(id, sourceID));
                return id;
            }
        }

        EdgeID addEdge(VertexID sourceID, VertexID destinationID, AttrID attrID)
        {
            if (!validVertexID(sourceID) || !validVertexID(destinationID))
                return 0;
            else
            {
                EdgeID id = nextEdgeID++;

                if (sourceID > destinationID)
                {
                    // reordering the end points is not strictly required.
                    std::swap(sourceID, destinationID);
                }

                insertEdge(id, sourceID, destinationID, attrID, true);
                if (sourceID != destinationID)
                {
                    insertEdge(id, destinationID, sourceID, attrID, true);
                }
                edgeIDtoSourceID.insert(std::make_pair(id, sourceID));

                return id;
            }
        }

        std::vector<VertexID> listOfVertices()
        {
            std::vector<VertexID> result(vertices.size());
            for (auto it = vertices.begin(); it != vertices.end(); ++it)
            {
                result.push_back(it->id);
            }
            return result;
        }

        std::vector<Pair> listOfEdges()
        {
            std::vector<Pair> result(countEdges());
            for (std::size_t ui = 0; ui < vertices.size(); ui++)
            {
                VertexID u = vertices[ui].id;

                for (std::size_t vi = 0; vi < vertices.size(); vi++)
                {
                    VertexID v = vertices[vi].id;

                    if ((u <= v) && hasEdge(u, v))
                    {
                        result.push_back(Pair(u, v));
                    }
                }
            }
            return result;
        }

        std::vector<Pair> listOfArcs()
        {
            std::vector<Pair> result(countEdges());
            for (std::size_t ui = 0; ui < vertices.size(); ui++)
            {
                VertexID u = vertices[ui].id;

                for (std::size_t vi = 0; vi < vertices.size(); vi++)
                {
                    VertexID v = vertices[vi].id;

                    if (hasArc(u, v))
                    {
                        result.push_back(Pair(u, v));
                    }
                }
            }
            return result;
        }

        std::vector<Pair> listOfAbsentEdges()
        {
            std::vector<Pair> result(countEdges());
            for (std::size_t ui = 0; ui < vertices.size(); ui++)
            {
                VertexID u = vertices[ui].id;

                for (std::size_t vi = 0; vi < vertices.size(); vi++)
                {
                    VertexID v = vertices[vi].id;

                    if ((u < v) && !hasArc(u, v) && !hasArc(v, u))
                    {
                        result.push_back(Pair(u, v));
                    }
                }
            }
            return result;
        }

        std::vector<Pair> listOfAbsentArcs()
        {
            std::vector<Pair> result(countEdges());
            for (std::size_t ui = 0; ui < vertices.size(); ui++)
            {
                VertexID u = vertices[ui].id;

                for (std::size_t vi = 0; vi < vertices.size(); vi++)
                {
                    VertexID v = vertices[vi].id;

                    if ((u != v) && !hasArc(u, v))
                    {
                        result.push_back(Pair(u, v));
                    }
                }
            }
            return result;
        }

    public:

        class VertexIterator
        {
        private:
            friend class Graph;

            const Graph* graph;
            std::size_t index;
            std::size_t count;

            VertexIterator(const Graph* graph)
            {
                this->graph = graph;
                index = 0;
                count = graph->countVertices();
            }

        public:
            bool hasNext()
            {
                return index < count;
            }

            bool next(Vertex& v)
            {
                if (index < count)
                {
                    graph->getVertex(graph->getVertexID(index), v);
                    index++;
                    return true;
                }
                else
                    return false;
            }

            bool current(Vertex& v) const
            {
                return graph->getVertex(graph->getVertexID(index), v);
            }
        };

        class EdgeIterator
        {
        private:
            friend class Graph;
            friend class Walker;

            const EdgeInfo* ei;
            bool exitingEdges;

            EdgeIterator(const EdgeInfo* ei, bool exitingEdges)
            {
                this->ei = ei;
                this->exitingEdges = exitingEdges;
            }

        public:
            bool hasNext()
            {
                return (ei != nullptr);
            }

            bool next(Edge& e)
            {
                if (ei)
                {
                    e = *ei;
                    if (exitingEdges)
                        ei = ei->nextFromSource;
                    else
                        ei = ei->nextToDestination;
                    return true;
                }
                else
                    return false;
            }

            bool current(Edge& e) const
            {
                if (ei)
                {
                    e = *ei;
                    return true;
                }
                else
                    return false;
            }
        };

        class Walker
        {
        private:
            friend class Graph;

            const Graph* graph;
            VertexID position;

            Walker(const Graph* graph, VertexID position)
            {
                this->graph = graph;
                this->position = position;
            }

        public:
            bool teleport(VertexID id)
            {
                if (graph->validVertexID(id))
                {
                    position = id;
                    return true;
                }
                else
                    return false;
            }

            EdgeIterator exitingEdges() const
            {
                return graph->exitingEdgeIterator(position);
            }

            EdgeIterator enteringEdges() const
            {
                return graph->enteringEdgeIterator(position);
            }

            VertexID moveForwardAlong(const EdgeIterator& it)
            {
                Edge e;
                if (it.current(e))
                {
                    position = e.v;
                    return position;
                }
                else
                    return 0;
            }

            VertexID moveBackwardAlong(const EdgeIterator& it)
            {
                Edge e;
                if (it.current(e))
                {
                    position = e.u;
                    return position;
                }
                else
                    return 0;
            }

            VertexID moveForwardAlong(EdgeID id)
            {
                auto it = exitingEdges();
                Edge e;
                while (it.next(e))
                {
                    if (e.id == id)
                    {
                        position = e.v;
                        return position;
                    }
                }
                return 0;
            }

            VertexID moveBackwardAlong(EdgeID id)
            {
                auto it = enteringEdges();
                Edge e;
                while (it.next(e))
                {
                    if (e.id == id)
                    {
                        position = e.u;
                        return position;
                    }
                }
                return 0;
            }

            EdgeID moveForwardTo(VertexID id)
            {
                auto it = exitingEdges();
                Edge e;
                while (it.next(e))
                {
                    if (e.v == id)
                    {
                        position = id;
                        return e.id;
                    }
                }
                return 0;
            }

            EdgeID moveBackwardTo(VertexID id)
            {
                auto it = enteringEdges();
                Edge e;
                while (it.next(e))
                {
                    if (e.u == id)
                    {
                        position = id;
                        return e.id;
                    }
                }
                return 0;
            }
        };
    };

}
