
#include <BitStructures.hpp>

namespace kn
{

    /// Inverse lookup for De Bruijn sequence given by 0x043147259A7ABB7E
    const int InverseDeBruijnSubsequenceTable[] =
    {
        63, 0, 1, 6, 2, 12, 7, 18,
        3, 24, 13, 27, 8, 33, 19, 39,
        4, 16, 25, 37, 14, 45, 28, 47,
        9, 30, 34, 53, 20, 49, 40, 56,
        62, 5, 11, 17, 23, 26, 32, 38,
        15, 36, 44, 46, 29, 52, 48, 55,
        61, 10, 22, 31, 35, 43, 51, 54,
        60, 21, 42, 50, 59, 41, 58, 57
    };

#define BitCountTableRow(n)(n+0), (n+1), (n+1), (n+2), (n+1), (n+2), (n+2), (n+3), (n+1), (n+2), (n+2), (n+3), (n+2), (n+3), (n+3), (n+4)

    const int BitCountTable[] =
    {
        // 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
        BitCountTableRow(0),
        BitCountTableRow(1),
        BitCountTableRow(1),
        BitCountTableRow(2),
        BitCountTableRow(1),
        BitCountTableRow(2),
        BitCountTableRow(2),
        BitCountTableRow(3),
        BitCountTableRow(1),
        BitCountTableRow(2),
        BitCountTableRow(2),
        BitCountTableRow(3),
        BitCountTableRow(2),
        BitCountTableRow(3),
        BitCountTableRow(3),
        BitCountTableRow(4)
    };

#undef BitCountTableRow


    bool IntegerSet::verifyIsEmpty() const
    {
        for (std::size_t index = 0; index < arraySize; index++)
        {
            if (array[index] != 0) return false;
        }
        return true;
    }

    IntegerSet::IntegerSet()
    {
        maxCardinality = 1;
        arraySize = 1;
        array = new uint64_t[1];
        assert(array);
        array[0] = 0;
    }

    IntegerSet::IntegerSet(std::size_t maxCardinality)
    {
        this->maxCardinality = maxCardinality;
        arraySize = (maxCardinality + 63) / 64;
        array = new uint64_t[arraySize];
        assert(array);
        clear();
    }

    IntegerSet::IntegerSet(const IntegerSet& pattern)
    {
        maxCardinality = pattern.maxCardinality;
        arraySize = (maxCardinality + 63) / 64;
        array = new uint64_t[arraySize];
        assert(array);
        copy(pattern);
    }

    IntegerSet::~IntegerSet()
    {
        if (array) delete[] array;
    }

    IntegerSet& IntegerSet::operator=(const IntegerSet& pattern)
    {
        if (this != &pattern)
        {
            if (arraySize != pattern.arraySize)
            {
                delete[] array;
                array = new uint64_t[pattern.arraySize];
                assert(array);
                arraySize = pattern.arraySize;
            }
            maxCardinality = pattern.maxCardinality;

            copy(pattern);
        }
        return *this;
    }

    /// Move constructor
    IntegerSet::IntegerSet(IntegerSet&& pattern)
        : maxCardinality(0), arraySize(0), array(nullptr)
    {
        maxCardinality = pattern.maxCardinality;
        arraySize = pattern.arraySize;
        array = pattern.array;

        pattern.array = nullptr;
        pattern.arraySize = 0;
        pattern.maxCardinality = 0;
    }

    /// Move assignment
    IntegerSet& IntegerSet::operator=(IntegerSet&& pattern)
    {
        if (this != &pattern)
        {
            if (array) delete[] array;

            maxCardinality = pattern.maxCardinality;
            arraySize = pattern.arraySize;
            array = pattern.array;

            pattern.array = nullptr;
            pattern.arraySize = 0;
            pattern.maxCardinality = 0;
        }
        return *this;
    }

    void IntegerSet::setMaxCardinality(std::size_t maxCardinality)
    {
        std::size_t arraySize = (maxCardinality + 63) / 64;
        if (this->arraySize != arraySize)
        {
            uint64_t* newArray = new uint64_t[arraySize];
            assert(newArray);
            for (std::size_t index = 0; index < this->arraySize; index++)
            {
                newArray[index] = array[index];
            }
            delete[] array;
            array = newArray;
            this->arraySize = arraySize;
        }
        this->maxCardinality = maxCardinality;
        sanitiseHighBits();
    }

    void IntegerSet::clear()
    {
        for (std::size_t index = 0; index < arraySize; index++)
        {
            array[index] = 0;
        }
    }

    void IntegerSet::fill()
    {
        for (std::size_t index = 0; index < arraySize; index++)
        {
            array[index] = ~0;
        }

        sanitiseHighBits();
    }

    void IntegerSet::copy(const IntegerSet& b)
    {
        assert(maxCardinality == b.maxCardinality);
        for (std::size_t index = 0; index < arraySize; index++)
        {
            array[index] = b.array[index];
        }
    }

    std::size_t IntegerSet::count() const
    {
        std::size_t sum = 0;
        for (std::size_t index = 0; index < arraySize; index++)
        {
            sum += countBits(array[index]);
        }
        return sum;
    }

    std::size_t IntegerSet::countCommon(const IntegerSet& b) const
    {
        assert(maxCardinality == b.maxCardinality);
        std::size_t sum = 0;
        for (std::size_t index = 0; index < arraySize; index++)
        {
            uint64_t bits = array[index] & b.array[index];
            sum += countBits(bits);
        }
        return sum;
    }

    std::size_t IntegerSet::countCommonLimit(const IntegerSet& b, std::size_t limit, std::size_t& w) const
    {
        assert(maxCardinality == b.maxCardinality);
        std::size_t sum = 0;
        std::size_t theW = maxCardinality;
        for (std::size_t index = 0; index < arraySize; index++)
        {
            uint64_t bits = array[index] & b.array[index];
            if (bits != 0)
            {
                sum += countBits(bits);
                theW = index * 64 + bitToIndex(lowestBit(bits)); /// any bit will do
                if (sum >= limit) break;
            }
        }
        w = theW;
        return sum;
    }

    void IntegerSet::invert()
    {
        for (std::size_t index = 0; index < arraySize; index++)
        {
            array[index] = ~array[index];
        }

        sanitiseHighBits();
    }

    void IntegerSet::intersectWith(const IntegerSet& b)
    {
        assert(maxCardinality == b.maxCardinality);
        for (std::size_t index = 0; index < arraySize; index++)
        {
            array[index] &= b.array[index];
        }
    }

    void IntegerSet::unionWith(const IntegerSet& b)
    {
        assert(maxCardinality == b.maxCardinality);
        for (std::size_t index = 0; index < arraySize; index++)
        {
            array[index] |= b.array[index];
        }
    }

    void IntegerSet::removeAll(const IntegerSet& b)
    {
        assert(maxCardinality == b.maxCardinality);
        for (std::size_t index = 0; index < arraySize; index++)
        {
            array[index] &= ~b.array[index];
        }
    }

    void IntegerSet::intersection(const IntegerSet& a, const IntegerSet& b)
    {
        assert(maxCardinality == a.maxCardinality);
        assert(maxCardinality == b.maxCardinality);
        for (std::size_t index = 0; index < arraySize; index++)
        {
            array[index] = a.array[index] & b.array[index];
        }
    }


}
