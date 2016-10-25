import sys
import os
from shapely.geometry import MultiPoint, Point
from tqdm import tqdm

points = {}
pt_buffer = []
fin = open(sys.argv[1], 'r')
count = 0
for line in fin:
    args = line.split(";")
    points[count] = (float(args[0]), float(args[1]))
    pt_buffer.append(Point(float(args[0]), float(args[1])).buffer(float(sys.argv[2])))
    count += 1

to_unite = {}
set_count = 0
for i in tqdm(range(len(pt_buffer))):
    pt1 = pt_buffer[i]
    for j in range(i+1, len(pt_buffer)):
        pt2 = pt_buffer[j]
        if pt1.intersects(pt2):
            i_set_idx = -1
            j_set_idx = -1
            for k in to_unite.keys():
                k_set = to_unite[k]
                if i in k_set and not j in k_set:
                    if j_set_idx == -1:
                        k_set.add(j)
                        i_set_idx = k
                    else:
                        # Merge k_set and j_set
                        k_set = k_set.union(to_unite[j_set_idx])
                        del to_unite[j_set_idx]
                        i_set_idx = k
                        break

                elif i not in k_set and j in k_set:
                    if i_set_idx == -1: # if i was added before
                        k_set.add(i)
                        j_set_idx = k
                    else:
                        # Merge k_set and i_set
                        k_set = k_set.union(to_unite[i_set_idx])
                        del to_unite[i_set_idx]
                        j_set_idx = k
                        break
                elif i in k_set and j in k_set:
                    # both points are already in the same set
                    i_set_idx = k
                    j_set_idx = k
                    break

            # add the two points
            if i_set_idx == j_set_idx == -1:
                to_unite[set_count] = {i,j}
                set_count += 1

res_points = []
for s in to_unite.values():
    pts = []
    for i in s:
        pts.append(points[i])
        del points[i]
    mp = MultiPoint(pts)
    res_points.append(mp.centroid)

for pt in points.values():
    res_points.append(Point(pt[0], pt[1]))

f_name, f_ext = os.path.splitext(sys.argv[1])
f_name_out = f_name+"-aggregated"+f_ext
with open(f_name_out, 'w') as fout:
    for pt in res_points:
        fout.write("%f;%f;150\n" % (pt.x, pt.y))

print "[DONE] Result written in %s\nReduced from %d to %d" % (f_name_out, count, len(res_points))
