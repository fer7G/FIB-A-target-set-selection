import warnings
warnings.filterwarnings("ignore")

import random
import networkx as nx
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation, PillowWriter

# Struct for undirected graph
class Graph:
    def __init__(self, n):
        self.numNodes = n
        self.adjList = [[] for i in range(n)]
        
    def addEdge(self, u, v):
        self.adjList[u].append(v)
        self.adjList[v].append(u)

# Simulate IC diffusion process and return all states for animation
def simulateIC(G, p, S, nx_G, pos):
    # Set of influenced nodes
    influenced = [False] * G.numNodes
    # Set of active nodes (per iteration)
    active = []
    
    # Number of steps
    t = 0
    
    # Store all states for animation
    states = []

    # Order of nodes as stored in the graph, lo necesitamos
    node_order = list(nx_G.nodes())

    # Activate initial set of nodes
    for vertex in S:
        influenced[vertex] = True
        active.append(vertex)

    # Save initial state
    node_colors = ['r' if influenced[node] else 'grey' for node in node_order]
    states.append(node_colors.copy())

    # Continue until no more active nodes
    while active:
        t += 1
        numCurrentActive = len(active)
        # Process all active nodes in the current layer
        for i in range(numCurrentActive):
            v = active.pop(0)
            # Check neighbours of v
            for neighbour in G.adjList[v]:
                # If neighbor is not already active, try to activate it
                if not influenced[neighbour]:
                    r = random.random() # random number between 0 and 1
                    if r < p:
                        influenced[neighbour] = True
                        active.append(neighbour)

        # Save current state
        node_colors = ['r' if influenced[node] else 'grey' for node in node_order]
        states.append(node_colors.copy())

    count = 0
    for node in influenced:
        if node:
            count += 1
    return count, t, influenced, states, node_order

def readInput():
    n, m = map(int, input("Enter number of nodes and number of edges: ").split())
    G = Graph(n)
    print(f"Enter {m} edges:")
    for i in range(m):
        u, v = map(int, input().split())
        G.addEdge(u, v)
    p = float(input("Enter probability for IC model: "))
    s = int(input("Enter number of nodes in the initial seed: "))
    print(f"Enter {s} nodes for the initial seed:")
    S = list(map(int, input().split()))
    return G, p, S

def createDefaultGraph():
    # Example graph with 25 nodes
    G = Graph(25)
    
    # Add edges to create an interesting graph structure
    edges = [
        # Central hub nodes (0-4)
        (0, 1), (0, 2), (0, 3), (0, 4),
        (1, 2), (1, 5), (1, 6),
        (2, 7), (2, 8),
        (3, 9), (3, 10),
        (4, 11), (4, 12),
        # Secondary connections
        (5, 6), (5, 13), (6, 14),
        (7, 8), (7, 15), (8, 16),
        (9, 10), (9, 17), (10, 18),
        (11, 12), (11, 19), (12, 20),
        # Tertiary layer
        (13, 14), (13, 21),
        (15, 16), (15, 22),
        (17, 18), (17, 23),
        (19, 20), (19, 24),
        # Cross connections
        (14, 21), (16, 22), (18, 23), (20, 24),
        (21, 22), (22, 23), (23, 24)
    ]
    
    for u, v in edges:
        G.addEdge(u, v)
    
    # Default probability
    p = 0.7
    
    # Default seed nodes
    S = [0, 1]
    
    return G, p, S

if __name__ == "__main__":
    # Default: Use hardcoded example graph
    G, p, S = createDefaultGraph()
    
    # Alternative: Read input from terminal (commented out)
    # G, p, S = readInput()

    # Convert adjacency list to list of edges
    edge_list = []
    for u in range(G.numNodes):
        for v in G.adjList[u]:
            if (u, v) not in edge_list and (v, u) not in edge_list:
                edge_list.append((u, v))

    # Create NetworkX graph from the edge list
    nx_G = nx.Graph()
    nx_G.add_edges_from(edge_list)

    # Draw the graph
    pos = nx.spring_layout(nx_G, seed=42)  # seed for consistent layout

    # Run simulation and get all states
    C, t, influenced, states, node_order = simulateIC(G, p, S, nx_G, pos)

    print("Size of C:", C)
    print("Value of t:", t)
    print("Generating GIF animation...")

    # Create animation
    fig, ax = plt.subplots(figsize=(12, 8))
    
    def update(frame):
        ax.clear()
        ax.set_title(f"IC Diffusion Simulation - Step {frame}/{len(states)-1}", fontsize=16)
        ax.axis('off')
        
        # Draw edges
        nx.draw_networkx_edges(nx_G, pos, ax=ax, alpha=0.3)
        
        # Draw labels
        nx.draw_networkx_labels(nx_G, pos, ax=ax, font_size=10)
        
        # Draw nodes with colors from current state
        nx.draw_networkx_nodes(nx_G, pos, nodelist=node_order, 
                              node_color=states[frame], 
                              node_size=500, ax=ax)
        
        # Add legend
        from matplotlib.patches import Patch
        legend_elements = [
            Patch(facecolor='red', label='Influenced'),
            Patch(facecolor='grey', label='Not influenced')
        ]
        ax.legend(handles=legend_elements, loc='upper right')
    
    # Create animation
    anim = FuncAnimation(fig, update, frames=len(states), interval=500, repeat=True)
    
    # Save as GIF
    writer = PillowWriter(fps=2)
    anim.save('ic_diffusion_simulation.gif', writer=writer)
    
    print("GIF saved as 'ic_diffusion_simulation.gif'")
    
    # Show the final plot
    update(len(states)-1)
    plt.show()
