
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
		size_t numVertices;
		std::vector<IntegerSet> N;
		std::vector<IntegerSet> K;

		const Graph* graph;
		CliqueReceiver* receiver;

		Context(const Graph* graph, CliqueReceiver* receiver)
		{
			this->graph = graph;
			this->receiver = receiver;

			numVertices = graph->countVertices();
			for (size_t ui = 0; ui < numVertices; ui++)
			{
				Graph::VertexID u = graph->getVertexID(ui);
				IntegerSet neighbours(numVertices);
				for (size_t vi = 0; vi < numVertices; vi++)
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
			for (size_t i = 0; i < 4 * (1+numVertices); i++)
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

		IntegerSet* insert(IntegerSet* a, size_t value)
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
			IntegerSet* Q = pivotConflict(S, P, X);
			if (Q)
			{
				auto it = Q->iterator();
				while (it.hasNext())
				{
					size_t v = it.next();
					P->remove(v);

					IntegerSet* s2 = this->insert(S, v);
					IntegerSet* p2 = this->intersect(P, &N[v]);
					IntegerSet* x2 = this->intersect(X, &N[v]);

					apply(s2, p2, x2);

					X->add(v);
				}

				this->releaseSet(); // Release Q
			}
			else
			if (X->isEmpty())
			{
				/// maximal clique found
				receiver->onClique(*graph, *S);
			}
			else
			{
				/// cut-off: sub-maximal clique
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
				size_t maximum = P->count() + 1;
				size_t most = 0;
				size_t q = 0;

				auto it = X->iterator();
				while (it.hasNext())
				{
					size_t v = it.next();
					size_t count = P->countCommon(N[v]) + 1;
					if (count > most)
					{
						most = count;
						q = v;
					}
				}

				auto it2 = P->iterator();
				while (it2.hasNext())
				{
					size_t v = it2.next();
					size_t count = P->countCommon(N[v]) + 1;
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
			size_t q = numVertices; // an initial value which is not a valid vertex
			size_t least = numVertices + 1; // not infinity, but large enough

			if (!X->isEmpty())
			{
				auto it = X->iterator();
				while (it.hasNext())
				{
					size_t v = it.next();
					size_t w = 0;
					size_t count = P->countCommonLimit(K[v], least, w);
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

								/// Very important!
								/// We are iterating through P, and we have potentially just modified P.
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
					size_t v = it.next();
					size_t w = 0;
					size_t count = P->countCommonLimit(K[v], least, w);
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

}
