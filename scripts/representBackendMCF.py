import sys
import os
import matplotlib.pyplot as plt
import networkx as nx
import re

G = nx.Graph()

REGEX = r"(.*?)adjacencyMatrix\-(\d+)snodes"

filename = sys.argv[1]
f_name, f_ext = os.path.splitext(filename)
m = re.match(REGEX, f_name)
if m: 
	f_name_out = m.group(1)+"distributionTree-"+m.group(2)+"snodes"+f_ext
else:
	f_name_out = f_name+"-out"+f_ext

f_pos_name = f_name+"-pos"+f_ext

pos_dict = {}
with open(f_pos_name, "r") as f:
	for line in f:
		snodeId, X, Y = line.split(" ")
		pos_dict[snodeId] = (float(X), float(Y))

max_weight = 0
with open(filename, "r") as f:
	for line in f:
		id1, id2, conFrom, conTo, travelTimeAvg, travelTimeStd, travelTimePercentile, nb, interVisit, groupIDs = line.strip().split(" ")

		if int(nb) == 0 or float(interVisit) == 0.0:
			continue

		groups = groupIDs[1:-1].split(",")
		weight = 1/(float(travelTimeAvg) + float(interVisit))
		
		G.add_edge(id1, id2, weight=weight, groups=groups)

		if weight > max_weight:
			max_weight = weight


path = nx.all_pairs_dijkstra_path(G)

distribution_tree = {}
for node_s in path.keys():
	if node_s not in distribution_tree:
		distribution_tree[node_s] = {}

	for p in path[node_s].values():
		if len(p) <= 1:
			continue

		for i in range(len(p)-1):
			node_a = p[i]
			node_b = p[i+1]

			if node_a not in distribution_tree[node_s]:
				distribution_tree[node_s][node_a] = {}
			
			distribution_tree[node_s][node_a][node_b] = G[node_a][node_b]['groups']

with open(f_name_out, 'w') as f:
	for node_s in distribution_tree.keys():
		for node_a in distribution_tree[node_s].keys():
			for node_b, g in distribution_tree[node_s][node_a].items():
				f.write("%s %s %s\n" % (node_s, node_a, node_b))

edges  = G.edges()
pos    = dict([(u, [pos_dict[u][0], -1*pos_dict[u][1]]) for u in G.nodes()])
labels = dict([(u, u) for u in G.nodes()])
widths = [1+3*G[u][v]['weight']/max_weight for u,v in edges]

nx.draw(G, pos=pos, width=widths, labels=labels, node_size=15)

# plt.draw()
plt.savefig(f_name+".png")
print "Exported "+f_name+".png"
print nx.is_connected(G)


def getMobileNodeTour(l):
	if len(l) < 6:
		print("not long enough")
		return []

	uni = set(l)
	for e in l[:3]:
		if e in uni:
			uni.remove(e)

	for i in range(3,len(l)-2):
		if l[i] in uni:
			uni.remove(l[i])

		print(i,l[i],uni, l[0] == l[i], l[1] == l[i+1], l[2] == l[i+2])

		if len(uni) == 0 and l[0] == l[i] and l[1] == l[i+1] and l[2] == l[i+2]:
			print("hello")
			return l[0:i]

def getNbConsecutiveElements(l):
	maxNb = 0
	nbConsecutiveElements = 1
	prev = l[0]
	for i in range(1,len(l)):
		if prev == l[i]: 
			nbConsecutiveElements += 1
		else:
			if nbConsecutiveElements > maxNb:
				maxNb = nbConsecutiveElements
			nbConsecutiveElements = 1
		prev = l[i]
	return maxNb

def getMobileNodeNextVisit(l, prev):
	lCont = l+l
	for i in range(1,len(lCont)-1):
		if prev[0] == lCont[i-1] and prev[1] == lCont[i]:
			return lCont[i+1]

