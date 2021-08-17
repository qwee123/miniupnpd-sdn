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

OUT_DIR = "./"
tries = 10
#url = 'http://172.17.0.3:40000/' #normal case
url = 'http://192.168.1.10:1024/' #sdn

timestamps = []

s = requests.Session()
timestamps.append(time.time())
for i in range(tries):
    s.get(url)
    timestamps.append(time.time())

durations = []
for i in range(1, len(timestamps)):
    durations.append((timestamps[i] - timestamps[i-1])*1000)

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
