# -*- encoding: utf-8 -*-
# Refered From https://github.com/prinzdezibel/ifstat

import argparse
import errno
import os
import sys
import time
import matplotlib.pyplot as plt


parser = argparse.ArgumentParser(description="Get simple receive/transmission" \
                                          " statistics from network interface.")
parser.add_argument('--interface', '-i', action='append',
                     help="Network interface for which to compute statistics." \
                           " Can be specified multiple times.")
parser.add_argument('--sampling', '-s', default=60, type=int,
                    help="period to monitor and record the traffic" \
                            "default is 60.")
parser.add_argument('--output', '-o', default="results",
                    help="Output directory of the result. Could be either an absolute or relative path." \
                            "Default is \"results\".")

namespace = parser.parse_args()
NET_DIR = "/sys/class/net"
STAT_DIRS = list()
INTERFACES = namespace.interface
if not INTERFACES:
    sys.exit("Please Specify at least one interface to be monitored.")
else:
    for i in INTERFACES:
        d = os.sep.join([NET_DIR, i, 'statistics'])
        if not os.path.exists(d):
            sys.exit("Network interface '%s' does not exist. " \
                     " Program aborted." % i)
        STAT_DIRS.append(d)
APP_DIR = namespace.output
SAMPLING = namespace.sampling    
SAMPLING_INTERVAL = 1
PLOT_TIMESTAMP = [t for t in range(SAMPLING_INTERVAL, SAMPLING, SAMPLING_INTERVAL)]
def main():
    try:
        mkdir_p(APP_DIR)
    except OSError as exc:
        if exc.errno == errno.EACCES:
            sys.exit("Please create directory %s with access for current " \
                     "user." % APP_DIR)
        else:
            raise

    allstats = dict([(i, list()) for i in INTERFACES])

    START_TIMESTAMP = time.time()
    END_TIMESTAMP = START_TIMESTAMP + SAMPLING
    NEXT_TIMESTAMP = START_TIMESTAMP
    while NEXT_TIMESTAMP < END_TIMESTAMP :

        for i in range(len(INTERFACES)):
            interface = INTERFACES[i]
            stat_dir = STAT_DIRS[i]
            with open(os.sep.join([stat_dir, 'rx_bytes'])) as f:
                rx_bytes = int(f.read())
            with open(os.sep.join([stat_dir, 'tx_bytes'])) as f:
                tx_bytes = int(f.read())
            allstats[interface].append((rx_bytes, tx_bytes))

        NEXT_TIMESTAMP = NEXT_TIMESTAMP + SAMPLING_INTERVAL
        while time.time() < NEXT_TIMESTAMP:
            time.sleep(0.1)
    
    result = dict([(i, (list(), list())) for i in INTERFACES])
    for iface in INTERFACES:
        result[iface] = cal(allstats[iface])
    for iface in INTERFACES:
        drawResult(iface, result[iface])

def cal(stats):
    rx_rate = list()
    tx_rate = list()
    
    prev_rx = stats[0][0]
    prev_tx = stats[0][1]
    for rx_bytes, tx_bytes in stats[1:]:
        rx_rate.append(rx_bytes - prev_rx)
        tx_rate.append(tx_bytes - prev_tx)

        prev_rx = rx_bytes
        prev_tx = tx_bytes
    
    return (rx_rate, tx_rate)

def drawResult(iface, stats):
    plt.figure(figsize=(15,10), dpi=100, linewidth=2)
    plt.plot(PLOT_TIMESTAMP, stats[0], color='r', label="RX_BYTES")
    plt.plot(PLOT_TIMESTAMP, stats[1], color='b', label="TX_BYTES")
    plt.xlabel('timestamp')
    plt.ylabel('bytes/second')
    plt.savefig(os.sep.join([APP_DIR, iface]))


def mkdir_p(path):
    try:
        os.makedirs(path)
    except OSError as exc:
        if exc.errno == errno.EEXIST and os.path.isdir(path):
            pass
        else:
            raise

if __name__ == '__main__':
    main()