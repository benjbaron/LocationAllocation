from matplotlib import pyplot
from shapely.geometry import Point
from descartes import PolygonPatch
import sys

BLUE = "#6699cc"
GRAY = "#cecece"
SIZE = 10,10

fig = pyplot.figure(1, figsize=SIZE, dpi=90)

ax = fig.add_subplot(111)

fin = open(sys.argv[1], 'r')
for line in fin:
    args = line.split(";")
    pt = Point(float(args[0]), float(args[1])).buffer(float(sys.argv[2]))
    patch = PolygonPatch(pt, fc=GRAY, ec=GRAY, alpha=0.7, zorder=1)
    ax.add_patch(patch)

# 1
xrange = [-10, 100]
yrange = [-10, 100]
ax.set_xlim(*xrange)
ax.set_xticks(range(*xrange) + [xrange[-1]])
ax.set_ylim(*yrange)
ax.set_yticks(range(*yrange) + [yrange[-1]])
ax.set_aspect(1)

pyplot.show()
