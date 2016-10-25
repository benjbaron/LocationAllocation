import sys
import os
import re

path = sys.argv[1]
points = set()


for filename in os.listdir(path):
	if("bus" in filename):
		print "filename:", filename
		with open(path+filename, 'r') as f:
			for line in f:
				m = re.search('^LINESTRING\s?\((.*)\)$', line)
				for pt in m.group(1).split(","):
					print pt
					(x,y) = pt.strip().split(" ")
					points.add((x,y))

with open("points-out.csv", 'w') as f:
	for pt in points:
		f.write("%s;%s;150\n" % (pt[0], pt[1]))

