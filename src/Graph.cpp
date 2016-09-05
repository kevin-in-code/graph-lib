
#include <Graph.hpp>

namespace kn
{

    Graph::Graph()
    {
        vertexAttributes = nullptr;
        edgeAttributes = nullptr;
        nextVertexID = 0;
        nextEdgeID = 0;
    }

    Graph::Graph(const AttributeModel* vertexAttributeModel, const AttributeModel* edgeAttributeModel)
    {
        vertexAttributes = vertexAttributeModel;
        edgeAttributes = edgeAttributeModel;
        nextVertexID = 0;
        nextEdgeID = 0;
    }

    Graph::Graph(const Graph& other, bool complement)
    {
        nextVertexID = 0;
        nextEdgeID = 0;

        std::unordered_map<VertexID, VertexID> map;
        Vertex oV, oU;
        for (auto it = other.vertexIterator(); it.next(oV); )
        {
            VertexID v = this->addVertex(oV.attrID);
            map.insert(std::make_pair(oV.id, v));
        }

        for (auto it = other.vertexIterator(); it.next(oV); )
        {
            VertexID v = map[oV.id];

            for (auto it2 = other.vertexIterator(); it2.next(oU); )
            {
                VertexID u = map[oU.id];

                Edge e, e2;
                if (!complement)
                {
                    if (other.getEdge(u, v, e) && (!e.undirected || (u < v)))
                    {
                        if (e.undirected)
                            this->addEdge(u, v, e.attrID);
                        else
                            this->addArc(u, v, e.attrID);
                    }
                }
                else
                {
                    if (u < v)
                    {
                        if (other.getEdge(u, v, e))
                        {
                            if (!e.undirected && !other.getEdge(v, u, e2))
                            {
                                this->addArc(v, u, e.attrID);
                            }
                        }
                        else
                        if (other.getEdge(v, u, e2) && !e2.undirected)
                        {
                            this->addArc(u, v, e2.attrID);
                        }
                        else
                        {
                            this->addEdge(u, v, 0);
                        }
                    }
                }
            }
        }
    }

    Graph::Graph(const Graph& other, const std::vector<VertexID>& permutation)
    {
        nextVertexID = 0;
        nextEdgeID = 0;

        std::unordered_map<VertexID, VertexID> map;
        Vertex oV, oU;
        for (std::size_t index = 0; index < permutation.size(); index++)
        {
            other.getVertexByIndex(permutation[index], oV);
            VertexID v = this->addVertex(oV.attrID);
            map.insert(std::make_pair(oV.id, v));
        }

        for (auto it = other.vertexIterator(); it.next(oV); )
        {
            VertexID v = map[oV.id];

            for (auto it2 = other.vertexIterator(); it2.next(oU); )
            {
                VertexID u = map[oU.id];

                Edge e;
                if (other.getEdge(oU.id, oV.id, e) && (!e.undirected || (u < v)))
                {
                    if (e.undirected)
                        this->addEdge(u, v, e.attrID);
                    else
                        this->addArc(u, v, e.attrID);
                }
            }
        }
    }

    Graph::~Graph()
    {
        for (std::size_t index = vertices.size(); index > 0; index--)
        {
            VertexID id = vertices[index - 1].id;
            removeVertex(id);
        }
    }

    /// Move constructor
    Graph::Graph(Graph&& other) :
        vertexAttributes(nullptr),
        edgeAttributes(nullptr),
        vertices(),
        vertexIDtoIndex(),
        edgeIDtoSourceID(),
        nextVertexID(1),
        nextEdgeID(1)
    {
        vertices.swap(other.vertices);
        std::swap(vertexAttributes, other.vertexAttributes);
        std::swap(edgeAttributes, other.edgeAttributes);
        vertexIDtoIndex.swap(other.vertexIDtoIndex);
        edgeIDtoSourceID.swap(other.edgeIDtoSourceID);
        nextVertexID = other.nextVertexID;
        nextEdgeID = other.nextEdgeID;
        other.nextVertexID = 0;
        other.nextEdgeID = 0;
    }

    /// Move assignment
    Graph& Graph::operator=(Graph&& other)
    {
        if (this != &other)
        {
            vertices.clear();
            vertexAttributes = nullptr;
            edgeAttributes = nullptr;
            vertexIDtoIndex.clear();
            edgeIDtoSourceID.clear();

            vertices.swap(other.vertices);
            std::swap(vertexAttributes, other.vertexAttributes);
            std::swap(edgeAttributes, other.edgeAttributes);
            vertexIDtoIndex.swap(other.vertexIDtoIndex);
            edgeIDtoSourceID.swap(other.edgeIDtoSourceID);

            nextVertexID = other.nextVertexID;
            nextEdgeID = other.nextEdgeID;
            other.nextVertexID = 0;
            other.nextEdgeID = 0;
        }

        return *this;
    }

    /// Copy assignment
    Graph& Graph::operator=(const Graph& other)
    {
        if (this != &other)
        {
            vertexAttributes = nullptr;
            edgeAttributes = nullptr;

            vertices.clear();
            vertexIDtoIndex.clear();
            edgeIDtoSourceID.clear();

            nextVertexID = 0;
            nextEdgeID = 0;

            std::unordered_map<VertexID, VertexID> map;
            Vertex oV, oU;
            for (auto it = other.vertexIterator(); it.next(oV); )
            {
                VertexID v = this->addVertex(oV.attrID);
                map.insert(std::make_pair(oV.id, v));
            }

            for (auto it = other.vertexIterator(); it.next(oV); )
            {
                VertexID v = map[oV.id];

                for (auto it2 = other.vertexIterator(); it2.next(oU); )
                {
                    VertexID u = map[oU.id];

                    Edge e;
                    if (other.getEdge(u, v, e) && (!e.undirected || (u < v)))
                    {
                        if (e.undirected)
                            this->addEdge(u, v, e.attrID);
                        else
                            this->addArc(u, v, e.attrID);
                    }
                }
            }
        }

        return *this;
    }


    void Graph::deleteEdge(EdgeInfo* e)
    {
        EdgeInfo* nextToDestination = e->nextToDestination;
        EdgeInfo* prevToDestination = e->prevToDestination;
        EdgeInfo* nextFromSource = e->nextFromSource;
        EdgeInfo* prevFromSource = e->prevFromSource;

        std::size_t fromIndex = vertexIDtoIndex[e->u];
        std::size_t toIndex = vertexIDtoIndex[e->v];
        vertices[fromIndex].outDegree--;
        vertices[toIndex].inDegree--;
        if (vertices[fromIndex].sourceEdges == e)
        {
            vertices[fromIndex].sourceEdges = nextFromSource ? nextFromSource : prevFromSource;
        }
        if (vertices[toIndex].destinationEdges == e)
        {
            vertices[toIndex].destinationEdges = nextToDestination ? nextToDestination : prevToDestination;
        }
        if (nextToDestination)
        {
            nextToDestination->prevToDestination = prevToDestination;
        }
        if (prevToDestination)
        {
            prevToDestination->nextToDestination = nextToDestination;
        }
        if (nextFromSource)
        {
            nextFromSource->prevFromSource = prevFromSource;
        }
        if (prevFromSource)
        {
            prevFromSource->nextFromSource = nextFromSource;
        }
        edgeIDtoSourceID.erase(e->id);

        delete e;
    }

    void Graph::insertEdge(EdgeID id, VertexID sourceID, VertexID destinationID, AttrID attrID, bool undirected)
    {
        std::size_t fromIndex = vertexIDtoIndex[sourceID];
        std::size_t toIndex = vertexIDtoIndex[destinationID];

        VertexInfo* u = &vertices[fromIndex];
        VertexInfo* v = &vertices[toIndex];

        EdgeInfo* nextToDestination = v->destinationEdges;
        EdgeInfo* prevToDestination = nextToDestination ? nextToDestination->prevToDestination : nullptr;
        EdgeInfo* nextFromSource = u->sourceEdges;
        EdgeInfo* prevFromSource = nextFromSource ? nextFromSource->prevFromSource : nullptr;

        EdgeInfo* e = new EdgeInfo;
        assert(e);
        e->id = id;
        e->undirected = undirected;
        e->u = sourceID;
        e->v = destinationID;
        e->attrID = attrID;
        e->nextToDestination = nextToDestination;
        e->prevToDestination = prevToDestination;
        e->nextFromSource = nextFromSource;
        e->prevFromSource = prevFromSource;

        if (nextToDestination)
        {
            nextToDestination->prevToDestination = e;
        }
        if (prevToDestination)
        {
            prevToDestination->nextToDestination = e;
        }
        if (nextFromSource)
        {
            nextFromSource->prevFromSource = e;
        }
        if (prevFromSource)
        {
            prevFromSource->nextFromSource = e;
        }

        u->sourceEdges = e;
        v->destinationEdges = e;

        u->outDegree++;
        v->inDegree++;
    }

    void Graph::vertexAdjacency(VertexID id, IntegerSet& row)
    {
        row.clear();
        for (std::size_t index = 0; index < vertices.size(); index++)
        {
            if (hasArc(id, vertices[index].id)) row.add(index);
        }
    }

    std::unique_ptr<std::vector<IntegerSet>> Graph::adjacency()
    {
        std::unique_ptr<std::vector<IntegerSet>> matrix(new std::vector<IntegerSet>(vertices.size()));
        for (std::size_t index = 0; index < vertices.size(); index++)
        {
            vertexAdjacency(vertices[index].id, (*matrix)[index]);
        }
        return matrix;
    }

    bool Graph::getEdge(EdgeID id, Edge& e) const
    {
        auto it = edgeIDtoSourceID.find(id);
        if (it == edgeIDtoSourceID.end())
        {
            // These lines prevents a spurious compiler warning.
            e.id = e.u = e.v = e.attrID = 0;
            e.undirected = true;
            return false;
        }

        std::size_t sourceID = it->second;
        const VertexInfo* u = &vertices[vertexIDtoIndex.at(sourceID)];

        for (const EdgeInfo* ei = u->sourceEdges; ei; ei = ei->nextFromSource)
        {
            if (ei->id == id)
            {
                e = *ei;
                return true;
            }
        }
        return false;
    }

    bool Graph::getEdge(VertexID sourceID, VertexID destinationID, Edge& e) const
    {
        if (validVertexID(sourceID) && validVertexID(destinationID))
        {
            const VertexInfo* u = &vertices[vertexIDtoIndex.at(sourceID)];
            const VertexInfo* v = &vertices[vertexIDtoIndex.at(destinationID)];

            if (u->outDegree <= v->inDegree)
            {
                for (EdgeInfo* ei = u->sourceEdges; ei; ei = ei->nextFromSource)
                {
                    if (ei->v == destinationID)
                    {
                        e = *ei;
                        return true;
                    }
                }
            }
            else
            {
                for (EdgeInfo* ei = v->destinationEdges; ei; ei = ei->nextToDestination)
                {
                    if (ei->u == sourceID)
                    {
                        e = *ei;
                        return true;
                    }
                }
            }
        }
        return false;
    }

    bool Graph::removeVertex(VertexID id)
    {
        auto it = vertexIDtoIndex.find(id);
        if (it == vertexIDtoIndex.end()) return false;

        std::size_t index = it->second;

        /// 1: remove all associated edges
        VertexInfo* v = &vertices[index];
        while (v->destinationEdges)
        {
            deleteEdge(v->destinationEdges);
        }
        while (v->sourceEdges)
        {
            deleteEdge(v->sourceEdges);
        }

        /// 2: update vertex ID to index mapping for indices that change
        for (std::size_t other = index + 1; other < vertices.size(); other++)
        {
            VertexID id = vertices[other].id;
            vertexIDtoIndex[id] = other - 1;
        }

        /// 3: remove the vertex
        vertices.erase(vertices.begin() + index);
        vertexIDtoIndex.erase(id);
        return true;
    }

    void Graph::removeEdgeHelper(VertexID sourceID, VertexID destinationID)
    {
        const VertexInfo* u = &vertices[vertexIDtoIndex.at(sourceID)];
        const VertexInfo* v = &vertices[vertexIDtoIndex.at(destinationID)];

        if (u->outDegree <= v->inDegree)
        {
            for (EdgeInfo* e = u->sourceEdges; e; e = e->nextFromSource)
            {
                if (e->v == destinationID)
                {
                    deleteEdge(e);
                    return;
                }
            }
        }
        else
        {
            for (EdgeInfo* e = v->destinationEdges; e; e = e->nextToDestination)
            {
                if (e->u == sourceID)
                {
                    deleteEdge(e);
                    return;
                }
            }
        }
    }

    bool Graph::removeEdge(VertexID sourceID, VertexID destinationID)
    {
        if (validVertexID(sourceID) && validVertexID(destinationID))
        {
            const VertexInfo* u = &vertices[vertexIDtoIndex.at(sourceID)];
            const VertexInfo* v = &vertices[vertexIDtoIndex.at(destinationID)];

            if (u->outDegree <= v->inDegree)
            {
                for (EdgeInfo* e = u->sourceEdges; e; e = e->nextFromSource)
                {
                    if (e->v == destinationID)
                    {
                        bool undirected = e->undirected;
                        deleteEdge(e);
                        if (undirected) removeEdgeHelper(destinationID, sourceID);
                        return true;
                    }
                }
            }
            else
            {
                for (EdgeInfo* e = v->destinationEdges; e; e = e->nextToDestination)
                {
                    if (e->u == sourceID)
                    {
                        bool undirected = e->undirected;
                        deleteEdge(e);
                        if (undirected) removeEdgeHelper(destinationID, sourceID);
                        return true;
                    }
                }
            }
        }
        return false;
    }

    bool Graph::hasArc(VertexID sourceID, VertexID destinationID) const
    {
        if (validVertexID(sourceID) && validVertexID(destinationID))
        {
            const VertexInfo* u = &vertices[vertexIDtoIndex.at(sourceID)];
            const VertexInfo* v = &vertices[vertexIDtoIndex.at(destinationID)];

            if (u->outDegree <= v->inDegree)
            {
                for (const EdgeInfo* e = u->sourceEdges; e; e = e->nextFromSource)
                {
                    if (e->v == destinationID) return true;
                }
            }
            else
            {
                for (const EdgeInfo* e = v->destinationEdges; e; e = e->nextToDestination)
                {
                    if (e->u == sourceID) return true;
                }
            }
        }
        return false;
    }

    bool Graph::hasEdge(VertexID sourceID, VertexID destinationID) const
    {
        if (validVertexID(sourceID) && validVertexID(destinationID))
        {
            const VertexInfo* u = &vertices[vertexIDtoIndex.at(sourceID)];
            const VertexInfo* v = &vertices[vertexIDtoIndex.at(destinationID)];

            if (u->outDegree <= v->inDegree)
            {
                for (const EdgeInfo* e = u->sourceEdges; e; e = e->nextFromSource)
                {
                    if ((e->v == destinationID) && e->undirected) return true;
                }
            }
            else
            {
                for (const EdgeInfo* e = v->destinationEdges; e; e = e->nextToDestination)
                {
                    if ((e->u == sourceID) && e->undirected) return true;
                }
            }
        }
        return false;
    }

}
