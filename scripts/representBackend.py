import sys
import matplotlib.pyplot as plt
import networkx as nx

G=nx.Graph()

filename = sys.argv[1]

with open(filename, "r") as f:
	for line in f:
		id1, id2, nb = line.split(" ")
		G.add_edge(id1, id2, weight=nb)


nx.draw(G)
# plt.draw()
plt.savefig("path.png")
print nx.is_connected(G)
