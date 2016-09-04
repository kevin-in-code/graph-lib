
#pragma once

/**
* OptimalMatching.hpp
* Purpose: Find the optimal matching over MxN bipartite pairings.
*
* This class uses Munkres' adaptation of the paper-based Hungarian algorithm
* to find the optimal matching over an MxN matrix of costs.
*
* @author Kevin A. Naudé
* @version 1.1
*/

#include <algorithm>
#include <memory>
#include <vector>
#include <cassert>
#include <ArrayView.hpp>
#include <BitStructures.hpp>
#include <Matrix.hpp>

namespace kn
{

    template <typename T>
    class Matching
    {
    public:
        struct Pair
        {
            std::size_t u, v;
            T score;

            Pair() {}

            Pair(std::size_t u, std::size_t v, T score)
            {
                this->u = u;
                this->v = v;
                this->score = score;
            }
        };

    private:
        static constexpr T Zero = 0;

        std::vector<Pair> pairs;
        std::vector<std::size_t> uToV, vToU;
        IntegerSet unmappedU, unmappedV;
        std::size_t divisor;
        T sum;

        struct IncreasingScore
        {
            inline bool operator() (const Pair& x, const Pair& y)
            {
                return (x.score < y.score);
            }
        };

        struct DecreasingScore
        {
            inline bool operator() (const Pair& x, const Pair& y)
            {
                return (y.score < x.score);
            }
        };

    public:
        Matching()
        {
            clear(1, 1);
        }

        void clear(std::size_t rows, std::size_t columns)
        {
            divisor = std::max(rows, columns);
            pairs.clear();
            uToV.clear();
            vToU.clear();
            uToV.reserve(rows);
            vToU.reserve(columns);
            for (std::size_t k = 0; k < rows; k++)
            {
                uToV.push_back(0);
            }
            for (std::size_t k = 0; k < columns; k++)
            {
                vToU.push_back(0);
            }
            unmappedU.setMaxCardinality(rows);
            unmappedV.setMaxCardinality(columns);
            unmappedU.fill();
            unmappedV.fill();
            sum = Zero;
        }

        std::size_t countPairs() const
        {
            return pairs.size();
        }

        const Pair& getPair(std::size_t index) const
        {
            return pairs[index];
        }

        void sortIncreasingScore()
        {
            std::sort(pairs.begin(), pairs.end(), IncreasingScore());
        }

        void sortDecreasingScore()
        {
            std::sort(pairs.begin(), pairs.end(), DecreasingScore());
        }

        void add(std::size_t u, std::size_t v, T score)
        {
			typename Matching<T>::Pair pair(u, v, score);
			add(pair);
        }

        void add(const Pair& pair)
		{
			std::size_t u = pair.u;
			std::size_t v = pair.v;
			T score = pair.score;
			if (unmappedU.contains(u) && unmappedV.contains(v))
			{
				unmappedU.remove(u);
				unmappedV.remove(v);
				uToV[u] = v;
				vToU[v] = u;
				sum += score;
				pairs.push_back(pair);
			}
		}

        std::size_t getU(std::size_t v) const
        {
            return vToU[v];
        }

        std::size_t getV(std::size_t u) const
        {
            return uToV[u];
        }

        bool isMappedU(std::size_t u) const
        {
            return !unmappedU.contains(u);
        }

        bool isMappedV(std::size_t v) const
        {
            return !unmappedV.contains(v);
        }

        IntegerSet::Iterator getUnmappedU() const
        {
            return unmappedU.iterator();
        }

        IntegerSet::Iterator getUnmappedV() const
        {
            return unmappedV.iterator();
        }

        T sumScore() const
        {
            return sum;
        }

        T meanScore() const
        {
            return sum / divisor;
        }
    };

    template <typename T>
    class AssignmentSolver
    {
    protected:
        virtual void solve(Matching<T>& matching, const Matrix<T>& costs, bool maximise) = 0;

    public:
        void match(Matching<T>& matching, const Matrix<T>& costs, bool maximise)
        {
            solve(matching, costs, maximise);
        }

        void minimise(Matching<T>& matching, const Matrix<T>& costs)
        {
            solve(matching, costs, false);
        }

        void maximise(Matching<T>& matching, const Matrix<T>& profits)
        {
            solve(matching, profits, true);
        }
    };

    template <typename T>
    class MunkresAssignment : public AssignmentSolver<T>
    {
    public:
        static constexpr T Zero = 0;
        static constexpr std::size_t Unused = ~0;

    private:
        std::size_t majorDim, minorDim;

        ArrayView<T>* arrayTs;
        T* arrayT;
        std::size_t* arraySizeT;
        typename Matching<T>::Pair* arrayMatching;

        std::size_t rows, columns;

        ArrayView<ArrayView<T> > matrix;

        ArrayView<std::size_t> chain;
        ArrayView<std::size_t> columnStars, rowStars, rowPrimes;

        IntegerSet rowsCovered, columnsCovered, rowPrimesTouched;

        std::size_t numColumnStars, numRowStars, numRowPrimes;
        std::size_t chainLen, numColumnsCovered;

        bool transposed;

        ArrayView<typename Matching<T>::Pair> matching;

        void prepare(const Matrix<T>& costs, bool maximise);
        void engageNext(std::size_t pi, std::size_t pj);
        bool findUncoveredZero(std::size_t& pi, std::size_t& pj);
        T findSmallestUncovered();
        void doNext();
        void extractMapping(const Matrix<T>& costs);
        void defineProblem(std::size_t m, std::size_t n);
        void reallocate(std::size_t estDim1, std::size_t estDim2);

    public:
        MunkresAssignment() : MunkresAssignment(10) {}
        MunkresAssignment(std::size_t estDim) : MunkresAssignment(estDim, estDim) {}

        MunkresAssignment(std::size_t estDim1, std::size_t estDim2);

        ~MunkresAssignment();

    protected:
        void solve(Matching<T>& mapping, const Matrix<T>& costs, bool maximise);
    };

    template<typename T>
    MunkresAssignment<T>::MunkresAssignment(std::size_t estDim1, std::size_t estDim2)
    	: rowsCovered(std::min(estDim1, estDim2)), columnsCovered(std::max(estDim1, estDim2)), rowPrimesTouched(std::min(estDim1, estDim2))
    {
        minorDim = std::min(estDim1, estDim2);
        majorDim = std::max(estDim1, estDim2);

        arrayTs = new ArrayView<T>[majorDim];
        arrayT = new T[majorDim * minorDim];
        arraySizeT = new std::size_t[majorDim + minorDim * 4 + 1];
        arrayMatching = new typename Matching<T>::Pair[minorDim];
    }

    template<typename T>
    MunkresAssignment<T>::~MunkresAssignment()
    {
        delete[] arrayMatching;
        delete[] arraySizeT;
        delete[] arrayT;
        delete[] arrayTs;
    }

    template<typename T>
    void MunkresAssignment<T>::reallocate(std::size_t estDim1, std::size_t estDim2)
    {
        std::size_t newMinorDim = std::min(estDim1, estDim2);
        std::size_t newMajorDim = std::max(estDim1, estDim2);

        if ((minorDim != newMinorDim) || (majorDim != newMajorDim))
        {
            delete[] arrayMatching;
            delete[] arraySizeT;
            delete[] arrayT;
            delete[] arrayTs;

            minorDim = newMinorDim;
            majorDim = newMajorDim;

            arrayTs = new ArrayView<T>[majorDim];
            arrayT = new T[majorDim * minorDim];
            arraySizeT = new std::size_t[majorDim + minorDim * 4 + 1];
            arrayMatching = new typename Matching<T>::Pair[minorDim];
        }
    }

    template<typename T>
    void MunkresAssignment<T>::defineProblem(std::size_t m, std::size_t n)
    {
        transposed = (m > n);
        rows = std::min(m, n);
        columns = std::max(m, n);

        if ((rows > minorDim) || (columns > majorDim)) reallocate(rows, columns);

        matrix = ArrayView<ArrayView<T> >(arrayTs, rows);
        chain = ArrayView<std::size_t>(arraySizeT, rows * 2 + 1);
        rowStars = ArrayView<std::size_t>(arraySizeT + rows * 2 + 1, rows);
        rowPrimes = ArrayView<std::size_t>(arraySizeT + rows * 3 + 1, rows);
        columnStars = ArrayView<std::size_t>(arraySizeT + rows * 4 + 1, columns);
        matching = ArrayView<typename Matching<T>::Pair>(arrayMatching, rows);

		rowPrimesTouched.setMaxCardinality(rows);
		rowsCovered.setMaxCardinality(rows);
		columnsCovered.setMaxCardinality(columns);

        for (std::size_t ri = 0; ri < rows; ri++)
        {
            matrix[ri] = ArrayView<T>(arrayT + ri * columns, columns);
        }
    }

    template<typename T>
    void MunkresAssignment<T>::prepare(const Matrix<T>& costs, bool maximise)
    {
        if (transposed)
        {
            for (std::size_t u = 0; u < rows; u++)
            {
                for (std::size_t v = 0; v < columns; v++)
                {
                    matrix[u][v] = costs.getValue(v, u);
                }
            }
        }
        else
        {
            for (std::size_t u = 0; u < rows; u++)
            {
                for (std::size_t v = 0; v < columns; v++)
                {
                    matrix[u][v] = costs.getValue(u, v);
                }
            }
        }


        if (maximise)
        {
            T big = matrix[0][0];
            for (std::size_t u = 0; u < rows; u++)
            {
                for (std::size_t v = 0; v < columns; v++)
                {
                    if (matrix[u][v] > big) big = matrix[u][v];
                }
            }
            for (std::size_t u = 0; u < rows; u++)
            {
                for (std::size_t v = 0; v < columns; v++)
                {
                    matrix[u][v] = big - matrix[u][v];
                }
            }
        }

        for (std::size_t t = 0; t < matching.size(); t++)
        {
            matching[t].u = Unused;
            matching[t].v = Unused;
        }

        chainLen = 0;
        numColumnsCovered = 0;

        for (std::size_t t = 0; t < rows; t++)
        {
            rowStars[t] = Unused;
            rowPrimes[t] = Unused;
        }
        for (std::size_t t = 0; t < columns; t++)
        {
            columnStars[t] = Unused;
        }

        numColumnStars = 0;
        numRowStars = 0;
        numRowPrimes = 0;

        rowsCovered.clear();
        columnsCovered.clear();
        rowPrimesTouched.clear();
    }

    template<typename T>
    void MunkresAssignment<T>::engageNext(std::size_t pi, std::size_t pj)
    {
        // InitChain(pi, pj);
        chain[0] = pi;
        chain[1] = pj;
        chainLen = 2;

        // TouchPrimedRow(pi);
        rowPrimesTouched.add(pi);

        for (;;)
        {
            if (columnStars[pj] != Unused)
            {
                // the starred zero being displaced is ...
                std::size_t qj = pj;
                std::size_t qi = columnStars[qj];

                // AddStarChain(qi);
                chain[chainLen++] = qi;

                //if (!rowPrimes.contains(qi))
                //    throw new InternalError("Unexpected error in Hungarian method: The current row *should* have a primed entry.");

                if (!rowPrimesTouched.contains(qi))
                {
                    // the next primed zero in the chain is ...
                    std::size_t ri = qi;
                    std::size_t rj = rowPrimes[ri];

                    // AddPrimeChain(rj);
                    chain[chainLen++] = rj;

                    // rowPrimesTouched.set(ri, True);
                    rowPrimesTouched.add(ri);

                    pi = ri;
                    pj = rj;
                }
                else
                    break; // break: must break for now, due to local cycle.
            }
            else
                break; // break: there are no more starred zeros in the primed zero's column.
        }

        // RemoveChainStars();
        for (std::size_t c = 1; (c + 1) < chainLen; c += 2)
        {
            // RemoveStar(chain[c + 1], chain[c]);
            std::size_t col = chain[c];
            std::size_t row = chain[c + 1];

            if (columnStars[col] != Unused) { columnStars[col] = Unused; numColumnStars--; }
            if (rowStars[row] != Unused) { rowStars[row] = Unused; numRowStars--; }
        }

        // ResolveChainPrimes();
        for (std::size_t c = 0; (c + 1) < chainLen; c += 2)
        {
            // AddStar(chain[c], target);
            std::size_t row = chain[c];
            std::size_t col = chain[c + 1];

            if (columnStars[col] == Unused) numColumnStars++;
            columnStars[col] = row;

            if (rowStars[row] == Unused) numRowStars++;
            rowStars[row] = col;

            // CoverColumn(col);
            numColumnsCovered++;
            columnsCovered.add(col);
        }

        rowsCovered.clear();
        rowPrimesTouched.clear();
        numRowPrimes = 0;
        for (std::size_t c = 0; c < rows; c++)
        {
            rowPrimes[c] = Unused;
        }

        // VerifyStars();
        for (std::size_t c = 0; c < columns; c++)
        {
            if ((columnStars[c] != Unused) && !columnsCovered.contains(c))
            {
                // CoverColumn(c);
                numColumnsCovered++;
                columnsCovered.add(c);
            }
        }
    }

    template<typename T>
    bool MunkresAssignment<T>::findUncoveredZero(std::size_t& pi, std::size_t& pj)
    {
        for (std::size_t i = 0; i < rows; i++)
        {
            if (rowsCovered.contains(i)) continue;

            for (std::size_t j = 0; j < columns; j++)
            {
                if (columnsCovered.contains(j)) continue;

                // Exact test for 0 is ok due to subtraction performed in Hungarian method
                if (matrix[i][j] == Zero)
                {
                    pi = i;
                    pj = j;
                    return true;
                }
            }
        }
        return false;
    }

    template<typename T>
    T MunkresAssignment<T>::findSmallestUncovered()
    {
        T smallest = matrix[0][0];
        bool empty = true;

        for (std::size_t i = 0; i < rows; i++)
        {
            if (rowsCovered.contains(i)) continue;

            for (std::size_t j = 0; j < columns; j++)
            {
                if (columnsCovered.contains(j)) continue;

                T value = matrix[i][j];
                if (empty || (value < smallest))
                {
                    smallest = value;
                    empty = false;
                }
            }
        }
        return smallest;
    }

    template<typename T>
    void MunkresAssignment<T>::doNext()
    {
        for (;;)
        {
            std::size_t pi, pj;
            if (findUncoveredZero(pi, pj)) // populates pi and pj
            {
                // AddPrime(pi, pj);
                numRowPrimes++;
                rowPrimes[pi] = pj;

                if (rowStars[pi] != Unused)
                {
                    std::size_t j = rowStars[pi];
                    // UncoverColumn(j);
                    numColumnsCovered--;
                    columnsCovered.remove(j);
                    // CoverRow(pi);
                    rowsCovered.add(pi);
                }
                else
                {
                    // We've primed a zero in a starless row.
                    engageNext(pi, pj);
                    return;
                }
            }
            else
            {
                // There were no uncovered zeros, so they must be created.
                T smallest = findSmallestUncovered();
                for (std::size_t j = 0; j < columns; j++)
                {
                    if (columnsCovered.contains(j)) continue;

                    for (std::size_t i = 0; i < rows; i++)
                    {
                        matrix[i][j] -= smallest;
                    }
                }
                for (std::size_t i = 0; i < rows; i++)
                {
                    if (!rowsCovered.contains(i)) continue;

                    for (std::size_t j = 0; j < columns; j++)
                    {
                        matrix[i][j] += smallest;
                    }
                }
            }
        }
    }

    template<typename T>
    void MunkresAssignment<T>::extractMapping(const Matrix<T>& costs)
    {
        for (std::size_t i = 0; i < rows; i++)
        {
            std::size_t j = rowStars[i];

            // RecordResult(i, j);
            if (transposed)
            {
                matching[i].u = j;
                matching[i].v = i;
                matching[i].score = costs.getValue(j, i);
            }
            else
            {
                matching[i].u = i;
                matching[i].v = j;
                matching[i].score = costs.getValue(i, j);
            }
        }
    }

    template<typename T>
    void MunkresAssignment<T>::solve(Matching<T>& mapping, const Matrix<T>& costs, bool maximise)
    {
        defineProblem(costs.countRows(), costs.countColumns());
        prepare(costs, maximise);
        while (numColumnsCovered < rows)
        {
            doNext();
        }
        extractMapping(costs);
        mapping.clear(costs.countRows(), costs.countColumns());
        for (std::size_t index = 0; index < matching.size(); index++)
        {
            mapping.add(matching[index]);
        }
    }

    template <typename T>
    class GreedyAssignment : public AssignmentSolver<T>
    {
    private:
        IntegerSet covered;

    protected:
        void solve(Matching<T>& matching, const Matrix<T>& costs, bool maximise);
    };

    template<typename T>
    void GreedyAssignment<T>::solve(Matching<T>& matching, const Matrix<T>& costs, bool maximise)
    {
        std::size_t rows = costs.countRows();
        std::size_t columns = costs.countColumns();

		matching.clear(rows, columns);
		if (rows <= columns)
        {
            std::size_t minorDim = rows;
            std::size_t majorDim = columns;

            covered.setMaxCardinality(majorDim);
            covered.clear();

            for (std::size_t u = 0; u < minorDim; u++)
            {
                std::size_t bestV = ~0;
                T best;
                if (!covered.contains(u))
                {
                    bestV = u;
                    best = costs[u][u];
                }
                for (std::size_t v = 0; v < majorDim; v++)
                {
                    if (covered.contains(v)) continue;
                    T cur = costs[u][v];

                    if (~bestV == 0)
                    {
                        bestV = v;
                        best = cur;
                    }
                    else
                        if (maximise)
                        {
                            if (cur > best)
                            {
                                bestV = v;
                                best = cur;
                            }
                        }
                        else
                        {
                            if (cur < best)
                            {
                                bestV = v;
                                best = cur;
                            }
                        }
                }
                covered.add(bestV);
                matching.add(u, bestV, best);
            }
        }
        else
        {
            std::size_t minorDim = columns;
            std::size_t majorDim = rows;

            covered.setMaxCardinality(majorDim);
            covered.clear();

            for (std::size_t u = 0; u < minorDim; u++)
            {
                std::size_t bestV = ~0;
                T best;
                if (!covered.contains(u))
                {
                    bestV = u;
                    best = costs[u][u];
                }
                for (std::size_t v = 0; v < majorDim; v++)
                {
                    if (covered.contains(v)) continue;
                    T cur = costs[v][u];

                    if (~bestV == 0)
                    {
                        bestV = v;
                        best = cur;
                    }
                    else
                        if (maximise)
                        {
                            if (cur > best)
                            {
                                bestV = v;
                                best = cur;
                            }
                        }
                        else
                        {
                            if (cur < best)
                            {
                                bestV = v;
                                best = cur;
                            }
                        }
                }
                covered.add(bestV);
                matching.add(bestV, u, best);
            }
        }
    }

}
