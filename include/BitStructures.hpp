
#pragma once

/**
 * BitStructures.hpp
 * Purpose: To provide an efficient non-negative integer set.
 *
 * @author Kevin A. Naudé
 * @version 1.1
 */

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cassert>

namespace kn
{

    /**
     * These are fast inline functions which enable iterating through the bits set in an integer.
     * I use a well known trick to isolate the lowest set bit in 2's complement binary integers.
     *
     * This isolated bit is always numerically a power of 2.  Multiplying by the bit is thus
     * equivalent to a left bit shift.  I use this fact, and another somewhat well known trick
     * to identify the bit index.  A specially chosen De Bruijn sequence is multiplied by the
     * binary value of the bit.  This shifts the De Bruijn sequence which allows us to select
     * a subequence of 6 bits.  Each subsequence is unique for any suitably long DeBruijn
     * sequence, and so it uniquely identifies the bit position.  The bit sequence is then
     * used as a small index into a precomputed inverse mapping table to obtain the bit index.
     *
     * Note: There are multiple De Bruin sequences with the neccessary properties to support this
     * technique.  I explored the search space of such sequences some time ago.  You may find
     * other sequences that produce the same behaviour although they may be numerically distinct.
     */
    extern const int InverseDeBruijnSubsequenceTable[64];
    extern const int BitCountTable[256];

    inline int bitToIndex(uint64_t bit)
    {
        return InverseDeBruijnSubsequenceTable[(std::size_t)(((uint64_t)bit * UINT64_C(0x043147259A7ABB7E)) >> 58)];
    }

    inline uint64_t singleBit(int index)
    {
        return ((uint64_t)1) << index;
    }

    inline uint64_t lowestBit(uint64_t bits)
    {
        return (bits & (uint64_t)(-(int64_t)bits));
    }

    inline int lowestBitIndex(uint64_t bits)
    {
        return bitToIndex(lowestBit(bits));
    }

    inline int highestBitIndex(uint64_t bits)
    {
        int position;
        int shift;

        position = (bits > 0xFFFFFFFF) << 5;
        bits >>= position;

        shift = (bits > 0xFFFF) << 4;
        bits >>= shift;
        position |= shift;

        shift = (bits > 0xFF) << 3;
        bits >>= shift;
        position |= shift;

        shift = (bits > 0xF) << 2;
        bits >>= shift;
        position |= shift;

        shift = (bits > 0x3) << 1;
        bits >>= shift;
        position |= shift;

        position |= (bits >> 1);

        return position;
    }

    inline uint64_t highestBit(uint64_t bits)
    {
        return singleBit(highestBitIndex(bits));
    }

    inline int countBits(uint64_t bits)
    {
        return
            BitCountTable[bits & 255] +
            BitCountTable[(bits >> 8) & 255] +
            BitCountTable[(bits >> 16) & 255] +
            BitCountTable[(bits >> 24) & 255] +
            BitCountTable[(bits >> 32) & 255] +
            BitCountTable[(bits >> 40) & 255] +
            BitCountTable[(bits >> 48) & 255] +
            BitCountTable[(bits >> 56) & 255];
    }


    /**
    * The IntegerSet provides a fast implementation of a set of non-negative integers,
    * by means of a bit vector. Each binary digit designates the presence or absence
    * of a candidate element.
    */
    class IntegerSet
    {
    public:
        /**
        * The IntegerSet Iterator provides fast iteration through the elements of a bit vector.
        * If the bit vector is sparse, the iteration remains fairly efficient by the following:
        *   - zero words are skipped immediately (skipping 64 possible entries at once)
        *   - non-zero words are processed in O(n), where n is the number of bits set in the word.
        */
        struct Iterator
        {
        private:
            friend class IntegerSet;

            uint64_t* array;

            std::size_t arraySize;
            std::size_t currentIndex;

            std::size_t currentBaseValue;
            uint64_t currentBits;
            uint64_t currentMask;

            Iterator(uint64_t* array, std::size_t arraySize)
            {
                this->array = array;
                this->arraySize = arraySize;

                currentIndex = 0;
                currentBaseValue = 0;
                currentBits = (arraySize) ? array[0] : 0;
                currentMask = 0xFFFFFFFFFFFFFFFFULL;
            }

        public:
            bool hasNext()
            {
                if (currentIndex < arraySize)
                {
                    currentBits = array[currentIndex] & currentMask;
                }
                while ((currentBits == 0) && (currentIndex + 1 < arraySize))
                {
                    currentIndex++;
                    currentBits = array[currentIndex];
                    currentMask = 0xFFFFFFFFFFFFFFFFULL;
                    currentBaseValue += 64;
                }

                return (currentBits != 0);
            }

            std::size_t next()
            {
                uint64_t bit = lowestBit(currentBits);
                currentBits ^= bit;
                currentMask = ~(bit - 1) - bit;
                return currentBaseValue + bitToIndex(bit);
            }
        };

    private:
        std::size_t maxCardinality;
        std::size_t arraySize;
        uint64_t* array;

        bool verifyIsEmpty() const;

        void sanitiseHighBits()
        {
            /// Here, I correct for a cardinality that does not lie on a 64-bit boundary.
            if ((maxCardinality & 63) != 0)
            {
                array[arraySize - 1] &= (singleBit(maxCardinality & 63) - 1);
            }
        }

    public:
        IntegerSet();
        IntegerSet(std::size_t maxCardinality);
        IntegerSet(const IntegerSet& pattern);
        ~IntegerSet();

        IntegerSet& operator=(const IntegerSet& pattern);

        IntegerSet(IntegerSet&& pattern);
        IntegerSet& operator=(IntegerSet&& pattern);

        Iterator iterator() const
        {
            return Iterator(array, arraySize);
        }

        void setMaxCardinality(std::size_t maxCardinality);

        void add(std::size_t value)
        {
            assert(value < maxCardinality);

            std::size_t index = value / 64;
            uint64_t bit = singleBit(value % 64);
            array[index] |= bit;
        }

        void remove(std::size_t value)
        {
            assert(value < maxCardinality);

            std::size_t index = value / 64;
            uint64_t bit = singleBit(value % 64);
            array[index] &= ~bit;
        }

        bool contains(std::size_t value) const
        {
            if (value >= maxCardinality) return false;

            std::size_t index = value / 64;
            uint64_t bit = singleBit(value % 64);
            return (0 != (array[index] & bit));
        }

        bool isEmpty() const
        {
            return (array[0] == 0) && verifyIsEmpty();
        }

        std::size_t firstElement() const
        {
            std::size_t currentIndex = 0;
            std::size_t currentBaseValue = 0;
            uint64_t currentBits = array[currentIndex];
            while ((currentBits == 0) && (currentIndex + 1 < arraySize))
            {
                currentIndex++;
                currentBits = array[currentIndex];
                currentBaseValue += 64;
            }
            if (currentBits != 0)
            {
                return currentBaseValue + lowestBitIndex(currentBits);
            }
            else
                return maxCardinality;
        }

        std::size_t lastElement() const
        {
            std::size_t currentIndex = arraySize - 1;
            std::size_t currentBaseValue = currentIndex * 64;
            uint64_t currentBits = array[currentIndex];
            while ((currentBits == 0) && (currentIndex > 0))
            {
                currentIndex--;
                currentBits = array[currentIndex];
                currentBaseValue -= 64;
            }
            if (currentBits != 0)
            {
                return currentBaseValue + highestBitIndex(currentBits);
            }
            else
                return maxCardinality;
        }

        void clear();
        void fill();
        void copy(const IntegerSet& b);

        std::size_t count() const;
        std::size_t countLimit(std::size_t limit) const;
        std::size_t countCommon(const IntegerSet& b) const;
        std::size_t countCommonLimit(const IntegerSet& b, std::size_t limit, std::size_t& w) const;

        void invert();

        void intersectWith(const IntegerSet& b);
        void unionWith(const IntegerSet& b);

        void removeAll(const IntegerSet& b);

        void intersection(const IntegerSet& a, const IntegerSet& b);

        /*
        void list(const char* head)
        {
            std::cout << head << "{";
            bool first = true;
            for (std::size_t k = 0; k < maxCardinality; k++)
            {
                if (contains(k))
                {
                    if (!first) std::cout << ",";
                    std::cout << " " << k;
                    first = false;
                }
            }
            std::cout << "}" << std::endl;
        }
        */

    };

}
