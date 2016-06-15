
#pragma once

/**
 * AttributeModel.hpp
 * Purpose: An abstract base model for indexed attributes.
 *
 * @author Kevin A. Naudé
 * @version 1.1
 */

#include <string>
#include <vector>
#include <cstdlib>
#include <Random.hpp>

namespace kn
{

	class AttributeModel
	{
	public:
		virtual ~AttributeModel() {}

		virtual std::size_t count() const
		{
			return 0;
		}

		virtual int relation(std::size_t idA, std::size_t idB) const
		{
			return 0;
		}

		virtual double similarity(std::size_t idA, std::size_t idB) const
		{
			return (relation(idA, idB) == 0) ? 1.0 : 0.0;
		}
	};

	class NullAttributeModel : public AttributeModel {};

	template <typename T>
	class VectorAttributeModel : public AttributeModel
	{
	private:
		std::vector<T> attributes;

	public:
		VectorAttributeModel(std::vector<T> attrs)
			: attributes(attrs)
		{
		}

		virtual int relation(std::size_t idA, std::size_t idB) const
		{
			if ((idA == idB) || (attributes[idA] == attributes[idB]))
				return 0;
			else
			if (attributes[idA] < attributes[idB])
				return -1;
			else
				return +1;
		}
	};

}
