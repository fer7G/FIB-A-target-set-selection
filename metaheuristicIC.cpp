#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <utility>
#include <set>
#include <ctime>
#include <chrono>
using namespace std;
using namespace std::chrono;
typedef pair<int, int> pii;

// Struct for undirected graph
struct Graph {
    int numNodes;
    vector<vector<int> > adjList;

    // Graph constructor:
    Graph(int n) {
        numNodes = n;
        adjList.resize(n);
    }

    Graph() {
    }

    void addEdge(int u, int v) {
        adjList[u].push_back(v);
        adjList[v].push_back(u);
    }

    void printEdges() {
        for (int vertex = 0; vertex < adjList.size(); ++vertex) {
            for (int neighbour : adjList[vertex]) cout << vertex << " -> " << neighbour << endl;
        }
    }
};

// Simulate IC difusion process
int difusioIC(const Graph& G, double& p, set<int>& S) {
    // Set of influenced nodes
    vector<bool> influenced(G.numNodes, false);
    // Set of active nodes (nodes able to activate other nodes per iteration)
    queue<int> active;

    int n_influenced = 0;
    
    int t = 0;

    // Activate initial set of nodes
    for (int vertex : S) {
        influenced[vertex] = true;
        ++n_influenced;
        active.push(vertex);
    }

    // Continue untill no more active nodes
    while (not active.empty()) {
        ++t;
        int numCurrentActive = active.size();
        // Process all active nodes in the current layer
        for (int i = 0; i < numCurrentActive; ++i) {
            int v = active.front();
            active.pop();
            // Check neighbours of v
            for (int j = 0; j < G.adjList[v].size(); ++j) {
                int neighbour = G.adjList[v][j];
                 // If neighbor is not already active, try to activate it
                if (not influenced[neighbour]) {
                    double r = (double) rand() / RAND_MAX;
                    if (r < p) {
                        influenced[neighbour] = true;
                        ++n_influenced;
                        active.push(neighbour);
                    }
                }
            }
        }
    }

    // Count all influenced nodes by S
/*     int count = 0;
    for (int i = 0; i < influenced.size(); ++i) if (influenced[i]) ++count;
    return count; */
    return n_influenced;
}

// Comparer for the priority queue
struct Compare {
    bool operator()(const pii& a, const pii& b) {
        return a.second < b.second;
    }
};

// Simulate the IC model several times for more accuracy
int monteCarlo(const Graph& G, double p, set<int>& S, int iterations) {
    int sum = 0;
    for (int i = 0; i < iterations; ++i) {
        sum += difusioIC(G, p, S);
    }
    return sum / iterations;
}

// Greedy algorithm to select the minimum influence set
set<int> greedyMinInfluenceSet(const Graph& G, double p, int nMonteCarlo, double optimality) {
    set<int> S;
    vector<pair<int, int> > gain;
    int numNodes = G.numNodes;
    priority_queue<pii, vector<pii>, Compare> Q;

    // Calcular la ganancia marginal de todos los nodos e ir insertando en la cola de prioridad
    for (int i = 0; i < G.numNodes; ++i) {
        set<int> single_node_set;
        single_node_set.insert(i);
        int gain_val = monteCarlo(G, p, single_node_set, nMonteCarlo);
        Q.push(make_pair(i, gain_val));
    }

    // Añadir el primer nodo de la cola Q a S y quitarlo de la cola
    S.insert(Q.top().first);
    Q.pop();

    // Calcular la difusión de S
    int diffusion = monteCarlo(G, p, S, nMonteCarlo);

    // Mientras la cola Q no esté vacía y difusio(G, p, S) != |V|
    while (!Q.empty()) {
        // If current difussion is OK, break
        if (diffusion >= optimality * numNodes) break;

        int current_node = Q.top().first;

        // Añadir el primer nodo de la cola Q a S y quitarlo de la cola, si el nodo no está en S
        if (S.find(current_node) == S.end()) {
            S.insert(current_node);
            diffusion = monteCarlo(G, p, S, nMonteCarlo);
        }
        Q.pop();
    }

    return S;
}

// Read a Graph (adapted for dimacs files)
Graph readGraph() {
    char p;
    string edge;
    cin >> p >> edge;
    int n, m;
    cin >> n >> m;

    Graph G(n);

    char e;
    int u, v;
    for (int i = 0; i < m; ++i) {
        cin >> e >> u >> v;
        G.addEdge(u-1, v-1);
    }

    return G;
}

// Simulated Annealing algorithm based on gain per node heuristics at every subset S, in order to minimize the number of nodes
set<int> simulatedAnnealing(const Graph& G, double p, set<int>& S, int nMonteCarlo, double optimality, int maxIter, double T, double alpha) {
    set<int> bestSolution = S;
    int numNodes = G.numNodes;
    double bestGain = monteCarlo(G, p, S, nMonteCarlo) / S.size();

    for (int iter = 0; iter < maxIter; ++iter) {
        // Generate random neighbor solution S' erasing or adding a random node
        set<int> S_prime = S;
        int random_node = rand() % numNodes;
        if (S_prime.count(random_node) > 0) {
            S_prime.erase(random_node);
        } else {
            S_prime.insert(random_node);
        }

        // Calculate gains per node for S and S'
        double gain_S;
        if (iter == 0) gain_S = bestGain;
        else gain_S = monteCarlo(G, p, S, nMonteCarlo) / S.size();
        double gain_S_prime = monteCarlo(G, p, S_prime, nMonteCarlo) / S_prime.size();

        // Calculate acceptance probability
        double delta = gain_S_prime - gain_S;
        double prob = 1.0 / exp(delta / T);

        // If S' is better or accepted with probability, update S
        if (delta > 0 or (double)rand() / RAND_MAX < prob) {
            S = S_prime;
            if (gain_S_prime > bestGain) {
                bestSolution = S_prime;
                bestGain = gain_S_prime;
            }
        }

        // Decrease temperature
        T *= alpha;
    }

    return bestSolution;
}

int main() {
    
    // Generate random seed for proper random values
    unsigned seed = chrono::high_resolution_clock::now().time_since_epoch().count();
    srand(seed);

    // Declare and read the graph G = (V, E)
    Graph G = readGraph();

    // Set the probability for the IC model
    double p = 0.5;

    // Set MonteCarlo iterations i.e. number of times the simulation will be executed each time (more iterations -> slower but more accurate)
    int nMonteCarlo = 10;

    // Set the optimality, e. g. if optimality is 0.9 we will obtain a subset S whose MonteCarlo simulation warrantees covering 90% of the graph
    double optimality = 1;

    auto start = high_resolution_clock::now();
    // Compute the subset with the greedy algorithm
    set<int> S = greedyMinInfluenceSet(G, p, nMonteCarlo, optimality);
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);

    // Greedy solution output
    cout << "Seed nodes selected in the initial solution: " << S.size() << " in " << (double)duration.count()/1000 << " s" << endl;

    // Set the parameters for Simulated Annealing
    int maxIter = 1000;
    double T = 100;
    double alpha = 0.99;

    // Improve the initial solution with Simulated Annealing
    auto start_SA = high_resolution_clock::now();
    set<int> improved_S = simulatedAnnealing(G, p, S, nMonteCarlo, optimality, maxIter, T, alpha);
    auto stop_SA = high_resolution_clock::now();
    auto duration_SA = duration_cast<milliseconds>(stop_SA - start_SA);

    // Simulated Annealing solution output
    cout << "Seed nodes selected by simulated annealing: " << S.size() << " in " << (double)duration_SA.count()/1000 << " s" << endl;
}
