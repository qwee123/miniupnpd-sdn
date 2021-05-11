from scapy.all import *

arp = Ether()/ARP(pdst="172.16.0.200")
res, unans = sr(arp)
