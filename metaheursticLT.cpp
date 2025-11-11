#include <algorithm>
#include <cmath>
#include "difusioLT.cpp"

// Greedy algorithm to select the minimum influence set
Subset greedyMinInfluenceSet(Graph& G, double r) {
    Subset S;
    priority_queue<pair<int,int>> gain;

    // Ordenar el vector de nodos por ganancia marginal
    /*
    for (int i = 0; i < G.numNodes; ++i) {
            Subset s(1, i);
            int t = 0;
            gain.push(make_pair(simulateLT(G, r, s, t), i));
    }*/
    // Ordenar el vector de nodos por grado de aristas
    
    for (int i = 0; i < G.numNodes; ++i) {
            gain.push(make_pair(G.adjList[i].size(), i));
    }
    
    int t;

    // Mientras aún haya nodos con ganancia marginal positiva y difusio(G, p, S) != |V|
    while (!gain.empty()) {
        int node = gain.top().second;
        gain.pop();
        while (G.influenced[node]) {
            node = gain.top().second;
            gain.pop();
        }
        S.push_back(node);
        if (simulateLT(G, r, S, t) == G.numNodes) break;
    }
    return S;
}

// Simulated Annealing algorithm based on gain per node heuristics at every subset S, in order to minimize the number of nodes
Subset simulatedAnnealing(Graph& G, double p, Subset& S, int maxIter, double T, double alpha) {
    
    Subset bestSolution = S;
    int t;
    double bestGain = simulateLT(G, p, S, t) / S.size();

    for (int iter = 0; iter < maxIter && T > 0; ++iter) {
        // Generate random neighbor solution S' erasing or adding a random node
        bool borrar = (double)rand() / RAND_MAX <= 0.75;
        Subset S_prime = S;
        int random_node;
        if (borrar) {
            random_node = S[rand() % S.size()];
            S_prime.erase(find(S_prime.begin(), S_prime.end(), random_node));
        } else {
            random_node =  rand() % G.numNodes;
            while(find(S.begin(), S.end(), random_node)!=S.end()) random_node =  rand() % G.numNodes;
            S_prime.push_back(random_node);
        }

        // Calculate gains per node for S and S'
        double gain_S;
        double propagation = simulateLT(G, p, S_prime, t);
        //cout << iter << "\t"<<S_prime.size() << endl;
        if (propagation != G.numNodes) continue;

        if (iter == 0) gain_S = bestGain;
        else gain_S = simulateLT(G, p, S, t) / S.size();
        double gain_S_prime = propagation / S_prime.size();

        // Calculate acceptance probability
        double delta = gain_S_prime - gain_S;
        double prob = exp(delta / T);

        // If S' is better or accepted with probability, update S
        
        if (delta > 0 or (double)rand() / RAND_MAX < prob) {
            S = S_prime;
            if (gain_S_prime > bestGain) {
                //cout << "Millora en iteració "<< iter << ": Tamany del conjunt:"<<S.size() << endl;
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
    double r = 0.5;

    auto start = high_resolution_clock::now();
    // Compute the subset with the greedy algorithm
    Subset S = greedyMinInfluenceSet(G, r);
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);

    // Greedy solution output
    cout << "Nodos semilla seleccionados en la solución inicial: " << S.size() << " en " << (double)duration.count()/1000 << " s" << endl;

    // Set the parameters for Simulated Annealing
    int maxIter = 15000;
    double T = 100;
    double alpha = 0.99;

    // Improve the initial solution with Simulated Annealing
    auto start_SA = high_resolution_clock::now();
    Subset improved_S = simulatedAnnealing(G, r, S, maxIter, T, alpha);
    auto stop_SA = high_resolution_clock::now();
    auto duration_SA = duration_cast<milliseconds>(stop_SA - start_SA);

    // Simulated Annealing solution output
    cout << "Nodos semilla seleccionados por el simulated annealing: " << improved_S.size() << " en " << (double)duration_SA.count()/1000 << " s" << endl;
}