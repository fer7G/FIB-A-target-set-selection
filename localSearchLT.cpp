#include <algorithm>
#include "difusioLT.cpp"

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


// Local search algorithm for best improvement
void localSearch(Graph& G, double r, Subset& S) {
    bool improvement = true;
    
    while (improvement) {
        improvement = false;
        for (int node : S) {
            Subset tempS(S);
            tempS.erase(find(tempS.begin(), tempS.end(), node));
            int t0,t1;
            int currentInfluence = simulateLT(G, r, S, t0);
            int tempInfluence = simulateLT(G, r, tempS, t1);
            
            if (tempInfluence == currentInfluence && tempS.size() < S.size()) {
                improvement = true;
                S = tempS;
            }
        }
    }
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
    cout << "Seed nodes selected in the initial solution: " << S.size() << " in " << (double)duration.count()/1000 << " s" << endl;

    start = high_resolution_clock::now();
    // Compute a localSearch approach from the greedy solution
    localSearch(G, r, S);
    stop = high_resolution_clock::now();
    duration = duration_cast<milliseconds>(stop - start);

    // Local search solution output
    cout << "Seed nodes selected by local search: " << S.size() << " in " << (double)duration.count()/1000 << " s" << endl;
}