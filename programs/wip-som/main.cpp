
/**
 * wip-som
 * This program is an experimental technique for applying self organising maps to graphs.
 *
 * @author Kevin A. Naudé
 * @version 1.1
 */

#include <time.h>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <Matrix.hpp>
#include <Graph.hpp>
#include <AssignmentSolver.hpp>
#include <GraphSimilarity.hpp>
#include <GraphLoader.hpp>
#include <Random.hpp>
#include <MersenneTwister.hpp>

using namespace kn;

struct Node
{
    float x, y;

    Matrix<float> attributes;
    Matrix<float> probabilities;

    void writeTo(std::ostream& sout)
    {
        sout << x << " " << y << " " << attributes.countColumns() << " " << probabilities.countColumns() << std::endl;
        for (std::size_t i = 0; i < attributes.countColumns(); i++)
        {
            if (i != 0) sout << " ";
            sout << attributes.getValue(0, i);
        }
        sout << std::endl;
        for (std::size_t i = 0; i < probabilities.countColumns(); i++)
        {
            if (i != 0) sout << " ";
            sout << probabilities.getValue(0, i);
        }
        sout << std::endl;
    }

    void readFrom(std::istream& sin)
    {
        std::size_t n, k;
        sin >> x >> y >> n >> k;

        attributes.reshape(1, n);
        probabilities.reshape(1, k);

        for (std::size_t i = 0; i < n; i++)
        {
            float value;
            sin >> value;
            attributes.setValue(0, i, value);
        }
        for (std::size_t i = 0; i < k; i++)
        {
            float value;
            sin >> value;
            probabilities.setValue(0, i, value);
        }
    }

    /*
    void init(Random& random, std::size_t n, std::size_t numGreyLevels, float x, float y)
    {
        this->x = x;
        this->y = y;

        std::size_t k = n + n * (n - 1) / 2;

        attributes.reshape(1, n);
        probabilities.reshape(1, k);

        for (std::size_t i = 0; i < n; i++)
        {
            attributes.setValue(0, i, (float)random.nextDoubleCO() * numGreyLevels);
        }

        for (std::size_t i = 0; i < k; i++)
        {
            probabilities.setValue(0, i, (float)random.nextDoubleCC());
        }
    }
    */

    void init(const Matrix<float>& defaultAttributes, const Matching<float>& mapping, const Graph& g, float x, float y)
    {
        this->x = x;
        this->y = y;

        Graph::Vertex vertex;
        attributes = defaultAttributes;
        std::size_t n = attributes.countColumns();
        std::size_t k = n + n * (n - 1) / 2;
        probabilities.reshape(1, k);

        for (std::size_t i = 0; i < k; i++)
        {
            probabilities.setValue(0, i, 0.0f);
        }

        for (std::size_t c = 0; c < mapping.countPairs(); c++)
        {
            Matching<float>::Pair pair = mapping.getPair(c);
            probabilities.setValue(0, pair.u, 1.0f);
            g.getVertexByIndex(pair.v, vertex);
            std::size_t a = vertex.attrID;
            attributes.setValue(0, pair.u, (float) a);
        }

        for (std::size_t i = 0; i < g.countVertices(); i++)
        {
            std::size_t u = mapping.getU(i);
            for (std::size_t j = i + 1; j < g.countVertices(); j++)
            {
                if (g.hasEdge(i, j))
                {
                    std::size_t v = mapping.getU(i);

                    std::size_t p = std::min(u, v);
                    std::size_t q = std::max(u, v);

                    std::size_t index = n + (p * (n - 1) - p * (p - 1) / 2) + (q - p - 1);
                    probabilities.setValue(0, index, 1.0f);
                }
                k++;
            }
        }
    }

    void init(const Graph& g, std::size_t maxVertices, float x, float y)
    {
        this->x = x;
        this->y = y;

        Graph::Vertex vertex;
        attributes.reshape(1, maxVertices);
        for (std::size_t j = 0; j < maxVertices; j++)
        {
            attributes.setValue(0, j, 0.0f);
        }

        std::size_t n = maxVertices;
        std::size_t k = n + n * (n - 1) / 2;
        probabilities.reshape(1, k);

        for (std::size_t i = 0; i < k; i++)
        {
            probabilities.setValue(0, i, 0.0f);
        }

        for (std::size_t c = 0; c < g.countVertices(); c++)
        {
            probabilities.setValue(0, c, 1.0f);
            g.getVertexByIndex(c, vertex);
            std::size_t a = vertex.attrID;
            attributes.setValue(0, c, (float)a);
        }

        for (std::size_t i = 0; i < g.countVertices(); i++)
        {
            for (std::size_t j = i + 1; j < g.countVertices(); j++)
            {
                if (g.hasEdge(i, j))
                {
                    std::size_t index = n + (i * (n - 1) - i * (i - 1) / 2) + (j - i - 1);
                    probabilities.setValue(0, index, 1.0f);
                }
                k++;
            }
        }
    }

    void blend(const Node& pattern, float alpha)
    {
        attributes.blend(pattern.attributes, alpha);
        probabilities.blend(pattern.probabilities, alpha);
    }
};

class SelfOrganisingMap
{
public:
    static constexpr double SimThreshold = 0.001;

    std::size_t maxVertices;
    Matching<float> mapping;
    BlondelSimilarity sim;

    std::size_t m, n;
    std::vector<std::vector<Node> > grid;
    std::vector<std::vector<int> > assignedClass;
    std::vector<std::vector<std::vector<float> > > classScores;
    std::vector<float> classLow, classHigh;

    std::vector<Graph> training;
    std::vector<int> trainingClass;
    std::vector<Graph> tests;

    int recentClass = 0;

    SelfOrganisingMap(std::size_t m, std::size_t n, std::size_t maxVertices)
    {
        this->m = m;
        this->n = n;
        this->maxVertices = maxVertices;

        // Uncomment to enable greedy assignment algorithm (sub-optimal)
        //sim.setAssignmentSolver(std::unique_ptr<AssignmentSolver<float> >(new GreedyAssignment<float>()));
    }

    void writeTo(std::ostream& sout, std::size_t nextIteration, std::size_t nextInstance, std::size_t numClasses)
    {
        sout << nextIteration << " " << nextInstance << " " << maxVertices << " " << m << " " << n << " " << numClasses << std::endl;
        for (std::size_t i = 0; i < numClasses; i++)
        {
            sout << classLow[i] << " " << classHigh[i] << std::endl;
        }
        for (std::size_t i = 0; i < m; i++)
        {
            for (std::size_t j = 0; j < n; j++)
            {
                if (j != 0) sout << " ";
                sout << std::fixed << std::setw(2) << assignedClass[i][j];
            }
            sout << std::endl;
        }
        for (std::size_t i = 0; i < m; i++)
        {
            for (std::size_t j = 0; j < n; j++)
            {
                grid[i][j].writeTo(sout);
            }
        }
    }

    void readFrom(std::istream& sin, std::size_t& nextIteration, std::size_t& nextInstance, std::size_t& numClasses)
    {
        sin >> nextIteration >> nextInstance >> maxVertices >> m >> n >> numClasses;
        for (std::size_t i = 0; i < numClasses; i++)
        {
            sin >> classLow[i];
            sin >> classHigh[i];
        }
        for (std::size_t i = 0; i < m; i++)
        {
            for (std::size_t j = 0; j < n; j++)
            {
                sin >> assignedClass[i][j];
            }
        }
        for (std::size_t i = 0; i < m; i++)
        {
            for (std::size_t j = 0; j < n; j++)
            {
                grid[i][j].readFrom(sin);
            }
        }
    }

    void loadTraining(std::string path, std::size_t firstTheta, std::size_t stepTheta, std::size_t limitTheta, std::size_t firstObj, std::size_t limitObj, bool append = false)
    {
        if (!append)
        {
            training.clear();
            trainingClass.clear();
        }

        for (std::size_t obj = firstObj; obj <= limitObj; obj++)
        {
            for (std::size_t theta = firstTheta; theta <= limitTheta; theta += stepTheta)
            {
                std::ostringstream ss;
                ss << path << "\\obj" << obj << "__" << theta << ".png.txt";
                GraphLoader gl(ss.str());
                if (gl.isOpen())
                {
                    Graph g;
                    gl.loadAttributedDIMACS(g);
                    training.push_back(std::move(g));
                    trainingClass.push_back(obj);
                }
            }
        }
    }

    void loadTests(std::string path, std::size_t firstTheta, std::size_t stepTheta, std::size_t limitTheta, std::size_t firstObj, std::size_t limitObj, bool append = false)
    {
        if (!append)
        {
            tests.clear();
        }

        for (std::size_t obj = firstObj; obj <= limitObj; obj++)
        {
            for (std::size_t theta = firstTheta; theta <= limitTheta; theta += stepTheta)
            {
                std::ostringstream ss;
                ss << path << "\\obj" << obj << "__" << theta << ".png.txt";
                GraphLoader gl(ss.str());
                if (gl.isOpen())
                {
                    Graph g;
                    gl.loadAttributedDIMACS(g);
                    tests.push_back(std::move(g));
                }
            }
        }
    }

    void initGrid(Random& random, int instancesPerClass, int numClasses)
    {
        Matrix<float> clearAttributes;
        clearAttributes.reshape(1, maxVertices);
        for (std::size_t j = 0; j < maxVertices; j++)
        {
            clearAttributes.setValue(0, j, 0.0f);
        }

        grid.clear();
        assignedClass.clear();
        for (std::size_t i = 0; i < m; i++)
        {
            grid.push_back(std::vector<Node>());
            assignedClass.push_back(std::vector<int>());

            std::vector<Node>* vec = &grid[i];
            std::vector<int>* veci = &assignedClass[i];
            for (std::size_t j = 0; j < n; j++)
            {
                uint32_t index = random.nextUInt((uint32_t)training.size());
                Graph* g = &training[index];
                Node node;
                vec->push_back(node);
                (*vec)[j].init(*g, maxVertices, i, j);
                veci->push_back(index / instancesPerClass + 1);
            }
        }

        classScores.clear();
        classLow.clear();
        classHigh.clear();
        for (int k = 1; k <= numClasses; k++)
        {
            classLow.push_back(0.0f);
            classHigh.push_back(0.0f);
            classScores.push_back(std::vector<std::vector<float> >());
            std::vector<std::vector<float> >* group = &classScores[k - 1];
            for (std::size_t i = 0; i < m; i++)
            {
                group->push_back(std::vector<float>());

                std::vector<float>* vec = &(*group)[i];
                for (std::size_t j = 0; j < n; j++)
                {
                    vec->push_back(0.0f);
                }
            }
        }
    }

    void build(Random& random, const Node& node, Graph& g)
    {
        g.clear();
        std::size_t n = node.attributes.countColumns();
        mapping.clear(n, n);

        for (std::size_t i = 0; i < n; i++)
        {
            if (random.nextDoubleCO() < node.probabilities.getValue(0, i))
            {
                std::size_t a = (std::size_t)std::round(node.attributes.getValue(0, i));
                std::size_t v = g.addVertex(a);
                mapping.add(i, v, 0.0f);
            }
        }

        std::size_t k = n;
        for (std::size_t i = 0; i < n; i++)
        {
            if (!mapping.isMappedU(i))
            {
                k += (n - i - 1);
            }
            else
            {
                for (std::size_t j = i + 1; j < n; j++)
                {
                    if (mapping.isMappedU(j))
                    {
                        if (random.nextDoubleCO() < node.probabilities.getValue(0, k))
                        {
                            g.addEdge(mapping.getU(i), mapping.getU(j), 0);
                        }
                    }
                    k++;
                }
            }
        }
    }

    void build(const Node& node, Graph& g, float vertThreshold, float edgeThreshold)
    {
        g.clear();
        std::size_t n = node.attributes.countColumns();
        mapping.clear(n, n);

        for (std::size_t i = 0; i < n; i++)
        {
            if (vertThreshold <= node.probabilities.getValue(0, i))
            {
                std::size_t a = (std::size_t)std::round(node.attributes.getValue(0, i));
                std::size_t v = g.addVertex(a);
                mapping.add(i, v, 0.0f);
            }
        }

        std::size_t k = n;
        for (std::size_t i = 0; i < n; i++)
        {
            if (!mapping.isMappedU(i))
            {
                k += (n - i - 1);
            }
            else
            {
                for (std::size_t j = i + 1; j < n; j++)
                {
                    if (mapping.isMappedU(j))
                    {
                        if (edgeThreshold <= node.probabilities.getValue(0, k))
                        {
                            g.addEdge(mapping.getU(i), mapping.getU(j), 0);
                        }
                    }
                    k++;
                }
            }
        }
    }

    float compare(const Graph& nodeInstance, const Graph& trainingInstance)
    {
        sim.solve(mapping, nodeInstance, trainingInstance, SimThreshold);
        return mapping.meanScore();
    }

    float compare(Random& random, const Node& node, const Graph& trainingInstance, std::size_t count = 1)
    {
        float sum = 0.0f;
        for (std::size_t c = 0; c < count; c++)
        {
            Graph g;
            //build(random, node, g);
            build(node, g, 0.5f, 0.5f);
            sim.solve(mapping, g, trainingInstance, SimThreshold);
            sum += mapping.meanScore();
        }
        return sum / count;
    }

    void buildBlendNode(Random& random, Node& pattern, const Node& node, const Graph& trainingInstance)
    {
        Graph nodeInstance;
        build(random, node, nodeInstance);
        sim.solve(mapping, nodeInstance, trainingInstance, SimThreshold);
        pattern.init(node.attributes, mapping, trainingInstance, node.x, node.y);
    }

    void blend(Random& random, Node& node, const Graph& trainingInstance, float alpha)
    {
        Node tempNode;
        buildBlendNode(random, tempNode, node, trainingInstance);
        node.blend(tempNode, alpha);
    }

    void computeHighLow(int k)
    {
        float low = 1.0f;
        float high = 0.0f;
        for (std::size_t i = 0; i < m; i++)
        {
            for (std::size_t j = 0; j < n; j++)
            {
                float score = classScores[k - 1][i][j];
                low = std::min(low, score);
                high = std::max(high, score);
            }
        }
        classLow[k - 1] = low;
        classHigh[k - 1] = high;
    }

    float normScore(int k, float score)
    {
        float diff = classHigh[k - 1] - classLow[k - 1];
        if (diff > 0.0f)
        {
            float result = (score - classLow[k - 1]) / diff;
            return result;
        }
        else
        {
            return score;
        }
    }

    void train(Random& random, float eta, float sigma, float maxDist, int instancesPerClass)
    {
        int numClasses = training.size() / instancesPerClass;
        uint32_t index = random.nextUInt((uint32_t)training.size());
        int goal = index / instancesPerClass + 1;
        recentClass = goal;
        float localAlpha = eta;
        localAlpha = 1.0f;

        Graph* g = &training[index];
        std::size_t bestI = 0, bestJ = 0;
        float bestSim = -1.0f;
        for (std::size_t i = 0; i < m; i++)
        {
            for (std::size_t j = 0; j < n; j++)
            {
                float sim = compare(random, grid[i][j], *g, 1);
                classScores[goal-1][i][j] = classScores[goal-1][i][j] * (1.0f - localAlpha) + sim * localAlpha;
                if (sim > bestSim)
                {
                    bestI = i;
                    bestJ = j;
                    bestSim = sim;
                }
            }
        }

        //assignedClass[bestI][bestJ] = goal;
        bool blendCommon = false;
        float maxDist2 = maxDist * maxDist;

        Node* best = &grid[bestI][bestJ];
        if (blendCommon)
        {
            Node blendNode;
            buildBlendNode(random, blendNode, *best, *g);
            for (std::size_t i = 0; i < m; i++)
            {
                for (std::size_t j = 0; j < n; j++)
                {
                    Node* cur = &grid[i][j];
                    float dx = cur->x - best->x;
                    float dy = cur->y - best->y;
                    float dist2 = dx * dx + dy * dy;
                    if (dist2 <= maxDist2)
                    {
                        float influence = std::exp(-dist2 / (2.0f * sigma * sigma));
                        float alpha = eta * influence;

                        (*cur).blend(blendNode, alpha);
                    }
                }
            }
        }
        else
        {
            Node blendNode;
            for (std::size_t i = 0; i < m; i++)
            {
                for (std::size_t j = 0; j < n; j++)
                {
                    Node* cur = &grid[i][j];
                    float dx = cur->x - best->x;
                    float dy = cur->y - best->y;
                    float dist2 = dx * dx + dy * dy;
                    if (dist2 <= maxDist2)
                    {
                        float influence = std::exp(-dist2 / (2.0f * sigma * sigma));
                        float alpha = eta * influence;

                        Node* toBlend = &grid[i][j];
                        buildBlendNode(random, blendNode, *toBlend, *g);
                        (*cur).blend(blendNode, alpha);
                    }
                }
            }
        }
        updateStats(numClasses, goal);
    }

    void updateStats(int numClasses, int goalClass = -1)
    {
        if (goalClass >= 0)
        {
            computeHighLow(goalClass);
        }
        else
        {
            for (int goal = 1; goal <= numClasses; goal++)
            {
                computeHighLow(goal);
            }
        }
        for (std::size_t i = 0; i < m; i++)
        {
            for (std::size_t j = 0; j < n; j++)
            {
                int bestK = 1;
                float bestKScore = normScore(1, classScores[0][i][j]);
                for (int k = 2; k <= numClasses; k++)
                {
                    float altScore = normScore(k, classScores[k - 1][i][j]);
                    if (altScore > bestKScore)
                    {
                        bestK = k;
                        bestKScore = altScore;
                    }
                }
                assignedClass[i][j] = bestK;
            }
        }
    }

    int test(Random& random, const Graph& testInstance, int count = 10)
    {
        std::size_t bestI = 0, bestJ = 0;
        float bestSim = -1.0f;
        for (std::size_t i = 0; i < m; i++)
        {
            for (std::size_t j = 0; j < n; j++)
            {
                float sim = compare(random, grid[i][j], testInstance, count);
                //sim = normScore(assignedClass[i][j], sim);
                if (sim > bestSim)
                {
                    bestI = i;
                    bestJ = j;
                    bestSim = sim;
                }
            }
        }
        return assignedClass[bestI][bestJ];
    }

    float testClass(Random& random, int k, int testsPerClass, std::ofstream& sout, int count = 10)
    {
        int good = 0;
        int total = 0;
        for (int j = 0; j < testsPerClass; j++)
        {
            int altK = test(random, tests[(k - 1) * testsPerClass + j], count);
            //std::cout << "* " << altK << std::endl;
            if (altK == k) good++;
            total++;
            if (j != 0) sout << ",";
            sout << altK;
        }
        sout << std::endl;
        return (float)good / total;
    }

    void check(Random& random, int numClasses, int count = 10)
    {
        for (int goal = 1; goal <= numClasses; goal++)
        {
            for (std::size_t i = 0; i < m; i++)
            {
                for (std::size_t j = 0; j < n; j++)
                {
                    classScores[goal - 1][i][j] = 0.0f;
                }
            }
        }
        for (int k = 0; k < count; k++)
        {
            for (int goal = 1; goal <= numClasses; goal++)
            {
                uint32_t numCases = (uint32_t)training.size() / 10;
                uint32_t index = (goal - 1) * numCases + random.nextUInt(numCases);

                Graph* g = &training[index];
                for (std::size_t i = 0; i < m; i++)
                {
                    for (std::size_t j = 0; j < n; j++)
                    {
                        float sim = compare(random, grid[i][j], *g);
                        classScores[goal - 1][i][j] += sim;
                    }
                }
            }
        }
        for (int goal = 1; goal <= numClasses; goal++)
        {
            for (std::size_t i = 0; i < m; i++)
            {
                for (std::size_t j = 0; j < n; j++)
                {
                    classScores[goal - 1][i][j] /= (float)count;
                }
            }
        }
        updateStats(numClasses);
    }

    void report(std::size_t iteration, std::size_t k, std::ostream& sout)
    {
        sout << iteration << ": " << k << " [" << recentClass << "]" << std::endl;
        for (std::size_t i = 0; i < m; i++)
        {
            for (std::size_t j = 0; j < n; j++)
            {
                sout << " " << std::fixed << std::setw(2) << assignedClass[i][j];
            }
            sout << std::endl;
        }
        sout << std::endl;
    }

    void reportDetail(int numClasses, std::ostream& sout)
    {
        for (std::size_t i = 0; i < m; i++)
        {
            for (int k = 1; k <= numClasses; k++)
            {
                for (std::size_t j = 0; j < n; j++)
                {
                    float score = normScore(k, classScores[k - 1][i][j]);
                    int x = (int) std::min(99.0f, std::round(score * 100));
                    sout << " " << std::fixed << std::setw(2) << x;
                }
                sout << " ";
            }
            sout << std::endl;
        }
        sout << std::endl;
    }
};

int main(int argc, const char* argv[])
{
    MersenneTwister random((uint32_t) time(NULL));
    SelfOrganisingMap som(9, 9, 20);

    int instancesPerClass = 18;
    int numClasses = 10;
    int numChecks = 1;

    std::cout << "Loading Training" << std::endl;
    som.loadTraining("Graphs", 0, 20, instancesPerClass * 20 - 1, 1, numClasses);
    std::cout << "Done Loading Training: " << som.training.size() << std::endl;

    std::cout << "Initing" << std::endl;
    som.initGrid(random, instancesPerClass, numClasses);
    std::cout << "Done Initing " << som.m << "x" << som.n << std::endl;

    std::size_t firstIteration = 0;
    std::size_t firstInstance = 0;

    std::ifstream istream;
    istream.open("som.txt");
    if (istream.is_open())
    {
        std::size_t providedNumClasses;
        som.readFrom(istream, firstIteration, firstInstance, providedNumClasses);
        istream.close();
    }

    std::size_t MaxIterations = 1000;
    float MaxSigma = std::min(som.m, som.n) * 0.25f;
    float MaxEta = 0.5f;
    float MinEta = 0.001f;
    float MaxDist = 2.5f;

    float scaleSigma = (std::log(MaxSigma) / std::log(2.0f)) / (MaxIterations/2-1);
    float scaleEta = std::log(MaxEta / MinEta) / (MaxIterations-1);

    if (firstIteration < MaxIterations)
    {
        std::ios_base::openmode openMode = std::ofstream::out | std::ofstream::app;
        if ((firstIteration == 0) && (firstInstance == 0))
        {
            openMode = std::ofstream::out | std::ofstream::trunc;
        }
        else
        {
            som.check(random, numClasses, numChecks);
        }

        std::ofstream file("results.txt", openMode);
        std::ofstream file20("results20.txt", openMode);


        for (std::size_t iteration = firstIteration; iteration < MaxIterations; iteration++)
        {
            float sigma = MaxSigma * std::exp(-(float)iteration * scaleSigma);
            float eta = MaxEta * std::exp(-(float)iteration * scaleEta);
            for (std::size_t k = firstInstance; k < som.training.size(); k++)
            {
                firstInstance = 0;
                som.train(random, eta, sigma, MaxDist, instancesPerClass);

                if (k + 1 == som.training.size())
                {
                    if ((((iteration + 1) % 20) == 0) && (k + 1 == som.training.size()))
                    {
                        som.check(random, numClasses, numChecks);
                        som.report(iteration + 1, k + 1, file20);
                        //som.reportDetail(numClasses, file20);
                    }
                    som.report(iteration + 1, k + 1, file);
                    som.report(iteration + 1, k + 1, std::cout);
                    //som.reportDetail(numClasses, std::cout);
                }

                std::ofstream ostream;
                ostream.open("som.txt", std::ofstream::out | std::ofstream::trunc);
                if (ostream.is_open())
                {
                    std::size_t nextInstance = k + 1;
                    std::size_t nextIteration = iteration;
                    if (nextInstance >= som.training.size())
                    {
                        nextInstance = 0;
                        nextIteration++;
                    }
                    som.writeTo(ostream, nextIteration, nextInstance, numClasses);
                    ostream.close();
                }
            }
        }

        som.report(MaxIterations, 0, file);
    }

    std::cout << "Loading Testing" << std::endl;
    for (int k = 1; k <= numClasses; k++)
    {
        som.loadTests("Graphs", 5, 20, 359, k, k, true);
        som.loadTests("Graphs", 10, 20, 359, k, k, true);
        som.loadTests("Graphs", 15, 20, 359, k, k, true);
    }
    std::size_t testsPerClass = som.tests.size() / numClasses;
    std::cout << "Done Loading Testing: " << som.tests.size() << std::endl;
    std::cout << "Test Instances per class: " << testsPerClass << std::endl;

    //som.check(random, numClasses, 100);
    std::ofstream testResults;
    testResults.open("tests.txt", std::ofstream::out | std::ofstream::trunc);
    if (testResults.is_open())
    {
        //som.writeTo(ostream2, firstIteration, firstInstance, numClasses);
        for (int k = 1; k <= numClasses; k++)
        {
            float recRate = som.testClass(random, k, testsPerClass, testResults, 40);
            std::cout << k << ": " << recRate << std::endl;
        }
        testResults.close();
    }

    //std::string t;
    //std::cin >> t;

}
