
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
#include <stack>
#include <iostream>
#include <sstream>
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

    class PrettyPrintCliqueReceiver : public CliqueReceiver
    {
    private:
        std::size_t indentation = 0;
        std::size_t nextIndent = 0;
        bool onNewLine = true;
        std::stack<std::size_t> levels;

        void breakLine(bool force = false)
        {
            if (!onNewLine || force)
            {
                std::cout << std::endl;
                for (int t = 0; t < indentation; t++)
                {
                    std::cout << " ";
                }
                onNewLine = true;
            }
        }

    public:
        virtual void onOpenGroup()
        {
            levels.push(indentation);
            indentation = nextIndent;
            nextIndent = indentation + 2;
            std::cout << "( ";
            onNewLine = false;
        }

        virtual void onPartition()
        {
            breakLine();
            std::cout << "| ";
            nextIndent = indentation + 2;
            onNewLine = false;
        }

        virtual void onCloseGroup()
        {
            breakLine();
            std::cout << ")";
            indentation = levels.top();
            levels.pop();
            nextIndent = indentation;
            breakLine(true);
        }

        virtual void onVertex(std::size_t v, std::size_t a)
        {
            std::stringstream ss;
            ss << v << ":" << a << " ";
            std::string s = ss.str();
            std::cout << s;
            nextIndent += s.length();
            onNewLine = false;
        }

        virtual void onOk()
        {
            std::cout << "[ok!]";
            onNewLine = false;
            breakLine(true);
        }

        virtual void onCutOff()
        {
            std::cout << "[cut]";
            onNewLine = false;
            breakLine(true);
        }
    };

    void AllCliques_Tomita(const Graph* graph, CliqueReceiver* receiver);

    void AllCliques_Naude(const Graph* graph, CliqueReceiver* receiver);

}
