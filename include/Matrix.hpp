
#pragma once

/**
* Matrix.hpp
* Purpose: Provides for common (and some less common) matrix operations.
*
* @author Kevin A. Naudé
* @version 1.1
*/

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <cassert>
#include <ArrayView.hpp>

namespace kn
{

    template <typename T>
    class Matrix
    {
    private:
        static constexpr T Zero = 0;

        T* values;
        std::size_t rowShift, columnShift;
        std::size_t rows, columns;
        std::size_t size;

    public:
        Matrix() : Matrix(10, 10) {}

        Matrix(const Matrix<T>& m) : Matrix(m.rows, m.columns)
        {
            for (std::size_t row = 0; row < m.rows; row++)
            {
                for (std::size_t column = 0; column < m.columns; column++)
                {
                    setValue(row, column, m.getValue(row, column));
                }
            }
        }

        Matrix(Matrix<T>&& m)
        {
            this->values = m.values;
            this->rowShift = m.rowShift;
            this->columnShift = m.columnShift;
            this->rows = m.rows;
            this->columns = m.columns;
            this->size = m.size;
            m.values = nullptr;
        }

        Matrix(std::size_t rows, std::size_t columns)
        {
            std::size_t sizeRC, sizeCR;
            std::size_t shiftRC = 0, shiftCR = 0;

            while (((std::size_t)1 << shiftRC) < columns) shiftRC++;
            while (((std::size_t)1 << shiftCR) < rows) shiftCR++;

            sizeRC = rows * ((std::size_t)1 << shiftRC);
            sizeCR = ((std::size_t)1 << shiftCR) * columns;

            if (sizeRC <= sizeCR)
            {
                this->size = sizeRC;
                this->rowShift = shiftRC;
                this->columnShift = 0;
            }
            else
            {
                this->size = sizeCR;
                this->rowShift = 0;
                this->columnShift = shiftCR;
            }

            this->rows = rows;
            this->columns = columns;

            this->values = new T[this->size];
        }

        ~Matrix()
        {
            delete[] this->values;
        }

        std::size_t countRows() const
        {
            return rows;
        }

        std::size_t countColumns() const
        {
            return columns;
        }

        void reshape(std::size_t rows, std::size_t columns)
        {
            bool reallocate = false;

            std::size_t sizeRC, sizeCR;
            std::size_t shiftRC = 0, shiftCR = 0;

            while (((std::size_t)1 << shiftRC) < columns) shiftRC++;
            while (((std::size_t)1 << shiftCR) < rows) shiftCR++;

            sizeRC = rows * ((std::size_t)1 << shiftRC);
            sizeCR = ((std::size_t)1 << shiftCR) * columns;

            if (sizeRC <= sizeCR)
            {
                if (this->size < sizeRC)
                {
                    this->size = sizeRC;
                    reallocate = true;
                }
                this->rowShift = shiftRC;
                this->columnShift = 0;
            }
            else
            {
                if (this->size < sizeCR)
                {
                    this->size = sizeCR;
                    reallocate = true;
                }
                this->rowShift = 0;
                this->columnShift = shiftCR;
            }

            this->rows = rows;
            this->columns = columns;

            if (reallocate)
            {
                delete[] this->values;
                this->values = new T[this->size];
            }
        }


        Matrix<T>& operator=(const Matrix<T>& m)
        {
            if (this != &m)
            {
                delete[] values;

                this->values = new double[m.size];
                this->rowShift = m.rowShift;
                this->columnShift = m.columnShift;
                this->rows = m.rows;
                this->columns = m.columns;
                this->size = m.size;

                std::copy(m.values, m.values + m.size, this->values);

            }
            return *this;
        }

        Matrix<T>& operator=(Matrix<T>&& m)
        {
            if (this != &m)
            {
                delete[] values;

                this->values = m.values;
                this->rowShift = m.rowShift;
                this->columnShift = m.columnShift;
                this->rows = m.rows;
                this->columns = m.columns;
                this->size = m.size;

                m.values = nullptr;
                m.rowShift = 0;
                m.columnShift = 0;
                m.rows = 0;
                m.columns = 0;
                m.size = 0;
            }
            return *this;
        }


        void transpose()
        {
            std::swap(rows, columns);
            std::swap(rowShift, columnShift);
        }


        T getValue(std::size_t row, std::size_t column) const
        {
            assert(row < rows);
            assert(column < columns);
            return values[(row << rowShift) + (column << columnShift)];
        }

        void setValue(std::size_t row, std::size_t column, T value)
        {
            assert(row < rows);
            assert(column < columns);
            values[(row << rowShift) + (column << columnShift)] = value;
        }


        void columnsFromVector(const Matrix<T>& m, std::size_t rows)
        {
            assert((m.rows % rows) == 0);
            std::size_t cols = m.rows / rows;
            reshape(rows, cols);
            std::size_t index = 0;
            for (std::size_t column = 0; column < columns; column++)
            {
                for (std::size_t row = 0; row < rows; row++)
                {
                    setValue(row, column, getValue(index++, 0));
                }
            }
        }

        void vectorFromColumns(const Matrix<T>& m)
        {
            reshape(m.rows * m.columns, 1);
            std::size_t index = 0;
            for (std::size_t column = 0; column < m.columns; column++)
            {
                for (std::size_t row = 0; row < m.rows; row++)
                {
                    setValue(index++, 0, getValue(row, column));
                }
            }
        }


        const ArrayView<T> getRowConst(std::size_t index) const
        {
            assert(index < rows);
            return ArrayView<T>(values + (index << rowShift), columns, columnShift);
        }

        ArrayView<T> getRowMutable(std::size_t index)
        {
            assert(index < rows);
            return ArrayView<T>(values + (index << rowShift), columns, columnShift);
        }


        const ArrayView<T> getColumnConst(std::size_t index) const
        {
            assert(index < columns);
            return ArrayView<T>(values + (index << columnShift), rows, rowShift);
        }

        ArrayView<T> getColumnMutable(std::size_t index)
        {
            assert(index < columns);
            return ArrayView<T>(values + (index << columnShift), rows, rowShift);
        }


        const ArrayView<T> operator[](std::size_t index) const
        {
            return getRowConst(index);
        }

        ArrayView<T> operator[](std::size_t index)
        {
            return getRowMutable(index);
        }


        void normalise(double p)
        {
            double n = norm(p);
            divide((T)n);
        }

        double norm(double p) const
        {
            if (p <= 0.0)
                return 1.0; // Strictly speaking, this result should be infinity.
            else
            {
                if (p > 100.0)
                {
                    double big = 0.0;
                    for (std::size_t row = 0; row < rows; row++)
                    {
                        for (std::size_t column = 0; column < columns; column++)
                        {
                            double value = std::abs((double)getValue(row, column));
                            if (value > big) big = value;
                        }
                    }
                    return big;
                }
                else
                if (p == 1.0)
                {
                    double sum = 0.0;
                    for (std::size_t row = 0; row < rows; row++)
                    {
                        for (std::size_t column = 0; column < columns; column++)
                        {
                            double value = getValue(row, column);
                            sum += value;
                        }
                    }
                    return sum;
                }
                else
                {
                    double sum = 0.0;
                    for (std::size_t row = 0; row < rows; row++)
                    {
                        for (std::size_t column = 0; column < columns; column++)
                        {
                            double value = getValue(row, column);
                            sum += std::pow(value, p);
                        }
                    }
                    return std::pow(sum, 1.0 / p);
                }
            }
        }

        double differenceNorm(const Matrix<T>& m, double p) const
        {
            if (p <= 0.0)
                return 1.0; // Strictly speaking, this result should be infinity.
            else
            {
                if (p > 100.0)
                {
                    double big = 0.0;
                    for (std::size_t row = 0; row < rows; row++)
                    {
                        for (std::size_t column = 0; column < columns; column++)
                        {
                            double value = std::abs((double)getValue(row, column) - m.getValue(row, column));
                            if (value > big) big = value;
                        }
                    }
                    return big;
                }
                else
                    if (p == 1.0)
                    {
                        double sum = 0.0;
                        for (std::size_t row = 0; row < rows; row++)
                        {
                            for (std::size_t column = 0; column < columns; column++)
                            {
                                double value = (double)getValue(row, column) - m.getValue(row, column);
                                sum += value;
                            }
                        }
                        return sum;
                    }
                    else
                    {
                        double sum = 0.0;
                        for (std::size_t row = 0; row < rows; row++)
                        {
                            for (std::size_t column = 0; column < columns; column++)
                            {
                                double value = (double)getValue(row, column) - m.getValue(row, column);
                                sum += std::pow(value, p);
                            }
                        }
                        return std::pow(sum, 1.0 / p);
                    }
            }
        }

        double largestDifference(const Matrix<T>& m) const
        {
            double big = 0.0;
            for (std::size_t row = 0; row < rows; row++)
            {
                for (std::size_t column = 0; column < columns; column++)
                {
                    double value = std::abs((double)getValue(row, column) - m.getValue(row, column));
                    if (value > big) big = value;
                }
            }
            return big;
        }

        bool exceedsThresholdDifference(const Matrix<T>& m, double threshold) const
        {
            for (std::size_t row = 0; row < rows; row++)
            {
                for (std::size_t column = 0; column < columns; column++)
                {
                    double value = std::abs((double)getValue(row, column) - m.getValue(row, column));
                    if (value > threshold) return true;
                }
            }
            return false;
        }


        void add(T k)
        {
            for (std::size_t row = 0; row < rows; row++)
            {
                for (std::size_t column = 0; column < columns; column++)
                {
                    setValue(row, column, getValue(row, column) + k);
                }
            }
        }

        void subtract(T k)
        {
            for (std::size_t row = 0; row < rows; row++)
            {
                for (std::size_t column = 0; column < columns; column++)
                {
                    setValue(row, column, getValue(row, column) - k);
                }
            }
        }

        void multiply(T k)
        {
            for (std::size_t row = 0; row < rows; row++)
            {
                for (std::size_t column = 0; column < columns; column++)
                {
                    setValue(row, column, getValue(row, column) * k);
                }
            }
        }

        void divide(T k)
        {
            for (std::size_t row = 0; row < rows; row++)
            {
                for (std::size_t column = 0; column < columns; column++)
                {
                    setValue(row, column, getValue(row, column) / k);
                }
            }
        }

        void subtractFrom(T k)
        {
            for (std::size_t row = 0; row < rows; row++)
            {
                for (std::size_t column = 0; column < columns; column++)
                {
                    setValue(row, column, k - getValue(row, column));
                }
            }
        }

        void add(const Matrix<T>& m)
        {
            assert((rows == m.rows) && (columns == m.columns));
            for (std::size_t row = 0; row < rows; row++)
            {
                for (std::size_t column = 0; column < columns; column++)
                {
                    setValue(row, column, getValue(row, column) + m.getValue(row, column));
                }
            }
        }

        void subtract(const Matrix<T>& m)
        {
            assert((rows == m.rows) && (columns == m.columns));
            for (std::size_t row = 0; row < rows; row++)
            {
                for (std::size_t column = 0; column < columns; column++)
                {
                    setValue(row, column, getValue(row, column) - m.getValue(row, column));
                }
            }
        }

        void multiply(const Matrix<T>& a, const Matrix<T>& b)
        {
            assert(a.columns == b.rows);
            reshape(a.rows, b.columns);
            std::size_t dim = a.columns;
            for (std::size_t row = 0; row < rows; row++)
            {
                for (std::size_t column = 0; column < columns; column++)
                {
                    T sum = Zero;
                    for (std::size_t index = 0; index < dim; index++)
                    {
                        sum += a.getValue(row, index) * b.getValue(index, column);
                    }
                    setValue(row, column, sum);
                }
            }
        }

        void multiplyAsColumn(const Matrix<T>& m, const Matrix<T>& c)
        {
            std::size_t length = c.rows * c.columns;
            assert(m.rows == length);
            reshape(c.rows, c.columns);
            for (std::size_t column = 0; column < columns; column++)
            {
                for (std::size_t row = 0; row < rows; row++)
                {
                    std::size_t mrow = column * rows + row;
                    T sum = Zero;
                    for (std::size_t ccol = 0; ccol < columns; ccol++)
                    {
                        for (std::size_t crow = 0; crow < rows; crow++)
                        {
                            std::size_t mcol = ccol * rows + crow;
                            sum += m.getValue(mrow, mcol) * c.getValue(crow, ccol);
                        }
                    }
                    setValue(row, column, sum);
                }
            }
        }

        void multiplyHadamard(const Matrix<T>& m)
        {
            assert((rows == m.rows) && (columns == m.columns));
            for (std::size_t row = 0; row < rows; row++)
            {
                for (std::size_t column = 0; column < columns; column++)
                {
                    setValue(row, column, getValue(row, column) * m.getValue(row, column));
                }
            }
        }

        void multiplyHadamard(const Matrix<T>& a, const Matrix<T>& b)
        {
            assert((a.rows == b.rows) && (a.columns == b.columns));
            reshape(a.rows, a.columns);
            for (std::size_t row = 0; row < rows; row++)
            {
                for (std::size_t column = 0; column < columns; column++)
                {
                    setValue(row, column, a.getValue(row, column) * b.getValue(row, column));
                }
            }
        }

        void multiplyKronecker(const Matrix<T>& a, const Matrix<T>& b)
        {
            reshape(a.rows * b.rows, a.columns * b.columns);
            std::size_t row = 0;
            for (std::size_t arow = 0; arow < a.rows; arow++)
            {
                for (std::size_t brow = 0; brow < b.rows; brow++, row++)
                {
                    std::size_t column = 0;
                    for (std::size_t acolumn = 0; acolumn < a.columns; acolumn++)
                    {
                        for (std::size_t bcolumn = 0; bcolumn < b.columns; bcolumn++, column++)
                        {
                            setValue(row, column, a.getValue(arow, acolumn) * b.getValue(brow, bcolumn));
                        }
                    }
                }
            }
        }
    };

}
