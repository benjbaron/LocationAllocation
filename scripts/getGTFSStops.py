import sys
import os

stops_filename = sys.argv[1]
f_name, f_ext = os.path.splitext(stops_filename)
f_name_out = f_name+"-out"+f_ext

with open(f_name_out, "w") as f_out:
	with open(stops_filename, "r") as f_in:
		next(f_in)
		for line in f_in:
			stop_lat,stop_lon = line.split(",")[3:5]
			f_out.write("%f %f\n" % (float(stop_lat),float(stop_lon)))
