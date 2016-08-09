
#pragma once

/**
* GraphSimilarity.hpp
* Purpose: Quantify the similarity between graphs.
*
* @author Kevin A. Naudé
* @version 1.1
*/

#include <Graph.hpp>
#include <Matrix.hpp>
#include <OptimalMatching.hpp>

namespace kn
{

    class FixedPointSimilarity
    {
    private:
        MatchingOptimiser<float> mo;
        Matrix<float> sim[2];
        int index;

    protected:
        virtual void doInit(Matrix<float>& newSim, const Graph& a, const Graph& b)
        {
            std::size_t rows = a.countVertices();
            std::size_t columns = b.countVertices();
            for (std::size_t row = 0; row < rows; row++)
            {
                for (std::size_t column = 0; column < rows; column++)
                {
                    newSim.setValue(row, column, 1.0f);
                }
            }
        }

        virtual void doStep(Matrix<float>& newSim, const Matrix<float>& sim) = 0;

        bool step(double threshold)
        {
            doStep(sim[1 - index], sim[index]);
            index = 1 - index;
            return sim[index].exceedsThresholdDifference(sim[1 - index], threshold);
        }

    public:
        void solve(const Graph& a, const Graph& b, double threshold, bool checkAttributes)
        {
            index = 0;
            std::size_t rows = a.countVertices();
            std::size_t columns = b.countVertices();
            sim[0].reshape(rows, columns);
            sim[1].reshape(rows, columns);
            doInit(sim[0], a, b);

            while (step(threshold));

            sim[1 - index] = sim[index];

            Matching<float> matching;

            if (checkAttributes)
            {
                for (std::size_t row = 0; row < rows; row++)
                {
                    for (std::size_t column = 0; column < rows; column++)
                    {
                        Graph::Vertex va, vb;
                        a.getVertexByIndex(row, va);
                        b.getVertexByIndex(column, vb);
                        if (va.attrID != vb.attrID)
                        {
                            sim[1].setValue(row, column, -100000.0);
                        }
                    }
                }

                matching = mo.solve(sim[1], true);
            }
            else
            {
                matching = mo.solve(sim[0], true);
            }
        }

        const Matrix<float>& currentSim()
        {
            return sim[0];
        }

        const ArrayView<MatchingPair<float> >& vertexMatching()
        {
            return mo.solve(sim[index], true);
        }
    };
    
    class VertexAssignment
    {
    public:
        std::vector<MatchingPair<float> > primary;
        std::vector<MatchingPair<float> > secondary;

        VertexAssignment(const Matrix<float>& sim, bool computeSecondary)
        {
            std::size_t m = sim.countRows();
            std::size_t n = sim.countColumns();
            MatchingOptimiser<float> mo(m, n);
            ArrayView<ArrayView<float> > grid = mo.defineProblem(m, n, true);

            for (std::size_t row = 0; row < m; row++)
            {
                for (std::size_t column = 0; column < n; column++)
                {
                    grid[row][column] = sim.getValue(row, column);
                }
            }
        }
    };

}
