
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
#include <OptimalMatching.hpp>

using namespace kn;

int main(int argc, const char* argv[])
{
    Matrix<double> m(5, 5);
    m[0][0] = 1.0; m[0][1] = 2.0; m[0][2] = 1.0; m[0][3] = 9.0; m[0][4] = 1.0;
    m[1][0] = 2.0; m[1][1] = 3.0; m[1][2] = 1.0; m[1][3] = 1.0; m[1][4] = 5.0;
    m[2][0] = 1.0; m[2][1] = 5.0; m[2][2] = 0.0; m[2][3] = 2.0; m[2][4] = 2.0;
    m[3][0] = 3.0; m[3][1] = 1.0; m[3][2] = 0.0; m[3][3] = 1.0; m[3][4] = 2.0;
    m[4][0] = 1.0; m[4][1] = 0.0; m[4][2] = 4.0; m[4][3] = 3.0; m[4][4] = 1.0;
    MatchingOptimiser<double> mo(5, 5);
    double sum = 0.0;
    Matching<double> matching(mo.solve(m, true));
    std::cout << matching.sumGood << std::endl;

    Matrix<double> a(6, 6);
    a[0][0] = 2.0; a[0][1] = 0.0; a[0][2] = 0.0; a[0][3] = 0.0; a[0][4] = 0.0; a[0][5] = 0.0;
    a[1][0] = 0.0; a[1][1] = 1.0; a[1][2] = 0.0; a[1][3] = 0.0; a[1][4] = 0.0; a[1][5] = 0.0;
    a[2][0] = 0.0; a[2][1] = 0.0; a[2][2] = 3.0; a[2][3] = 0.0; a[2][4] = 0.0; a[2][5] = 0.0;
    a[3][0] = 0.0; a[3][1] = 0.0; a[3][2] = 0.0; a[3][3] = 1.0; a[3][4] = 0.0; a[3][5] = 0.0;
    a[4][0] = 0.0; a[4][1] = 0.0; a[4][2] = 0.0; a[4][3] = 0.0; a[4][4] = 2.0; a[4][5] = 1.0;
    a[5][0] = 0.0; a[5][1] = 0.0; a[5][2] = 0.0; a[5][3] = 0.0; a[5][4] = 0.0; a[5][5] = 9.0;
    Matrix<double> b(2, 3);
    b[0][0] = 1.0; b[0][1] = 1.0; b[0][2] = 1.0;
    b[1][0] = 1.0; b[1][1] = 1.0; b[1][2] = 2.0;
    Matrix<double> c;
    c.multiplyAsColumn(a, b);
    for (std::size_t row = 0; row < c.countRows(); row++)
    {
        for (std::size_t column = 0; column < c.countColumns(); column++)
        {
            std::cout << " " << c.getValue(row, column);
        }
        std::cout << std::endl;
    }
}
