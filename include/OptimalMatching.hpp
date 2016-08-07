
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
#include <assert.h>
#include <View.hpp>
#include <BitStructures.hpp>

namespace kn
{

    template <typename T>
    struct Matching
    {
        size_t u, v;
        T score;
    };

    template <typename T>
    class MatchingOptimiser
    {
    public:
        static constexpr T Zero = 0;
        static constexpr size_t Unused = ~0;

    private:
        size_t majorDim, minorDim;

        View<T>* arrayTs;
        T* arrayT;
        size_t* arraySizeT;
        Matching<T>* arrayMatching;

        size_t rows, columns;

        View<View<T> > costs;
        View<View<T> > matrix;

        View<size_t> chain;
        View<size_t> columnStars, rowStars, rowPrimes;

        IntegerSet rowsCovered, columnsCovered, rowPrimesTouched;

        size_t numColumnStars, numRowStars, numRowPrimes;
        size_t chainLen, numColumnsCovered;

        bool transposed, maximise;

        View<Matching<T> > mapping;

        void Prepare();
        void EngageNext(size_t pi, size_t pj);
        bool FindUncoveredZero(size_t& pi, size_t& pj);
        T FindSmallestUncovered();
        void DoNext();
        void ExtractMapping();

    public:
        MatchingOptimiser() : MatchingOptimiser(50) {}
        MatchingOptimiser(size_t estDim) : MatchingOptimiser(estDim, estDim) {}

        MatchingOptimiser(size_t estDim1, size_t estDim2);

        ~MatchingOptimiser();

        void Reallocate(size_t estDim1, size_t estDim2);

        const View<View<T> >& DefineProblem(size_t m, size_t n, bool maximise);
        void ClearCosts(T defaultCost = Zero);

        const View<Matching<T> >& Solve(T& sum);
    };

    template<typename T>
    MatchingOptimiser<T>::MatchingOptimiser(size_t estDim1, size_t estDim2)
    	: rowsCovered(std::min(estDim1, estDim2)), columnsCovered(std::max(estDim1, estDim2)), rowPrimesTouched(std::min(estDim1, estDim2))
    {
        minorDim = std::min(estDim1, estDim2);
        majorDim = std::max(estDim1, estDim2);

        arrayTs = new View<T>[majorDim * 2];
        arrayT = new T[majorDim * minorDim * 2];
        arraySizeT = new size_t[majorDim + minorDim * 4 + 1];
        arrayMatching = new Matching<T>[minorDim];
    }

    template<typename T>
    MatchingOptimiser<T>::~MatchingOptimiser()
    {
        delete[] arrayMatching;
        delete[] arraySizeT;
        delete[] arrayT;
        delete[] arrayTs;
    }

    template<typename T>
    void MatchingOptimiser<T>::Reallocate(size_t estDim1, size_t estDim2)
    {
        size_t newMinorDim = std::min(estDim1, estDim2);
        size_t newMajorDim = std::max(estDim1, estDim2);

        if ((minorDim != newMinorDim) || (majorDim != newMajorDim))
        {
            delete[] arrayMatching;
            delete[] arraySizeT;
            delete[] arrayT;
            delete[] arrayTs;

            minorDim = newMinorDim;
            majorDim = newMajorDim;

            arrayTs = new View<T>[majorDim];
            arrayT = new T[majorDim * minorDim * 2];
            arraySizeT = new size_t[majorDim + minorDim * 4 + 1];
            arrayMatching = new Matching<T>[minorDim];
        }
    }

    template<typename T>
    const View<View<T> >& MatchingOptimiser<T>::DefineProblem(size_t m, size_t n, bool maximise)
    {
        this->maximise = maximise;
        transposed = (m > n);
        rows = std::min(m, n);
        columns = std::max(m, n);

        if ((rows > minorDim) || (columns > majorDim)) Reallocate(rows, columns);

        costs = View<View<T> >(arrayTs, m);
        matrix = View<View<T> >(arrayTs + m, rows);
        chain = View<size_t>(arraySizeT, rows * 2 + 1);
        rowStars = View<size_t>(arraySizeT + rows * 2 + 1, rows);
        rowPrimes = View<size_t>(arraySizeT + rows * 3 + 1, rows);
        columnStars = View<size_t>(arraySizeT + rows * 4 + 1, columns);
        mapping = View<Matching<T> >(arrayMatching, rows);

        for (size_t mi = 0; mi < m; mi++)
        {
            costs[mi] = View<T>(arrayT + mi * n, n);
        }

        for (size_t ri = 0; ri < rows; ri++)
        {
            matrix[ri] = View<T>((arrayT + m * n) + ri * columns, columns);
        }

        return costs;
    }

    template<typename T>
    void MatchingOptimiser<T>::ClearCosts(T defaultCost)
    {
        for (size_t i = 0; i < costs.size(); i++)
        {
            View<T> row = costs[i];
            for (size_t j = 0; j < row.size(); j++)
            {
                row[j] = defaultCost;
            }
        }
    }

    template<typename T>
    void MatchingOptimiser<T>::Prepare()
    {
        if (transposed)
        {
            for (size_t u = 0; u < rows; u++)
            {
                for (size_t v = 0; v < columns; v++)
                {
                    matrix[u][v] = costs[v][u];
                }
            }
        }
        else
        {
            for (size_t u = 0; u < rows; u++)
            {
                for (size_t v = 0; v < columns; v++)
                {
                    matrix[u][v] = costs[u][v];
                }
            }
        }


        if (maximise)
        {
            double big = costs[0][0];
            for (size_t u = 0; u < rows; u++)
            {
                for (size_t v = 0; v < columns; v++)
                {
                    if (matrix[u][v] > big) big = matrix[u][v];
                }
            }
            for (size_t u = 0; u < rows; u++)
            {
                for (size_t v = 0; v < columns; v++)
                {
                    matrix[u][v] = big - matrix[u][v];
                }
            }
        }

        for (size_t t = 0; t < mapping.size(); t++)
        {
            mapping[t].u = Unused;
            mapping[t].v = Unused;
        }

        chainLen = 0;
        numColumnsCovered = 0;

        for (size_t t = 0; t < rows; t++)
        {
            rowStars[t] = Unused;
            rowPrimes[t] = Unused;
        }
        for (size_t t = 0; t < columns; t++)
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
    void MatchingOptimiser<T>::EngageNext(size_t pi, size_t pj)
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
                size_t qj = pj;
                size_t qi = columnStars[qj];

                // AddStarChain(qi);
                chain[chainLen++] = qi;

                //if (!rowPrimes.contains(qi))
                //    throw new InternalError("Unexpected error in Hungarian method: The current row *should* have a primed entry.");

                if (!rowPrimesTouched.contains(qi))
                {
                    // the next primed zero in the chain is ...
                    size_t ri = qi;
                    size_t rj = rowPrimes[ri];

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
        for (size_t c = 1; (c + 1) < chainLen; c += 2)
        {
            // RemoveStar(chain[c + 1], chain[c]);
            size_t col = chain[c];
            size_t row = chain[c + 1];

            if (columnStars[col] != Unused) { columnStars[col] = Unused; numColumnStars--; }
            if (rowStars[row] != Unused) { rowStars[row] = Unused; numRowStars--; }
        }

        // ResolveChainPrimes();
        for (size_t c = 0; (c + 1) < chainLen; c += 2)
        {
            // AddStar(chain[c], target);
            size_t row = chain[c];
            size_t col = chain[c + 1];

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
        for (size_t c = 0; c < rows; c++)
        {
            rowPrimes[c] = Unused;
        }

        // VerifyStars();
        for (size_t c = 0; c < columns; c++)
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
    bool MatchingOptimiser<T>::FindUncoveredZero(size_t& pi, size_t& pj)
    {
        for (size_t i = 0; i < rows; i++)
        {
            if (rowsCovered.contains(i)) continue;

            for (size_t j = 0; j < columns; j++)
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
    T MatchingOptimiser<T>::FindSmallestUncovered()
    {
        T smallest = matrix[0][0];

        for (size_t i = 0; i < rows; i++)
        {
            if (rowsCovered.contains(i)) continue;

            for (size_t j = 0; j < columns; j++)
            {
                if (columnsCovered.contains(j)) continue;

                T value = matrix[i][j];
                if (value < smallest) smallest = value;
            }
        }
        return smallest;
    }

    template<typename T>
    void MatchingOptimiser<T>::DoNext()
    {
        for (;;)
        {
            size_t pi, pj;
            if (FindUncoveredZero(pi, pj)) // populates pi and pj
            {
                // AddPrime(pi, pj);
                numRowPrimes++;
                rowPrimes[pi] = pj;

                if (rowStars[pi] != Unused)
                {
                    size_t j = rowStars[pi];
                    // UncoverColumn(j);
                    numColumnsCovered--;
                    columnsCovered.remove(j);
                    // CoverRow(pi);
                    rowsCovered.add(pi);
                }
                else
                {
                    // We've primed a zero in a starless row.
                    EngageNext(pi, pj);
                    return;
                }
            }
            else
            {
                // There were no uncovered zeros, so they must be created.
                T smallest = FindSmallestUncovered();
                for (size_t j = 0; j < columns; j++)
                {
                    if (columnsCovered.contains(j)) continue;

                    for (size_t i = 0; i < rows; i++)
                    {
                        matrix[i][j] -= smallest;
                    }
                }
                for (size_t i = 0; i < rows; i++)
                {
                    if (!rowsCovered.contains(i)) continue;

                    for (size_t j = 0; j < columns; j++)
                    {
                        matrix[i][j] += smallest;
                    }
                }
            }
        }
    }

    template<typename T>
    void MatchingOptimiser<T>::ExtractMapping()
    {
        for (size_t i = 0; i < rows; i++)
        {
            size_t j = rowStars[i];

            // RecordResult(i, j);
            if (transposed)
            {
                mapping[i].u = j;
                mapping[i].v = i;
                mapping[i].score = costs[j][i];
            }
            else
            {
                mapping[i].u = i;
                mapping[i].v = j;
                mapping[i].score = costs[i][j];
            }
        }
    }

    template<typename T>
    const View<Matching<T> >& MatchingOptimiser<T>::Solve(T& sum)
    {
        Prepare();
        while (numColumnsCovered < rows)
        {
            DoNext();
        }
        ExtractMapping();

        double localSum = 0.0;
        for (size_t i = 0; i < rows; i++)
        {
            localSum += mapping[i].score;
        }
        sum = localSum;
        return mapping;
    }

}
