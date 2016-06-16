
#include <MersenneTwister.hpp>

namespace kn
{

#define MT_MAYBE_A(y)((uint32_t)(-((int32_t)((y)&1))) & MT_A)

    MersenneTwister::MersenneTwister(uint32_t seed)
        : Random()
    {
        uint32_t y = seed & MT_BITS;
        x[0] = y;
        for (std::size_t i = 1; i < MT_N; i++)
        {
            y = x[i] = (MT_LCG_M * (y ^ (y >> MT_LCG_D)) + i) & MT_BITS;
        }
        next = MT_N;

        /* Give things a bit of a stir. */
        flush();
        flush();
    }

    MersenneTwister::MersenneTwister(Random& seed)
        : Random()
    {
        for (std::size_t i = 0; i < MT_N; i++)
        {
            x[i] = seed.nextBits() ^ seed.nextBits() ^ seed.nextBits();
        }
        next = MT_N;
    }

    void MersenneTwister::flush()
    {
        uint32_t y;
        for (std::size_t k = 0; k < (MT_N - MT_M); k++)
        {
            y = (x[k] & MT_X) | (x[k + 1] & MT_Y);
            x[k] = x[k + MT_M] ^ MT_MAYBE_A(y);
        }
        for (std::size_t k = MT_N - MT_M; k < MT_N - 1; k++)
        {
            y = (x[k] & MT_X) | (x[k + 1] & MT_Y);
            x[k] = x[k - (MT_N - MT_M)] ^ MT_MAYBE_A(y);
        }
        y = (x[MT_N - 1] & MT_X) | (x[0] & MT_Y);
        x[MT_N - 1] = x[MT_M - 1] ^ MT_MAYBE_A(y);
        next = 0;
        Random::flush();
    }

}
