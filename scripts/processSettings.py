import sys
import os
import re

REGEX = r"Group(\d+).groupID\s\=\st"

filename = sys.argv[1]
f_name, f_ext = os.path.splitext(filename)
f_name_out = f_name+"-out"+f_ext

file_in  = open(filename, 'r')
file_out = open(f_name_out, 'w')

for line in file_in:
	m = re.match(REGEX, line)
	if m:
		line = m.group(0)+m.group(1)
	file_out.write("%s\n" % line.strip())
