#include "difusioLT.cpp"

Subset greedyMinInfluenceSet(Graph& G, double r) {
    Subset S;
    priority_queue<pair<int,int>> gain;

    // Ordenar el vector de nodos por ganancia marginal
    for (int i = 0; i < G.numNodes; ++i) {
            Subset s(1, i);
            int t = 0;
            gain.push(make_pair(simulateLT(G, r, s, t), i));
    }
    // Ordenar el vector de nodos por grado de aristas
    /*
    for (int i = 0; i < G.numNodes; ++i) {
            gain.push(make_pair(G.adjList[i].size(), i));
    }
    */
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

int main () {
    unsigned seed = chrono::high_resolution_clock::now().time_since_epoch().count();
    Graph G= readGraph();
    double r = 0.5;

    auto start = high_resolution_clock::now();
    Subset S = greedyMinInfluenceSet(G,r);
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);

    //printSet(S);
    cout << "Selected seed nodes:" << S.size()<<endl;
    cout << " in " << (double)duration.count()/1000 << " s"<<endl;
}