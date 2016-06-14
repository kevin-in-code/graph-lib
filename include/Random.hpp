#pragma once

/**
* Random.hpp
* Purpose: Base class for PRNG implementations.
*
* @author Kevin A. Naudé
* @version 1.1
*/

#include <cstdint>
#include <math.h>

class Random
{
private:
	bool haveNormal;
	double normal;

public:
	Random()
	{
		haveNormal = false;
	}

	virtual void flush()
	{
		haveNormal = false;
	}

	virtual uint32_t nextBits() = 0;

	uint32_t nextUInt(uint32_t n)
	{
		/// The common approach of returning (random() % n) is stastically biased, and should not be done.

		/* Non-useful cases */
		if (n <= 1) return 0;

		uint32_t num = nextBits();

		/* Optimise case: powers of 2 */
		if ((n & (uint32_t)(-(int32_t)n)) == n)
		{
			return num & (n - 1);
		}
		else
			/* General case */
		{
			uint32_t divisor = ((uint32_t)0xFFFFFFFFU) / n;
			uint32_t threshold = n * divisor;
			while (num >= threshold)
			{
				num = nextBits();
			}
			num /= divisor;
			return num;
		}
	}

	double nextDoubleCC()
	{
		return
			(nextBits() / 4294967296.0)
			+ ((nextBits() & 0x1FFFFFU) / (2097151.0 * 4294967296.0));
	}

	double nextDoubleCO()
	{
		return
			(nextBits() / 4294967296.0)
			+ ((nextBits() & 0x1FFFFFU) / (2097152.0 * 4294967296.0));
	}

	double nextDoubleOO()
	{
		uint32_t x = nextBits();
		uint32_t y = nextBits() & 0x1FFFFFU;
		if ((x | y) == 0)
		{
			do
			{
				y = nextBits() & 0x1FFFFFU;
			} while (y == 0);
		}
		return
			(x / 4294967296.0)
			+ (y / (2097152.0 * 4294967296.0));
	}

	double nextDoubleOC() { return 1.0 - nextDoubleCO(); }

	double nextNormal()
	{
		if (haveNormal)
		{
			haveNormal = 0;
			return normal;
		}
		else
		{
			double radius, theta;
			radius = sqrt(-2.0 * log(nextDoubleOC()));
			theta = 2.0 * 3.14159265358979323846264338328 * nextDoubleCO();

			normal = radius * sin(theta); // this is the second normal of the pair
			haveNormal = 1;

			return radius * cos(theta);
		}
	}

	double nextGaussian(double mean, double stdev)
	{
		return mean + stdev * nextNormal();
	}
};

