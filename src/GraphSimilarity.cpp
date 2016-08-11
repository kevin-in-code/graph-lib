
#include <GraphSimilarity.hpp>

namespace kn
{

    void FixedPointSimilarity::doInit(Matrix<float>& newSim, const Graph& a, const Graph& b)
    {
        std::size_t rows = a.countVertices();
        std::size_t columns = b.countVertices();
        newSim.reshape(rows, columns);
        for (std::size_t row = 0; row < rows; row++)
        {
            for (std::size_t column = 0; column < columns; column++)
            {
                newSim.setValue(row, column, 1.0f);
            }
        }
    }

    bool FixedPointSimilarity::doPostprocess(const Graph& a, const Graph& b, Matrix<float>& newSim, const Matrix<float>& sim)
    {
        std::size_t rows = a.countVertices();
        std::size_t columns = b.countVertices();
        const AttributeModel* am = a.getVertexAttributeModel();
        std::size_t n = std::min(rows, columns);
        float scale = 0.25f / n;
        for (std::size_t row = 0; row < rows; row++)
        {
            for (std::size_t column = 0; column < columns; column++)
            {
                Graph::Vertex va, vb;
                a.getVertexByIndex(row, va);
                b.getVertexByIndex(column, vb);
                float value = sim.getValue(row, column);
                if ((am == nullptr) && (va.attrID == vb.attrID))
                {
                    newSim.setValue(row, column, (1.0f + value) / 3.0f);
                }
                else
                if ((am != nullptr) && (am->compatible(va.attrID, vb.attrID)))
                {
                    newSim.setValue(row, column, (1.0f + value) / 3.0f);
                }
                else
                {
                    newSim.setValue(row, column, scale * value);
                }
            }
        }
        return true;
    }

    void FixedPointSimilarity::solve(Mapping<float>& mapping, const Graph& a, const Graph& b, double threshold)
    {
        index = 0;
        concludedIndex = 0;
        std::size_t rows = a.countVertices();
        std::size_t columns = b.countVertices();
        sim[0].reshape(rows, columns);
        sim[1].reshape(rows, columns);
        doInit(sim[0], a, b);

        do {
            doStep(sim[1 - index], sim[index]);
            index = 1 - index;
        } while (sim[index].exceedsThresholdDifference(sim[1 - index], threshold));

        concludedIndex = index;
        if (doPostprocess(a, b, sim[1 - index], sim[index])) index = 1 - index;
        mo.solve(mapping, sim[index], true);
    }

    void BlondelSimilarity::doInit(Matrix<float>& newSim, const Graph& a, const Graph& b)
    {
        Matrix<float> A, B, M2, s;

        std::size_t rows = a.countVertices();
        std::size_t columns = b.countVertices();
        temp.reshape(rows, columns);

        a.constructAdjacencyMatrix(A);
        b.constructAdjacencyMatrix(B);

        M.multiplyKronecker(A, B);

        A.transpose();
        B.transpose();
        M2.multiplyKronecker(A, B);
        M.add(M2);

        if (odd)
        {
            FixedPointSimilarity::doInit(s, a, b);
            newSim.multiplyAsColumn(M, s);
        }
        else
        {
            FixedPointSimilarity::doInit(newSim, a, b);
        }
    }

    void BlondelSimilarity::doStep(Matrix<float>& newSim, const Matrix<float>& sim)
    {
        temp.multiplyAsColumn(M, sim);
        temp.add(bias);
        temp.normalise(2.0);

        newSim.multiplyAsColumn(M, temp);
        newSim.add(bias);
        newSim.normalise(2.0);
    }

}
