#!/bin/bash

nfv_net=onosnfv
version=v6
CMD="miniupnpd -f miniupnpd.conf"
docker run --rm -it --cap-add NET_ADMIN --cap-add NET_BROADCAST --network ${nfv_net} miniupnpd-sdn:${version} ${CMD}
