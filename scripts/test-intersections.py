l,c=(5,3)
maxX,maxY=(100.0,100.0)

links = []
intersections = []
for i in range(0,l+2):
  x = i*maxX/(l+1)
  prev = (x,0)
  intersections.append(prev)
  for j in range(1,c+2):
  	y = j*maxY/(c+1)
  	cur = (x,y)
  	intersections.append(cur)
  	links.append((prev,cur))
  	print prev,cur
  	prev = cur

print "intersections", len(intersections)
print "links", len(links)

for j in range(0,c+2):
  y = j*maxY/(c+1)
  prev = (0,y)
  intersections.append(prev)
  for i in range(1,l+2):
  	x = i*maxX/(l+1)
  	cur = (x,y)
  	intersections.append(cur)
  	links.append((prev,cur))
  	print prev,cur
  	prev = cur

print "intersections", len(intersections)
print "links", len(links)
