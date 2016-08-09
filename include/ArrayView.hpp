
#pragma once

/**
* ArrayView.hpp
* Purpose: Provides transient view over an external array, with index checking.
*
* @author Kevin A. Naudé
* @version 1.1
*/

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <assert.h>

namespace kn
{

    template <typename T>
    class ArrayView
    {
    private:
        T* values;
        std::size_t length;
        std::size_t shift;
        /**
        * The role of 'shift' is for indexing elements separated by a power of two stride.
        * There are various scenarios in which indexing elements separated by an arbitrary stride
        * would be convenient, but including a multiply per index operation would be costly
        * to the common case in which the stride is 1.  Restricting the stride to powers
        * of two is a reasonable compromise.
        *
        * For an example of use, see the matrix class which exposes columns and rows as views.
        */

    public:
        ArrayView() : ArrayView(nullptr, 0, 1) {}

        ArrayView(T* values, std::size_t length) : ArrayView(values, length, 1) {}

        ArrayView(T* values, std::size_t length, std::size_t shift)
        {
            this->values = values;
            this->length = length;
            this->shift = shift;
        }

        ArrayView(const ArrayView<T>& other) : ArrayView(other.values, other.length, other.shift) {}

        std::size_t size() const
        {
            return length;
        }

        ArrayView<T>& operator=(const ArrayView<T>& other)
        {
            this->values = other.values;
            this->length = other.length;
            this->shift = other.shift;
            return *this;
        }


        const T& operator[](std::size_t index) const
        {
#if defined(DEBUG) || defined(_DEBUG)
            if (index >= length) throw std::out_of_range("ArrayView index out of range");
#endif
            return &values[index << shift];
        }

        T& operator[](std::size_t index)
        {
#if defined(DEBUG) || defined(_DEBUG)
            if (index >= length) throw std::out_of_range("ArrayView index out of range");
#endif
            return values[index << shift];
        }
    };

}
