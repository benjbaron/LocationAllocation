import sys
import os
import re
import urllib2
import time
import datetime
from datetime import time
from collections import OrderedDict

# URL format: http://mhd.ms2soft.com/tcds/graph/tcds_vgcs_24hr.aspx?i=H8495&sdate=2016-10-18&a=96
BASE_URL = "http://mhd.ms2soft.com/tcds/graph/tcds_vgcs_24hr.aspx"
NB_MAX_REQ = 3
dates = ["2/21/2015"]# , "2/22/2015", "2/23/2015", "2/24/2015", "2/25/2015", "2/26/2015", "2/27/2015", "2/28/2015", "3/1/2015"]
NB_SPEED_CLASSES = 15 # to get the speeds

re_prog = re.compile(r"data\.addRows\(\[(.*?)\]\)")
re_blanks = re.compile(r"[\n\r\'\t\s+]")

filename = sys.argv[1]
f_name, f_ext = os.path.splitext(filename)

def speed_class_to_average(s):
	if s == 0: return 10 # 0-20 mph
	if s == 1: return 23 # 21-25 mph
	if s == 2: return 28 # 26-30 mph
	if s == 3: return 33 # 31-35 mph
	if s == 4: return 38 # 36-40 mph
	if s == 5: return 43 # 41-45 mph
	if s == 6: return 48 # 46-50 mph
	if s == 7: return 53 # 51-55 mph
	if s == 8: return 58 # 56-60 mph
	if s == 9: return 63 # 61-65 mph
	if s == 10: return 68 # 66-70 mph
	if s == 11: return 73 # 71-75 mph
	if s == 12: return 78 # 76-80 mph
	if s == 13: return 83 # 81-85 mph
	if s == 14: return 86 # 86+ mph

def time_to_time_period(h, period):
	# time format: 7:45AM
	d = datetime.datetime.strptime(h, "%I:%M%p").time()
	return (d.minute + d.hour*60) / period


def get_active_locations():
	f_flow = f_name+"-flow"+f_ext
	ids = set()
	with open(f_flow, 'r') as f:
		for line in f:
			fields = line.split(";")
			ids.add(fields[0])

	for id in ids:
		print id


def send_request(base, params = {}):
	""" Forms the request to send to the API and returns the data if no errors occured """
	nb_req = 0
	while nb_req < NB_MAX_REQ:
		full_url = base + "?" + "&".join(["%s=%s" % (k,v.replace(' ','+')) for (k,v) in params.items()])

		print full_url

		try:
			data = urllib2.urlopen(full_url).read()
		except:
			print "error:", sys.exc_info()[0]
			nb_req += 1
			time.sleep(min(1,nb_req/10))
			continue

		print "data recieved"
		return data
	return None


def write_flows():
	f_flow_out = f_name+"-flow"+f_ext
	f_w_flow  = open(f_flow_out, 'w')

	with open(filename, 'r') as f:
		f.readline()
		for line in f:
			fields = line.strip().split(";")
			loc_id = fields[0]
			lat = fields[9]
			lon = fields[10]
			latest = fields[12]
			if latest and (int(latest.split("/")[2]) >= 2015):
				print loc_id, lat, lon, latest
				for d in dates:
					params = {
						'i': loc_id,
						'sdate': d,
						'a': '96'
					}
					data = send_request(BASE_URL, params)
					text = re_blanks.sub("", data)
					m = re.search(re_prog, text)
					f = m.group(1)
					if not f:
						continue
					flows = re.findall(r"\[(.*?)\]", f)
					period = 60 * 24 / len(flows)
					for flow in flows:
						flow_fields = flow.split(",")
						time_period = time_to_time_period(flow_fields[0], period)
						flow_2_way  = int(flow_fields[1])
						flow_NB     = int(flow_fields[2])
						flow_SB     = int(flow_fields[3])
						f_w_flow.write("%s;%s;%d;%d;%d;%d;%d\n" % (loc_id, d, time_period, period, flow_2_way, flow_NB, flow_SB))

def write_speeds():
	f_speed_out = f_name+"-speed"+f_ext
	f_w_speed = open(f_speed_out, 'w')

	f_speed_mean_out = f_name+"-speed-mean"+f_ext
	f_w_speed_mean = open(f_speed_mean_out, 'w')

	with open(filename, 'r') as f:
		f.readline()
		for line in f:
			fields = line.strip().split(";")
			loc_id = fields[0]
			lat = fields[9]
			lon = fields[10]
			latest = fields[12]
			if latest and (int(latest.split("/")[2]) >= 2015):
				print loc_id, lat, lon, latest
				counter = 0
				for d in dates:
					speed_2_way_avg = OrderedDict()
					speed_NB_avg = OrderedDict()
					speed_SB_avg = OrderedDict()
					for i in range(0,NB_SPEED_CLASSES):
						params = {
							'i': loc_id,
							'sdate': d,
							'a': '96',
							't': 's',
							'class': str(i)
						}
						data = send_request(BASE_URL, params)
						text = re_blanks.sub("", data)
						m = re.search(re_prog, text)
						f = m.group(1)
						if not f:
							print "no data"
							continue
						
						speeds = re.findall(r"\[(.*?)\]", f)
						period = 60 * 24 / len(speeds)
						
						for speed in speeds:
							speed_fields = speed.split(",")
							time_period  = time_to_time_period(speed_fields[0], period)
							speed_2_way  = int(speed_fields[1])
							speed_NB     = int(speed_fields[2])
							speed_SB     = int(speed_fields[3])
							f_w_speed.write("%s;%s;%d;%d;%d;%d;%d;%d\n" % (loc_id, d, time_period, period, i, speed_2_way, speed_NB, speed_SB))

							# compute averages
							if time_period not in speed_2_way_avg:
								speed_2_way_avg[time_period] = [0.0,0.0]
							if time_period not in speed_NB_avg:
								speed_NB_avg[time_period] = [0.0,0.0]
							if time_period not in speed_SB_avg:
								speed_SB_avg[time_period] = [0.0,0.0]
							
							speed_value = speed_class_to_average(i)
							speed_2_way_avg[time_period][0] += speed_value * speed_2_way
							speed_NB_avg[time_period][0]    += speed_value * speed_NB
							speed_SB_avg[time_period][0]    += speed_value * speed_SB

							speed_2_way_avg[time_period][1] += speed_2_way
							speed_NB_avg[time_period][1]    += speed_NB
							speed_SB_avg[time_period][1]    += speed_SB

					if len(speed_2_way_avg) > 0:
						period = 60 * 24 / len(speed_2_way_avg)
						for tp in speed_2_way_avg.keys():
							avg_2_way = 0.0 if speed_2_way_avg[tp][1] == 0 else speed_2_way_avg[tp][0] / speed_2_way_avg[tp][1]
							avg_NB = 0.0 if speed_NB_avg[tp][1] == 0 else speed_NB_avg[tp][0] / speed_NB_avg[tp][1]
							avg_SB = 0.0 if speed_SB_avg[tp][1] == 0 else speed_SB_avg[tp][0] / speed_SB_avg[tp][1]
							f_w_speed_mean.write("%s;%s;%d;%d;%f;%f;%f\n" % (loc_id, d, tp, period, 
								avg_2_way, 
								avg_NB, 
								avg_SB))
 
def main():
	write_speeds()

if __name__ == '__main__':
	main()



