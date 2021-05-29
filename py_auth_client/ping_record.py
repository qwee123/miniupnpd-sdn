from pythonping import ping
import time

DURATION = 20

stats = []
cur_time = time.time()
end_time = cur_time + DURATION
while cur_time < end_time:
    res = ping('172.17.0.2', count = 1, verbose=True)
    stats.append(res.rtt_avg)
    cur_time = time.time()
    time.sleep(1)

print(stats)