l = [1,4,8,5,14,7]
avg = 0.0
count = 0.0

for v in l:
  print avg, count, v
  avg = (avg * count + v) / (count + 1.0)
  count += 1.0

print "#1",avg, count

count = 0.0
summ = 0.0
for v in l:
  summ += v
  count += 1.0
print "#2",summ/count, count