
#pragma once

/**
 * MersenneTwister.hpp
 * Purpose: This is a correctly initialised implementation of the long period Mersenne Twister PRNG.
 *
 * @author Kevin A. Naudé
 * @note Based on published works.
 * @version 1.1
 */

#include <cstdint>
#include <cstdlib>
#include <math.h>
#include <Random.hpp>

namespace kn
{

/*
#define MT_N 624
#define MT_M 397
#define MT_U 11
#define MT_S 7
#define MT_T 15
#define MT_L 18
#define MT_A 0x9908B0DFU
#define MT_B 0x9D2C5680U
#define MT_C 0xEFC60000U
#define MT_X 0x80000000U
#define MT_Y 0x7FFFFFFFU

#define MT_BITS 0xFFFFFFFFU

#define MT_LCG_M 1812433253
#define MT_LCG_D 30
*/

	class MersenneTwister : public Random
	{
	private:
		static const std::size_t MT_N = 624;
		const std::size_t MT_M = 397;

		const int MT_U = 11;
		const int MT_S = 7;
		const int MT_T = 15;
		const int MT_L = 18;

		const uint32_t MT_A = 0x9908B0DFU;
		const uint32_t MT_B = 0x9D2C5680U;
		const uint32_t MT_C = 0xEFC60000U;
		const uint32_t MT_X = 0x80000000U;
		const uint32_t MT_Y = 0x7FFFFFFFU;

		const uint32_t MT_BITS = 0xFFFFFFFFU;
		const uint32_t MT_LCG_M = 1812433253;
		const int      MT_LCG_D = 30;

	private:
		std::size_t next;
		uint32_t x[MT_N];

	public:
		MersenneTwister(uint32_t seed);
		MersenneTwister(Random& seed);
		virtual void flush();

		virtual uint32_t nextBits()
		{
			if (next >= MT_N) flush();
			uint32_t y = x[next++];
			y ^= y >> MT_U;
			y ^= (y << MT_S) & MT_B;
			y ^= (y << MT_T) & MT_C;
			y ^= (y >> MT_L);
			return y;
		}
	};

}
