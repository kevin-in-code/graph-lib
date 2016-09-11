
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
    private:
        friend class BKSearch;
        uint64_t cliqueCounter = 0;
        uint64_t recursionCounter = 0;
        uint64_t cutOffCounter = 0;

    public:
        void reset()
        {
            cliqueCounter = 0;
            recursionCounter = 0;
            cutOffCounter = 0;
        }

        uint64_t cliqueCount() { return cliqueCounter; }
        uint64_t recursionCount() { return recursionCounter; }
        uint64_t cutOffCount() { return cutOffCounter; }

        virtual void onClear() {}

        virtual void onClique(const Graph& graph, const IntegerSet& vertices) {}

        virtual void onOpenGroup() {}
        virtual void onPartition() {}
        virtual void onCloseGroup() {}

        virtual void onVertex(std::size_t v, std::size_t a) {}
        virtual void onOk() {}
        virtual void onCutOff() {}

        virtual void onComplete() {}
    };

    void AllCliques_Tomita(const Graph* graph, CliqueReceiver* receiver);

    void AllCliques_Naude(const Graph* graph, CliqueReceiver* receiver);

}
