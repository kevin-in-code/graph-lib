
#pragma once

/**
 * Graph_ErdosRenyi.hpp
 * Purpose: Generate random graphs using the Gnp and Gnm Erdos-Renyi models.
 *
 * @author Kevin A. Naudé
 * @version 1.1
 */

#include <algorithm>
#include <cstdint>
#include <memory>
#include <Random.hpp>
#include <AttributeModel.hpp>
#include <Graph.hpp>

namespace kn
{

	namespace ErdosRenyi
	{
		Graph* Gnp(Random& r, uint32_t n, double p, const AttributeModel* vertexAttributes, const AttributeModel* edgeAttributes);
		Graph* Gnm(Random& r, uint32_t n, uint32_t m, const AttributeModel* vertexAttributes, const AttributeModel* edgeAttributes);
	}

}
