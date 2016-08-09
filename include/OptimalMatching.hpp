
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
#include <ArrayView.hpp>
#include <BitStructures.hpp>
#include <Matrix.hpp>

namespace kn
{

    template <typename T>
    struct Matching
    {
        std::size_t u, v;
        T score;
    };

    template <typename T>
    class MatchingOptimiser
    {
    public:
        static constexpr T Zero = 0;
        static constexpr std::size_t Unused = ~0;

    private:
        std::size_t majorDim, minorDim;

        ArrayView<T>* arrayTs;
        T* arrayT;
        std::size_t* arraySizeT;
        Matching<T>* arrayMatching;

        std::size_t rows, columns;

        ArrayView<ArrayView<T> > matrix;

        ArrayView<std::size_t> chain;
        ArrayView<std::size_t> columnStars, rowStars, rowPrimes;

        IntegerSet rowsCovered, columnsCovered, rowPrimesTouched;

        std::size_t numColumnStars, numRowStars, numRowPrimes;
        std::size_t chainLen, numColumnsCovered;

        bool transposed, maximise;

        ArrayView<Matching<T> > mapping;

        void prepare(const Matrix<T>& costs);
        void engageNext(std::size_t pi, std::size_t pj);
        bool findUncoveredZero(std::size_t& pi, std::size_t& pj);
        T findSmallestUncovered();
        void doNext();
        void extractMapping(const Matrix<T>& costs);
        void defineProblem(std::size_t m, std::size_t n, bool maximise);


    public:
        MatchingOptimiser() : MatchingOptimiser(50) {}
        MatchingOptimiser(std::size_t estDim) : MatchingOptimiser(estDim, estDim) {}

        MatchingOptimiser(std::size_t estDim1, std::size_t estDim2);

        ~MatchingOptimiser();

        void reallocate(std::size_t estDim1, std::size_t estDim2);

        const ArrayView<Matching<T> >& solve(const Matrix<T>& costs, bool maximise);

        static T sumMatching(ArrayView<Matching<T> >& matching, bool excludeNegatives);
    };

    template<typename T>
    MatchingOptimiser<T>::MatchingOptimiser(std::size_t estDim1, std::size_t estDim2)
    	: rowsCovered(std::min(estDim1, estDim2)), columnsCovered(std::max(estDim1, estDim2)), rowPrimesTouched(std::min(estDim1, estDim2))
    {
        minorDim = std::min(estDim1, estDim2);
        majorDim = std::max(estDim1, estDim2);

        arrayTs = new ArrayView<T>[majorDim];
        arrayT = new T[majorDim * minorDim];
        arraySizeT = new std::size_t[majorDim + minorDim * 4 + 1];
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
    void MatchingOptimiser<T>::reallocate(std::size_t estDim1, std::size_t estDim2)
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
            arrayMatching = new Matching<T>[minorDim];
        }
    }

    template<typename T>
    void MatchingOptimiser<T>::defineProblem(std::size_t m, std::size_t n, bool maximise)
    {
        this->maximise = maximise;
        transposed = (m > n);
        rows = std::min(m, n);
        columns = std::max(m, n);

        if ((rows > minorDim) || (columns > majorDim)) reallocate(rows, columns);

        matrix = ArrayView<ArrayView<T> >(arrayTs, rows);
        chain = ArrayView<std::size_t>(arraySizeT, rows * 2 + 1);
        rowStars = ArrayView<std::size_t>(arraySizeT + rows * 2 + 1, rows);
        rowPrimes = ArrayView<std::size_t>(arraySizeT + rows * 3 + 1, rows);
        columnStars = ArrayView<std::size_t>(arraySizeT + rows * 4 + 1, columns);
        mapping = ArrayView<Matching<T> >(arrayMatching, rows);

        for (std::size_t ri = 0; ri < rows; ri++)
        {
            matrix[ri] = ArrayView<T>(arrayT + ri * columns, columns);
        }
    }

    template<typename T>
    void MatchingOptimiser<T>::prepare(const Matrix<T>& costs)
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
            for (std::size_t u = 0; u < rows-1; u++)
            {
                for (std::size_t v = 0; v < columns; v++)
                {
                    matrix[u][v] = costs.getValue(u, v);
                }
            }
            for (std::size_t v = 0; v < columns; v++)
            {
                matrix[4][v] = costs.getValue(4, v);
            }
        }


        if (maximise)
        {
            double big = matrix[0][0];
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

        for (std::size_t t = 0; t < mapping.size(); t++)
        {
            mapping[t].u = Unused;
            mapping[t].v = Unused;
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
    void MatchingOptimiser<T>::engageNext(std::size_t pi, std::size_t pj)
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
    bool MatchingOptimiser<T>::findUncoveredZero(std::size_t& pi, std::size_t& pj)
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
    T MatchingOptimiser<T>::findSmallestUncovered()
    {
        T smallest = matrix[0][0];

        for (std::size_t i = 0; i < rows; i++)
        {
            if (rowsCovered.contains(i)) continue;

            for (std::size_t j = 0; j < columns; j++)
            {
                if (columnsCovered.contains(j)) continue;

                T value = matrix[i][j];
                if (value < smallest) smallest = value;
            }
        }
        return smallest;
    }

    template<typename T>
    void MatchingOptimiser<T>::doNext()
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
    void MatchingOptimiser<T>::extractMapping(const Matrix<T>& costs)
    {
        for (std::size_t i = 0; i < rows; i++)
        {
            std::size_t j = rowStars[i];

            // RecordResult(i, j);
            if (transposed)
            {
                mapping[i].u = j;
                mapping[i].v = i;
                mapping[i].score = costs.getValue(j, i);
            }
            else
            {
                mapping[i].u = i;
                mapping[i].v = j;
                mapping[i].score = costs.getValue(i, j);
            }
        }
    }

    template<typename T>
    const ArrayView<Matching<T> >& MatchingOptimiser<T>::solve(const Matrix<T>& costs, bool maximise)
    {
        defineProblem(costs.countRows(), costs.countColumns(), maximise);
        prepare(costs);
        while (numColumnsCovered < rows)
        {
            doNext();
        }
        extractMapping(costs);
        return mapping;
    }

    template<typename T>
    T MatchingOptimiser<T>::sumMatching(ArrayView<Matching<T> >& matching, bool excludeNegatives)
    {
        double localSum = 0.0;
        if (excludeNegatives)
        {
            for (std::size_t i = 0; i < matching.size(); i++)
            {
                if (matching[i].score >= 0.0)
                {
                    localSum += matching[i].score;
                }
            }
        }
        else
        {
            for (std::size_t i = 0; i < matching.size(); i++)
            {
                localSum += matching[i].score;
            }
        }
        return localSum;
    }


}
