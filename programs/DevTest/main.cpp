
/**
 * DevTest
 * This program is for unspecified testing during development.
 *
 * @author Kevin A. Naudé
 * @version 1.1
 */

#include <iostream>
#include <string>
#include <Matrix.hpp>
#include <Graph.hpp>
#include <OptimalMatching.hpp>
#include <GraphSimilarity.hpp>

using namespace kn;

void show(Matrix<float> m)
{
    std::size_t rows = m.countRows();
    std::size_t columns = m.countColumns();
    for (std::size_t row = 0; row < rows; row++)
    {
        for (std::size_t column = 0; column < columns; column++)
        {
            std::cout << " " << m.getValue(row, column);
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

int main(int argc, const char* argv[])
{
    ///*
    Graph a;
    a.addVertex(1);
    a.addVertex(2);
    a.addVertex(3);
    a.addEdge(0, 1, 0);
    a.addEdge(1, 2, 0);

    Graph b;
    b.addVertex(1);
    b.addVertex(2);
    b.addVertex(1);
    b.addEdge(0, 1, 0);
    b.addEdge(1, 2, 0);
    //b.addEdge(2, 0, 0);
    //*/

    /*
    Graph a;
    a.addVertex(0);
    a.addVertex(0);
    a.addVertex(0);
    a.addVertex(0);
    a.addVertex(0);
    a.addArc(0, 1, 0);
    a.addArc(0, 2, 0);
    a.addArc(1, 2, 0);
    a.addArc(2, 3, 0);
    a.addArc(3, 1, 0);
    a.addArc(1, 4, 0);
    a.addArc(2, 4, 0);

    Graph b;
    b.addVertex(0);
    b.addVertex(0);
    b.addVertex(0);
    b.addArc(0, 1, 0);
    b.addArc(1, 2, 0);
    */

    /*
    Graph a;
    a.addVertex(0);
    a.addVertex(0);
    a.addArc(0, 1, 0);

    Graph b;
    b.addVertex(0);
    b.addVertex(0);
    b.addArc(0, 1, 0);
    */

    /*
    Graph a;
    a.addVertex(0);
    a.addVertex(0);
    a.addVertex(0);
    a.addArc(0, 1, 0);
    a.addArc(1, 2, 0);

    Graph b;
    b.addVertex(0);
    b.addVertex(0);
    b.addVertex(0);
    b.addArc(0, 1, 0);
    b.addArc(1, 2, 0);
    */

    BlondelSimilarity blondel;
    Matching<float> mapping;
    blondel.solve(mapping, a, b, 0.000000001);

    std::cout << mapping.meanScore() << std::endl;

    Matrix<float> sim = blondel.fixedPoint();
    Matrix<float> M = blondel.M;

    show(M);
    show(sim);

    std::string t;
    std::cin >> t;
    
}
