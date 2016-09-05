
/**
 * Benchmarks
 * This program applies benchmarks for clique enumeration, as reported in literature.
 *
 * @author Kevin A. Naudé
 * @version 1.1
 */

#include <iostream>
#include <string>
#include <string.h>
#include <algorithm>
#include <BitStructures.hpp>
#include <Graph.hpp>
#include <CliqueEnumeration.hpp>
#include <StopWatch.hpp>
#include <GraphLoader.hpp>

using namespace kn;

#ifdef NDEBUG
    class CountingCliqueReceiver : public CliqueReceiver
#else
    class CountingCliqueReceiver : public PrettyPrintCliqueReceiver
#endif
{
public:
    uint64_t count = 0;
    uint64_t size = 0;

    virtual void onClique(const Graph& graph, const IntegerSet& vertices)
    {
        count++;
    }

    virtual void onApply()
    {
        size++;
    }
};

typedef void(*CliqueEnumerator)(const Graph* g, CliqueReceiver* cr);

void test(CliqueEnumerator ce, const Graph& graph)
{
    CountingCliqueReceiver cr;
    StopWatch sw;
    sw.start();
    ce(&graph, &cr);
    sw.stop();

    double seconds = sw.elapsedSeconds();
    std::cout << cr.count << " cliques, " << seconds << " seconds, " << cr.size << " recursive calls" << std::endl;
}

std::vector<std::string> Benchmarks = {
    "C125.9", "C125.9.clq.b",
    "", "",
    "MANN_a9", "MANN_a9.clq.b",
    "brock200_1", "brock200_1.clq.b",
    "brock200_2", "brock200_2.clq.b",
    "brock200_3", "brock200_3.clq.b",
    "brock200_4", "brock200_4.clq.b",
    "", "",
    "c-fat500-5", "c-fat500-5.clq.b",
    "c-fat500-10", "c-fat500-10.clq.b",
    "", "",
    "DSJC1000.1", "DSJC1000.1.col.b",
    "DSJC1000.5", "DSJC1000.5.col.b",
    "DSJC500.1", "DSJC500.1.col.b",
    "DSJC500.5", "DSJC500.5.col.b",
    "", "",
    "hamming8-4", "hamming8-4.clq.b",
    "hamming6-2", "hamming6-2.clq.b",
    "", "",
    "johnson16-2-4", "johnson16-2-4.clq.b",
    "johnson8-4-4", "johnson8-4-4.clq.b",
    "", "",
    "keller4", "keller4.clq.b",
    "", "",
    "p_hat300-1", "p_hat300-1.clq.b",
    "p_hat300-2", "p_hat300-2.clq.b",
    "p_hat500-1", "p_hat500-1.clq.b",
    "p_hat700-1", "p_hat700-1.clq.b",
    "p_hat1000-1", "p_hat1000-1.clq.b",
    "p_hat1500-1", "p_hat1500-1.clq.b",
    "", "",
    "san400_0.5_1", "san400_0.5_1.clq.b",
    "sanr200_0.7", "sanr200_0.7.clq.b",
    "sanr400_0.5", "sanr400_0.5.clq.b"
};

int main(int argc, const char* argv[])
{
    if ((argc < 2) || ((strcmp(argv[1], "tomita-et-al") != 0) && (strcmp(argv[1], "naude") != 0) && (strcmp(argv[1], "segundo-et-al") != 0)))
    {
        std::cout << "usage: program algorithm" << std::endl;
        std::cout << "  e.g. program tomita-et-al" << std::endl;
        std::cout << std::endl;
        std::cout << " tomita-et-al   use Tomita et al. pivot selection" << std::endl;
        std::cout << " naude          use Naude's pivot selection" << std::endl;
        std::cout << " segundo-et-al  use Segundo et al. pivot selection" << std::endl;
        std::cout << std::endl;
        std::cout << "  important note: all required benchmark files must be in the working directory" << std::endl;
    }
    else
    {
    }
}
