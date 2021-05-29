# -*- encoding: utf-8 -*-
# Refered From https://github.com/prinzdezibel/ifstat

import argparse
import errno
import os
import sys
import time
import matplotlib.pyplot as plt
import shutil
import subprocess
import string

parser = argparse.ArgumentParser(description="Get simple receive/transmission" \
                                          " statistics from network interface.")
parser.add_argument('--interface', '-i', action='append',
                     help="Network interface for which to compute statistics." \
                           " Can be specified multiple times.")
parser.add_argument('--victim-interface', '-vi',
                     help="Network interface for which to compute statistics." \
                           " Can be specified multiple times.")
parser.add_argument('--duration', '-d', default=60, type=int,
                    help="period to monitor and record the traffic" \
                            "default is 60.")
parser.add_argument('--output', '-o', default="results",
                    help="Output directory of the result. Could be either an absolute or relative path." \
                            "Default is \"results\".")

namespace = parser.parse_args()
NET_DIR = "/sys/class/net"
STAT_DIRS = list()
INTERFACES = namespace.interface
VICTIM_INTERFACE = namespace.victim_interface
if not INTERFACES or not VICTIM_INTERFACE:
    sys.exit("Please Specify at least one interface to be monitored for traffic, and one victim interface for queue depth.")
else:
    for i in INTERFACES:
        d = os.sep.join([NET_DIR, i, 'statistics'])
        if not os.path.exists(d):
            sys.exit("Network interface '%s' does not exist. " \
                     " Program aborted." % i)
        STAT_DIRS.append(d)
APP_DIR = namespace.output
SAMPLING = namespace.duration    
SAMPLING_INTERVAL = 0.5
def main():
    try:
        initpath(APP_DIR)
    except OSError as exc:
        if exc.errno == errno.EACCES:
            sys.exit("Please create directory %s with access for current " \
                     "user." % APP_DIR)
        else:
            raise

    allstats = dict([(i, list()) for i in INTERFACES])
    queuestats = []

    START_TIMESTAMP = time.time()
    END_TIMESTAMP = START_TIMESTAMP + SAMPLING
    NEXT_TIMESTAMP = START_TIMESTAMP
    while NEXT_TIMESTAMP < END_TIMESTAMP:

        for i in range(len(INTERFACES)):
            interface = INTERFACES[i]
            stat_dir = STAT_DIRS[i]
            with open(os.sep.join([stat_dir, 'rx_bytes'])) as f:
                rx_bytes = int(f.read())
            with open(os.sep.join([stat_dir, 'tx_bytes'])) as f:
                tx_bytes = int(f.read())
            allstats[interface].append((rx_bytes, tx_bytes))

        #tc_res = subprocess.Popen(['tc', '-s', '-p', 'qdisc', 'ls', 'dev', VICTIM_INTERFACE], stdout=subprocess.PIPE, universal_newlines=True)
        #queue_dep = subprocess.Popen(['sed', '-n', 's/ backlog \\([0-9]*[K]\?\\)b \\([0-9]*\\)p \\(.*\\)$/\\2/p'], stdin=tc_res.stdout, stdout=subprocess.PIPE, universal_newlines=True)
        #out, err = queue_dep.communicate()
        #queuestats.append(out)

        NEXT_TIMESTAMP = NEXT_TIMESTAMP + SAMPLING_INTERVAL
        while time.time() < NEXT_TIMESTAMP:
            time.sleep(0.1)
    
    result = dict([(i, (list(), list())) for i in INTERFACES])
    for iface in INTERFACES:
        result[iface] = cal(allstats[iface])
    for iface in INTERFACES:
        drawResult(iface, result[iface])

    #post_queue_data = postProcessQueueLenData(queuestats)
    #drawQueueLen(VICTIM_INTERFACE, post_queue_data)

def postProcessQueueLenData(stats):
    result = []
    for data in stats:
        try:
            result.append(int(data[:-1]))
        except ValueError:
            print(data)
            result.append(0)

    return result

def cal(stats):
    rx_rate = list()
    tx_rate = list()
    SAMPLING_ENTRY_INTERVAL = (int)(1/SAMPLING_INTERVAL)
    for i in range(SAMPLING_ENTRY_INTERVAL, len(stats)):
        rx_bytes = stats[i][0]
        tx_bytes = stats[i][1]

        rx_rate.append((rx_bytes - stats[i-SAMPLING_ENTRY_INTERVAL][0])/1024)
        tx_rate.append((tx_bytes - stats[i-SAMPLING_ENTRY_INTERVAL][1])/1024)
    
    return (rx_rate, tx_rate)

def drawResult(iface, stats):
    PLOT_TIMESTAMP=[i*SAMPLING_INTERVAL for i in range(len(stats[0]))]
    plt.figure(figsize=(15,10), dpi=100, linewidth=2)
    plt.plot(PLOT_TIMESTAMP, stats[0], color='r', label="RX_BYTES")
    plt.plot(PLOT_TIMESTAMP, stats[1], color='b', label="TX_BYTES")
    plt.xlabel('timestamp')
    plt.ylabel('KB/s')
    plt.legend(loc='upper right')
    plt.savefig(os.sep.join([APP_DIR, iface]))

def drawQueueLen(iface, stats):
    PLOT_TIMESTAMP=[i*SAMPLING_INTERVAL for i in range(len(stats))]
    plt.figure(figsize=(15,10), dpi=100, linewidth=2)
    plt.bar(PLOT_TIMESTAMP, stats, width=0.45 , color='b', edgecolor='w')
    plt.xlabel('timestamp')
    plt.ylabel('packets')
    plt.savefig(os.sep.join([APP_DIR, iface + '_queuelen']))


def initpath(path):
    shutil.rmtree(path)
    try:
        os.makedirs(path)
    except OSError as exc:
        raise
    

if __name__ == '__main__':
    main()