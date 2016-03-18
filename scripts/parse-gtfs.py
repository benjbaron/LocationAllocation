import sys
import operator
from collections import OrderedDict
import os

def to_time(str):
    units = str.split(":")
    return 3600*int(units[0])+60*int(units[1])+int(units[2])

gtfs_folder     = sys.argv[1]
gtfs_output_folder = gtfs_folder+"-out"

trips_output_file      = gtfs_output_folder + "/trips.txt"
stop_times_output_file = gtfs_output_folder + "/stop_times.txt"
routes_output_files    = gtfs_output_folder + "/routes.txt"
shapes_output_files    = gtfs_output_folder + "/shapes.txt"

trips_file      = gtfs_folder + "/trips.txt"
stop_times_file = gtfs_folder + "/stop_times.txt"
routes_files    = gtfs_folder + "/routes.txt"
shapes_files    = gtfs_folder + "/shapes.txt"

routes = {} # route_id: set(trip_id)
trips  = {} # trip_id: [direction_id, start_time, end_time, shape_id]
with open(routes_files, "r") as f:
    next(f)
    for line in f:
        fields = line.strip().split(',')
        route_id   = fields[0]
        route_type = fields[5]
        if int(route_type) != 3:
            continue
        routes[route_id] = set()

with open(trips_file, "r") as f:
    next(f)
    for line in f:
        route_id,service_id,trip_id,trip_headsign,direction_id,block_id,shape_id = line.strip().split(",")
        if route_id not in routes:
            continue
        routes[route_id].add(trip_id)
        trips[trip_id] = [direction_id, 1e6, 0, shape_id]

with open(stop_times_file, "r") as f:
    next(f)
    for line in f:
        trip_id,arrival_time,departure_time,stop_id,stop_sequence = line.strip().split(",")[:5]
        if trip_id not in trips:
            continue
        min_val = trips[trip_id][1]
        max_val = trips[trip_id][2]

        arr = to_time(arrival_time)

        if min_val > arr:
            trips[trip_id][1] = arr
        if max_val < arr:
            trips[trip_id][2] = arr

res = {}
for r_id, val in routes.items():
    s = set()
    for t_id in val:
        if trips[t_id][1] >= 21600 and trips[t_id][2] <= 75600:
            s.add(t_id)
    res[r_id] = s

r = OrderedDict(reversed(sorted([(r_id, len(val)) for r_id, val in res.items() if len(val) > 0], key=operator.itemgetter(1))))
print r

valid_routes = set(r.keys()[:3])
print valid_routes
valid_trips  = set(t for r in valid_routes for t in res[r])
print valid_trips
valid_shapes = set(trips[t][3] for t in valid_trips)
print valid_shapes

# output the best route_id

if not os.path.exists(gtfs_output_folder):
    os.makedirs(gtfs_output_folder)

with open(routes_output_files, "w") as f_out:
    with open(routes_files, "r") as f_in:
        header = f_in.readline()
        f_out.write(header)
        for line in f_in:
            route_id = line.strip().split(',')[0]
            if route_id in valid_routes:
                f_out.write(line)

with open(trips_output_file, "w") as f_out:
    with open(trips_file, "r") as f_in:
        header = f_in.readline()
        f_out.write(header)
        for line in f_in:
            route_id,service_id,trip_id = line.strip().split(',')[:3]
            if trip_id in valid_trips and route_id in valid_routes:
                f_out.write(line)

with open(stop_times_output_file, "w") as f_out:
    with open(stop_times_file, "r") as f_in:
        header = f_in.readline()
        f_out.write(header)
        for line in f_in:
            trip_id = line.strip().split(',')[0]
            if trip_id in valid_trips:
                f_out.write(line)

with open(shapes_output_files, "w") as f_out:
    with open(shapes_files, "r") as f_in:
        header = f_in.readline()
        f_out.write(header)
        for line in f_in:
            shape_id = line.split(',')[0]
            if shape_id in valid_shapes:
                f_out.write(line)
