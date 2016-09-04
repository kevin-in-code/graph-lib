
#pragma once

/**
* GraphSimilarity.hpp
* Purpose: Quantify the similarity between graphs.
*
* @author Kevin A. Naudé
* @version 1.1
*/

#include <memory>
#include <Graph.hpp>
#include <Matrix.hpp>
#include <AssignmentSolver.hpp>

namespace kn
{

    class FixedPointSimilarity
    {
    private:
        std::unique_ptr<AssignmentSolver<float>> assignmentSolver;
        Matrix<float> sim[2];
        int index, concludedIndex;

    protected:
        virtual void doInit(Matrix<float>& newSim, const Graph& a, const Graph& b);

        virtual void doStep(Matrix<float>& newSim, const Matrix<float>& sim) = 0;

        virtual bool doPostprocess(const Graph& a, const Graph& b, Matrix<float>& newSim, const Matrix<float>& sim);

    public:
        FixedPointSimilarity()
            : assignmentSolver(new MunkresAssignment<float>())
        {
        }

        FixedPointSimilarity(std::unique_ptr<AssignmentSolver<float>> solver)
        {
            assignmentSolver = std::move(solver);
        }

        void solve(Matching<float>& mapping, const Graph& a, const Graph& b, double threshold);

        void setAssignmentSolver(std::unique_ptr<AssignmentSolver<float>> solver)
        {
            assignmentSolver = std::move(solver);
        }

        const Matrix<float>& fixedPoint()
        {
            return sim[concludedIndex];
        }

        const Matrix<float>& finalSim()
        {
            return sim[index];
        }
    };

    class BlondelSimilarity : public FixedPointSimilarity
    {
    public:
        Matrix<float> temp;
        Matrix<float> M;
        float bias;
        bool odd;

    protected:
        virtual void doInit(Matrix<float>& newSim, const Graph& a, const Graph& b);

        virtual void doStep(Matrix<float>& newSim, const Matrix<float>& sim);

    public:
        BlondelSimilarity(float bias = 0.0f, bool odd = false)
        {
            this->bias = bias;
            this->odd = odd;
        }

        BlondelSimilarity(std::unique_ptr<AssignmentSolver<float>> solver, float bias = 0.0f, bool odd = false)
            : FixedPointSimilarity(std::move(solver))
        {
            this->bias = bias;
            this->odd = odd;
        }
    };

}
