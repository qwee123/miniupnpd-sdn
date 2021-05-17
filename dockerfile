FROM ubuntu:18.04

RUN apt update && apt install -y net-tools iputils-ping vim tcpdump
RUN apt update && apt install -y iproute2 iptables libip4tc0

COPY miniupnpd/miniupnpd /bin

WORKDIR miniupnpd
COPY miniupnpd/miniupnpd.docker.conf miniupnpd.conf
COPY miniupnpd/netfilter/*.sh ./

CMD /bin/bash
#dep: iproute2 iptables libatm1 libelf1 libip4tc0 libip6tc0 libiptc0 libmnl0 libnetfilter-conntrack3 libnfnetlink0 libxtables12 multiarch-support
#e.g. docker run -it --name miniupnpd --cap-add NET_ADMIN miniupnpd:v1