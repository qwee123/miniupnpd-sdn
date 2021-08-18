import argparse
import errno
import os
import sys
import time
import matplotlib.pyplot as plt
from mpl_toolkits.axes_grid.inset_locator import (inset_axes, InsetPosition, mark_inset)
import shutil
import requests
import logging
import datetime

testmode = 'sdn-first'

OUT_DIR = "./"
sample_num = 50
#url = 'http://www.google.com'
url_normal = 'http://140.114.93.102:40000/' #normal case
url_docker = 'http://172.17.0.2:60000'
url_sdn = 'http://192.168.1.10:1024/' #sdn
url = ''

if testmode == 'normal':
    url = url_normal
elif testmode == 'docker':
    url = url_docker
elif testmode == 'sdn' or testmode == 'sdn-first':
    url = url_sdn
else:
    exit(-1)

requests.get(url) #init global object of requests package and activate the traffic in SDN mode
if testmode == 'sdn-first':
    time.sleep(20) # wait until the reative flow timeout

timestamps = []
for sn in range(sample_num):
    timestamps.append(time.time())
    requests.get(url)
    timestamps.append(time.time())
    if testmode == 'sdn-first':
        time.sleep(20) # wait until the reactive flow timeout

filename_ext = time.strftime("%Y-%m-%d-%H-%M-%S", time.localtime())
output = open(testmode+'-'+filename_ext, 'w')
for i in range(0, len(timestamps), 2):
    duration = (timestamps[i+1] - timestamps[i])*1000
    output.write(str(duration) + ', ')
output.close()
'''
plot_timestamps = [t+1 for t in range(tries)]

fig, ax1 = plt.subplots()
ax1.bar(plot_timestamps, durations)
ax1.set_title('Request Lantency in SDN')
ax1.set_xlabel('xth connection attempts')
ax1.set_ylabel('time(ms)')
#ax1.set_ylim([0, 18])
ax1.set_xticks(plot_timestamps)

# Create a set of inset Axes: these should fill the bounding box allocated to
# them.
ax2 = plt.axes([0,0,1,1])
# Manually set the position and relative size of the inset axes within ax1
ip = InsetPosition(ax1, [0.4,0.4,0.5,0.5])
ax2.set_axes_locator(ip)
# Mark the region corresponding to the inset axes on ax1 and draw lines
# in grey linking the two axes.

ax2.bar(plot_timestamps, durations)
ax2.set_ylim([0, 10])
ax2.set_ylabel('time(ms)')
ax2.set_xticks(plot_timestamps)

filename_ext = time.strftime("%Y-%m-%d-%H-%M-%S", time.localtime())
plt.savefig(os.sep.join([OUT_DIR, filename_ext]))
'''