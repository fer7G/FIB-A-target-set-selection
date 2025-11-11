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
    for (int i = 0; i < numNodes; ++i) {
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

// Local search algorithm for first improvement, based on number of nodes heuristics at every subset S
// We pick the first successor state which reduces S by 1 (obviously) and the state is still a solution
void localSearch(const Graph& G, double p, set<int>& S, int nMonteCarlo, double optimality) {
    bool improvement = true;
    
    while (improvement) {
        improvement = false;
        int currentInfluence = monteCarlo(G, p, S, nMonteCarlo);
        for (int node : S) {
            set<int> tempS(S);
            tempS.erase(node);
            // int currentInfluence = monteCarlo(G, p, S, nMonteCarlo);
            int tempInfluence = monteCarlo(G, p, tempS, nMonteCarlo);
            
            if (tempInfluence >= currentInfluence && tempInfluence >= optimality * G.numNodes) {
                improvement = true;
                S = tempS;
                break;
            }
        }
    }
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

int main() {
    
    // Generate random seed for proper random values
    unsigned seed = chrono::high_resolution_clock::now().time_since_epoch().count();
    srand(seed);

    // Declare and read the graph G = (V, E)
    Graph G = readGraph();

    // Set the probability for the IC model
    double p = 0.5;

    // Set MonteCarlo iterations i.e. number of times the simulation will be executed each time (more iterations -> slower but more accurate)
    int nMonteCarlo = 100;

    // Set the optimality, e. g. if optimality is 0.9 we will obtain a subset S whose MonteCarlo simulation warrantees covering 90% of the graph
    double optimality = 1;

    auto start = high_resolution_clock::now();
    // Compute the subset with the greedy algorithm
    set<int> S = greedyMinInfluenceSet(G, p, nMonteCarlo, optimality);
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);

    // Greedy solution output
    cout << "Seed nodes selected in the initial solution: " << S.size() << " in " << (double)duration.count()/1000 << " s" << endl;

    start = high_resolution_clock::now();
    // Compute a localSearch approach from the greedy solution
    localSearch(G, p, S, nMonteCarlo, optimality);
    stop = high_resolution_clock::now();
    duration = duration_cast<milliseconds>(stop - start);

    // Local search solution output
    cout << "Seed nodes selected by local search: " << S.size() << " in " << (double)duration.count()/1000 << " s" << endl;
}
