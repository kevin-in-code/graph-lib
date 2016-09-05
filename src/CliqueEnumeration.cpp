
#include <cstdlib>
#include <CliqueEnumeration.hpp>
#include <BitStructures.hpp>

namespace kn
{

    /// The common context for many clique enumerators in the BronKerbosch family.

    class Context
    {
    private:
        IntegerSet* pool;
        IntegerSet* next;

    public:
        std::size_t numVertices;
        std::vector<IntegerSet> N;
        std::vector<IntegerSet> K;

        const Graph* graph;
        CliqueReceiver* receiver;

        Context(const Graph* graph, CliqueReceiver* receiver)
        {
            this->graph = graph;
            this->receiver = receiver;

            numVertices = graph->countVertices();
            for (std::size_t ui = 0; ui < numVertices; ui++)
            {
                Graph::VertexID u = graph->getVertexID(ui);
                IntegerSet neighbours(numVertices);
                for (std::size_t vi = 0; vi < numVertices; vi++)
                {
                    if (ui == vi) continue;
                    Graph::VertexID v = graph->getVertexID(vi);
                    if (graph->hasEdge(u, v))
                    {
                        neighbours.add(v);
                    }
                }

                IntegerSet conflicts(neighbours);
                conflicts.invert();

                N.push_back(neighbours);
                K.push_back(conflicts);
            }

            this->pool = new IntegerSet[4 * (1+numVertices)];
            for (std::size_t i = 0; i < 4 * (1+numVertices); i++)
            {
                this->pool[i].setMaxCardinality(numVertices);
            }
            this->next = &this->pool[0];
        }

        ~Context()
        {
            delete[] this->pool;
        }

        IntegerSet* reserveSet()
        {
            return next++;
        }

        void releaseSet()
        {
            next--;
        }

        /*
        void releaseSet(IntegerSet* set)
        {
            next--;
            if (next != set)
                throw new std::exception();
        }
        */

        IntegerSet* intersect(IntegerSet* a, IntegerSet* b)
        {
            IntegerSet* r = reserveSet();
            r->intersection(*a, *b);
            return r;
        }

        IntegerSet* insert(IntegerSet* a, std::size_t value)
        {
            IntegerSet* r = reserveSet();
            r->copy(*a);
            r->add(value);
            return r;
        }
    };


    /// The base class for a BronKerbosch clique enumerator.
    class BKSearch : public Context
    {
    public:
        BKSearch(const Graph* graph, CliqueReceiver* receiver) :
            Context(graph, receiver) {}
            
        void apply(IntegerSet* S, IntegerSet* P, IntegerSet* X)
        {
            receiver->onApply();
            IntegerSet* Q = pivotConflict(S, P, X);
            if (Q)
            {
#ifndef NDEBUG
                bool grouped = (Q->countLimit(2) > 1);
                if (grouped) receiver->onOpenGroup();
                bool first = true;
                Graph::Vertex vertex;
#endif

                auto it = Q->iterator();
                while (it.hasNext())
                {
                    std::size_t v = it.next();
                    P->remove(v);
#ifndef NDEBUG
                    if (!first) receiver->onPartition();
                    first = false;
                    graph->getVertexByIndex(v, vertex);
                    receiver->onVertex(v, vertex.attrID);
#endif

                    IntegerSet* s2 = this->insert(S, v);
                    IntegerSet* p2 = this->intersect(P, &N[v]);
                    IntegerSet* x2 = this->intersect(X, &N[v]);

                    apply(s2, p2, x2);

                    X->add(v);
                }

#ifndef NDEBUG
                if (first) receiver->onCutOff();
                if (grouped) receiver->onCloseGroup();
#endif
                this->releaseSet(); // Release Q
            }
            else
            if (X->isEmpty())
            {
                /// maximal clique found
                receiver->onClique(*graph, *S);
#ifndef NDEBUG
                receiver->onOk();
#endif
            }
            else
            {
                /// cut-off: sub-maximal clique
#ifndef NDEBUG
                receiver->onCutOff();
#endif
            }

            this->releaseSet(); // Release X
            this->releaseSet(); // Release P
            this->releaseSet(); // Release S
        }

        virtual IntegerSet* pivotConflict(IntegerSet* S, IntegerSet* P, IntegerSet* X) = 0;
    };


    class BKSearch_Tomita : public BKSearch
    {
    public:
        BKSearch_Tomita(const Graph* graph, CliqueReceiver* receiver) :
            BKSearch(graph, receiver) {}

        virtual IntegerSet* pivotConflict(IntegerSet* S, IntegerSet* P, IntegerSet* X)
        {
            if (!P->isEmpty())
            {
                std::size_t most = 0;
                std::size_t q = 0;

                auto it = X->iterator();
                while (it.hasNext())
                {
                    std::size_t v = it.next();
                    std::size_t count = P->countCommon(N[v]) + 1;
                    if (count > most)
                    {
                        most = count;
                        q = v;
                    }
                }

                auto it2 = P->iterator();
                while (it2.hasNext())
                {
                    std::size_t v = it2.next();
                    std::size_t count = P->countCommon(N[v]) + 1;
                    if (count > most)
                    {
                        most = count;
                        q = v;
                    }
                }

                IntegerSet* Q = this->intersect(P, &K[q]);

                return Q;
            }
            else
            {
                return nullptr;
            }
        }
    };

    class BKSearch_Naude : public BKSearch
    {
    public:
        BKSearch_Naude(const Graph* graph, CliqueReceiver* receiver) :
            BKSearch(graph, receiver) {}

        virtual IntegerSet* pivotConflict(IntegerSet* S, IntegerSet* P, IntegerSet* X)
        {
        search:
            std::size_t q = numVertices; // an initial value which is not a valid vertex
            std::size_t least = numVertices + 1; // not infinity, but large enough
#ifndef NDEBUG
            Graph::Vertex vertex;
#endif

            if (!X->isEmpty())
            {
                auto it = X->iterator();
                while (it.hasNext())
                {
                    std::size_t v = it.next();
                    std::size_t w = 0;
                    std::size_t count = P->countCommonLimit(K[v], least, w);
                    if (count < least)
                    {
                        if (count <= 2)
                        {
                            if (count != 1) // count in { 0, 2 }
                            {
                                q = v;
                                goto conclude;
                            }
                            else
                            {
                                /// Process w in place
                                S->add(w);
                                P->intersectWith(N[w]);
                                X->intersectWith(N[w]);

#ifndef NDEBUG
                                graph->getVertexByIndex(w, vertex);
                                receiver->onVertex(w, vertex.attrID);
#endif

                                /// Very important!
                                /// We are iterating through X, and we have potentially just modified X.
                                /// It is essential that the iterator be written to allow concurrent updates.
                                /// If your iterator does not allow concurrent updates, then just "goto search".

                                if (K[w].contains(q)) goto search;
                            }
                        }
                        else
                        {
                            q = v;
                            least = count;
                        }
                    }
                }
            }

            if (!P->isEmpty())
            {
                auto it = P->iterator();
                while (it.hasNext())
                {
                    std::size_t v = it.next();
                    std::size_t w = 0;
                    std::size_t count = P->countCommonLimit(K[v], least, w);
                    if (count < least)
                    {
                        if (count <= 2)
                        {
                            if (count != 1) // count in { 0, 2 }
                            {
                                q = v;
                                goto conclude;
                            }
                            else
                            {
                                /// Process v in place
                                S->add(v);
                                P->intersectWith(N[v]);
                                X->intersectWith(N[v]);

#ifndef NDEBUG
                                graph->getVertexByIndex(v, vertex);
                                receiver->onVertex(v, vertex.attrID);
#endif

                                /// Very important!
                                /// We are iterating through P, and we have potentially just modified P.
                                /// It is essential that the iterator be written to allow concurrent updates.
                                /// If your iterator does not allow concurrent updates, then just "goto search".

                                if (K[v].contains(q)) goto search;
                            }
                        }
                        else
                        {
                            q = v;
                            least = count;
                        }
                    }
                }
            }

        conclude:
            if (q < numVertices)
            {
                IntegerSet* Q = this->intersect(P, &K[q]);
                return Q;
            }
            else
                return nullptr;
        }
    };

    class BKSearch_Segundo : public BKSearch
    {
    public:
        BKSearch_Segundo(const Graph* graph, CliqueReceiver* receiver) :
            BKSearch(graph, receiver) {}

        virtual IntegerSet* pivotConflict(IntegerSet* S, IntegerSet* P, IntegerSet* X)
        {
            if (!X->isEmpty())
            {
                std::size_t q = X->firstElement();
                IntegerSet* Q = this->intersect(P, &K[q]);

                return Q;
            }
            else
            if (!P->isEmpty())
            {
                std::size_t q = P->firstElement();
                IntegerSet* Q = this->intersect(P, &K[q]);

                return Q;
            }
            else
            {
                return nullptr;
            }
        }
    };


    Graph* maxDegreeLast(const Graph* graph)
    {
        Graph::Vertex vertex;
        std::size_t n = graph->countVertices();
        std::vector<Graph::VertexID> permutation(n);
        IntegerSet avail(n);
        avail.fill();

        for (std::size_t index = permutation.size(); index > 0; index--)
        {
            std::size_t i = index - 1;
            std::size_t highestDegree = 0;
            std::size_t indexOfHighest = n;

            for (auto it = avail.iterator(); it.hasNext(); )
            {
                std::size_t v = it.next();
                graph->getVertexByIndex(v, vertex);
                if ((indexOfHighest == n) || (vertex.outDegree >= highestDegree))
                {
                    highestDegree = vertex.outDegree;
                    indexOfHighest = v;
                }
            }
            avail.remove(indexOfHighest);
            permutation[(n - 1) - i] = indexOfHighest;
        }
        return new Graph(*graph, permutation, true);
    }


    void AllCliques_Tomita(const Graph* graph, CliqueReceiver* receiver)
    {
        BKSearch_Tomita alg(graph, receiver);

        IntegerSet* S = alg.reserveSet();
        IntegerSet* P = alg.reserveSet();
        IntegerSet* X = alg.reserveSet();

        S->clear();
        P->fill();
        X->clear();

        receiver->onClear();
        alg.apply(S, P, X);
        receiver->onComplete();
    }

    void AllCliques_Naude(const Graph* graph, CliqueReceiver* receiver)
    {
        BKSearch_Naude alg(graph, receiver);

        IntegerSet* S = alg.reserveSet();
        IntegerSet* P = alg.reserveSet();
        IntegerSet* X = alg.reserveSet();

        S->clear();
        P->fill();
        X->clear();

        receiver->onClear();
        alg.apply(S, P, X);
        receiver->onComplete();
    }

    void AllCliques_Segundo(const Graph* graph, CliqueReceiver* receiver)
    {
        std::unique_ptr<Graph> permutedGraph(maxDegreeLast(graph));
        BKSearch_Segundo alg(permutedGraph.get(), receiver);

        //BKSearch_Segundo alg(graph, receiver);

        IntegerSet* S = alg.reserveSet();
        IntegerSet* P = alg.reserveSet();
        IntegerSet* X = alg.reserveSet();

        S->clear();
        P->fill();
        X->clear();

        receiver->onClear();
        alg.apply(S, P, X);
        receiver->onComplete();
    }

}
