
#pragma once

/**
* View.hpp
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
    class View
    {
    private:
        T* values;
        size_t length;

    public:
        View() : View(nullptr, 0) {}

        View(T* values, size_t length)
        {
            this->values = values;
            this->length = length;
        }

        View(const View<T>& other) : View(other.values, other.length) {}

        size_t size() const
        {
            return length;
        }

        View<T>& operator=(const View<T>& other)
        {
            this->values = other.values;
            this->length = other.length;
            return *this;
        }


        const T& operator[](size_t index) const
        {
#if defined(DEBUG) || defined(_DEBUG)
            if (index >= length) throw std::out_of_range("View index out of range");
#endif
            return &values[index];
        }

        T& operator[](size_t index)
        {
#if defined(DEBUG) || defined(_DEBUG)
            if (index >= length) throw std::out_of_range("View index out of range");
#endif
            return values[index];
        }
    };

}
