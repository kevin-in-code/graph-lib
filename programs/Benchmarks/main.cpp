
/**
 * Benchmarks
 * This program applies benchmarks for clique enumeration, as reported in literature.
 *
 * @author Kevin A. Naudé
 * @version 1.1
 */

#include <iostream>
#include <iomanip>
#include <string>
#include <string.h>
#include <time.h>
#include <algorithm>
#include <sstream>
#include <BitStructures.hpp>
#include <Graph.hpp>
#include <CliqueEnumeration.hpp>
#include <StopWatch.hpp>
#include <GraphLoader.hpp>
#include <MersenneTwister.hpp>
#include <Graph_ErdosRenyi.hpp>

using namespace kn;

#ifdef NDEBUG
typedef CliqueReceiver TheCliqueReceiver;
#else
typedef PrettyPrintCliqueReceiver TheCliqueReceiver;
#endif

typedef void(*CliqueEnumerator)(const Graph* g, CliqueReceiver* cr);

void test(CliqueEnumerator ce, const Graph& graph)
{
    TheCliqueReceiver cr;
    StopWatch sw;
    sw.start();
    ce(&graph, &cr);
    sw.stop();

    double seconds = sw.elapsedSeconds();
    std::cout << cr.cliqueCount() << " cliques, " << seconds << " seconds, " << cr.recursionCount() << " recursive calls" << std::endl;
}

struct FixedBenchmark
{
    std::string name;
    std::string filename;
    int level;
};

std::vector<FixedBenchmark> FixedBenchmarks = {
    FixedBenchmark { "C125.9", "C125.9.clq.b", 2},
    FixedBenchmark { "", "", 100 },
    FixedBenchmark{ "MANN_a9", "MANN_a9.clq.b", 0 },
    FixedBenchmark{ "brock200_1", "brock200_1.clq.b", 1 },
    FixedBenchmark{ "brock200_2", "brock200_2.clq.b", 0 },
    FixedBenchmark{ "brock200_3", "brock200_3.clq.b", 0 },
    FixedBenchmark{ "brock200_4", "brock200_4.clq.b", 1 },
    FixedBenchmark{ "", "", 100 },
    FixedBenchmark{ "c-fat500-5", "c-fat500-5.clq.b", 0 },
    FixedBenchmark{ "c-fat500-10", "c-fat500-10.clq.b", 0 },
    FixedBenchmark{ "", "", 100 },
    FixedBenchmark{ "DSJC500.1", "DSJC500.1.col.b", 0 },
    FixedBenchmark{ "DSJC500.5", "DSJC500.5.col.b", 1 },
    FixedBenchmark{ "DSJC1000.1", "DSJC1000.1.col.b", 0 },
    FixedBenchmark{ "DSJC1000.5", "DSJC1000.5.col.b", 2 },
    FixedBenchmark{ "", "", 100 },
    FixedBenchmark{ "hamming8-4", "hamming8-4.clq.b", 1 },
    FixedBenchmark{ "hamming6-2", "hamming6-2.clq.b", 0 },
    FixedBenchmark{ "", "", 100 },
    FixedBenchmark{ "johnson16-2-4", "johnson16-2-4.clq.b", 0 },
    FixedBenchmark{ "johnson8-4-4", "johnson8-4-4.clq.b", 0 },
    FixedBenchmark{ "", "", 100 },
    FixedBenchmark{ "keller4", "keller4.clq.b", 0 },
    FixedBenchmark{ "", "", 100 },
    FixedBenchmark{ "p_hat300-1", "p_hat300-1.clq.b", 0 },
    FixedBenchmark{ "p_hat300-2", "p_hat300-2.clq.b", 1 },
    FixedBenchmark{ "p_hat500-1", "p_hat500-1.clq.b", 0 },
    FixedBenchmark{ "p_hat700-1", "p_hat700-1.clq.b", 0 },
    FixedBenchmark{ "p_hat1000-1", "p_hat1000-1.clq.b", 0 },
    FixedBenchmark{ "p_hat1500-1", "p_hat1500-1.clq.b", 1 },
    FixedBenchmark{ "", "", 100 },
    FixedBenchmark{ "san400_0.5_1", "san400_0.5_1.clq.b", 2 },
    FixedBenchmark{ "sanr200_0.7", "sanr200_0.7.clq.b", 1 },
    FixedBenchmark{ "sanr400_0.5", "sanr400_0.5.clq.b", 1 }
};

struct SyntheticBenchmark
{
    uint32_t n;
    double p;
    int level;
};

std::vector<SyntheticBenchmark> SyntheticBenchmarks = {
    SyntheticBenchmark{ 10000, 0.001, 0 },
    SyntheticBenchmark{ 100, 0.6, 0 },
    SyntheticBenchmark{ 100, 0.7, 0 },
    SyntheticBenchmark{ 100, 0.8, 0 },
    SyntheticBenchmark{ 100, 0.9, 1 },
    SyntheticBenchmark{ 1, 1.0, 100 },
    SyntheticBenchmark{ 300, 0.1, 0 },
    SyntheticBenchmark{ 300, 0.2, 0 },
    SyntheticBenchmark{ 300, 0.3, 0 },
    SyntheticBenchmark{ 300, 0.4, 0 },
    SyntheticBenchmark{ 300, 0.5, 0 },
    SyntheticBenchmark{ 300, 0.6, 1 },
    SyntheticBenchmark{ 1, 1.0, 100 },
    SyntheticBenchmark{ 500, 0.1, 0 },
    SyntheticBenchmark{ 500, 0.2, 0 },
    SyntheticBenchmark{ 500, 0.3, 0 },
    SyntheticBenchmark{ 500, 0.4, 0 },
    SyntheticBenchmark{ 500, 0.5, 1 },
    SyntheticBenchmark{ 1, 1.0, 100 },
    SyntheticBenchmark{ 700, 0.1, 0 },
    SyntheticBenchmark{ 700, 0.2, 0 },
    SyntheticBenchmark{ 700, 0.3, 0 },
    SyntheticBenchmark{ 1, 1.0, 100 },
    SyntheticBenchmark{ 1000, 0.1, 0 },
    SyntheticBenchmark{ 1000, 0.2, 0 },
    SyntheticBenchmark{ 1000, 0.3, 0 },
    SyntheticBenchmark{ 2000, 0.1, 0 },
    SyntheticBenchmark{ 1, 1.0, 100 },
    SyntheticBenchmark{ 3000, 0.1, 0 },
    SyntheticBenchmark{ 1, 1.0, 100 },
    SyntheticBenchmark{ 10000, 0.003, 0 },
    SyntheticBenchmark{ 10000, 0.005, 0 },
    SyntheticBenchmark{ 10000, 0.01, 0 },
    SyntheticBenchmark{ 10000, 0.03, 1 }
};

std::string formatDouble(double v)
{
    std::stringstream ss;
    ss << std::fixed << std::setprecision(3) << v;
    return ss.str();
}

std::string formatSeconds(double v)
{
    std::stringstream ss;
    ss << std::fixed << std::setprecision(3) << std::setw(9) << v;
    return ss.str();
}

std::string formatInt(uint64_t v, int width)
{
    std::stringstream ss;
    ss << std::setw(width) << v;
    return ss.str();
}

std::string formatName(std::string v)
{
    std::stringstream ss;
    ss << std::setw(13) << v;
    return ss.str();
}

int main(int argc, const char* argv[])
{
    if ((argc < 2) || ((strcmp(argv[1], "tomita-et-al") != 0) && (strcmp(argv[1], "naude") != 0) && (strcmp(argv[1], "segundo-et-al") != 0)))
    {
        std::cout << "usage: program algorithm [level]" << std::endl;
        std::cout << "  e.g. program tomita-et-al" << std::endl;
        std::cout << std::endl;
        std::cout << " tomita-et-al   use Tomita et al. pivot selection" << std::endl;
        std::cout << " naude          use Naude's pivot selection" << std::endl;
        std::cout << " segundo-et-al  use Segundo et al. pivot selection" << std::endl;
        std::cout << std::endl;
        std::cout << " 0, 1, 2        level of complexity allowed, default is 2 (full complexity)" << std::endl;
        std::cout << std::endl;
        std::cout << " important note: all required benchmark files must be in the working directory" << std::endl;
    }
    else
    {
        CliqueEnumerator ce = nullptr;

        if (strcmp(argv[1], "tomita-et-al") == 0)
        {
            ce = &AllCliques_Tomita;
        }
        else
        if (strcmp(argv[1], "naude") == 0)
        {
            ce = &AllCliques_Naude;
        }
        else
        if (strcmp(argv[1], "segundo-et-al") == 0)
        {
            ce = &AllCliques_Segundo;
        }
        else
        {
            std::cout << "argument \"" << argv[1] << "\" is not a recognised maximal clique enumerator" << std::endl;
        }


        int level = 2;
        if (argc >= 3) level = atoi(argv[2]);


        for (std::size_t t = 0; t < FixedBenchmarks.size(); t++)
        {
            if (FixedBenchmarks[t].level <= level)
            {
                GraphLoader loader(FixedBenchmarks[t].filename);

                if (loader.isOpen())
                {
                    Graph* g = loader.loadDIMACSB();
                    delete g;
                }
                else
                {
                    std::cout << "Fatal error: could not load " << FixedBenchmarks[t].filename << std::endl;
                    return -1;
                }
            }
        }

        /*
        std::cout << "======================================================================" << std::endl;
        for (std::size_t t = 0; t < FixedBenchmarks.size(); t++)
        {
            if (FixedBenchmarks[t].level >= 100)
            {
                std::cout << "----------------------------------------------------------------------" << std::endl;
            }
            else
            if (FixedBenchmarks[t].level <= level)
            {
                GraphLoader loader(FixedBenchmarks[t].filename);

                if (loader.isOpen())
                {
                    Graph* g = loader.loadDIMACSB();

                    CliqueReceiver cr;
                    StopWatch sw;

                    sw.start();
                    ce(g, &cr);
                    sw.stop();

                    double seconds = sw.elapsedSeconds();
                    std::cout << formatName(FixedBenchmarks[t].name) << " : " << formatInt(cr.cliqueCount(), 10) << " cliques, " << formatInt(cr.recursionCount(), 11) << " calls, " << formatSeconds(seconds) << " seconds" << std::endl;

                    delete g;
                }
            }
        }

        std::cout << "======================================================================" << std::endl;*/

        MersenneTwister random((uint32_t)time(NULL));
        for (std::size_t t = 0; t < SyntheticBenchmarks.size(); t++)
        {
            if (SyntheticBenchmarks[t].level >= 100)
            {
                std::cout << "----------------------------------------------------------------------" << std::endl;
            }
            else
                if (SyntheticBenchmarks[t].level <= level)
                {
                    Graph* g = ErdosRenyi::Gnp(random, SyntheticBenchmarks[t].n, SyntheticBenchmarks[t].p, nullptr, nullptr);

                    CliqueReceiver cr;
                    StopWatch sw;

                    sw.start();
                    ce(g, &cr);
                    sw.stop();

                    double seconds = sw.elapsedSeconds();
                    std::cout << formatInt(SyntheticBenchmarks[t].n, 7) << " " << formatDouble(SyntheticBenchmarks[t].p) << " : " << formatInt(cr.cliqueCount(), 10) << " cliques, " << formatInt(cr.recursionCount(), 11) << " calls, " << formatSeconds(seconds) << " seconds" << std::endl;

                    delete g;
                }
        }
        std::cout << "======================================================================" << std::endl;
    }
}
