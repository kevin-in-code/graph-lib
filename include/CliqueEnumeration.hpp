
#pragma once

/**
 * CliqueEnumeration.hpp
 * Purpose: Implements new and well known clique enumeration algorithms.
 *
 * @author Kevin A. Naudé
 * @version 1.1
 */

#include <algorithm>
#include <memory>
#include <assert.h>
#include <BitStructures.hpp>
#include <Graph.hpp>

namespace kn
{

	class CliqueReceiver
	{
	public:
		virtual void onClear() {}

		virtual void onClique(const Graph& graph, const IntegerSet& vertices) {}

		virtual void onComplete() {}
	};

	void AllCliques_Tomita(const Graph* graph, CliqueReceiver* receiver);

	void AllCliques_Naude(const Graph* graph, CliqueReceiver* receiver);

}
