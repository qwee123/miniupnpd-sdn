#!/bin/bash

onos_nfv_network=onosnfv
controller_address=
controller_port=
controller_container_name=
client_gateway=172.16.0.1
miniupnpd_addr=172.16.0.100
miniupnpd_version=v10
vnf_num=5
attacker_num=1
client_num=2
monitor_interfaces=
victim_interface=

if [ -z "$1" ]; then
	echo "Please Specify an experiment situation, it's either 'pytest' or 'onos'"
	exit 1
fi

case "$1" in
	pytest )
		if [ ! "$(docker ps -q -f name='^py_test_server$')" ]; then
			if [ "$(docker ps -aq -f status=exited -f name='^py_test_server$')" ]; then
				docker start py_test_server
			else
				docker run -td --name py_test_server --net ${onos_nfv_network} py_test_server
			fi
		fi
		controller_container_name=py_test_server
		controller_port=60000
		;;
	onos )	
		if [ ! "$(docker ps -q -f name='^onos$')" ]; then
			if [ "$(docker ps -aq -f status=exited -f name='^onos$')" ]; then
				docker start onos
			else
				docker run -td --name onos --net ${onos_nfv_network} onosproject/onos:2.5.1 #no interactive mode
			fi
		fi
		controller_container_name=onos
		controller_port=6653
		;;
	-h|--help )
		echo "Usage : $0 [pytest|onos]"
		echo "pytest: Use pytest server instead of a real sdn controller."
		echo "onos: Use onos controller."
		exit 1
		;;
	* )
		echo "Unknown mode"
		exit 1
		;;
esac

controller_address=$(docker inspect ${controller_container_name} -f '{{ .NetworkSettings.Networks.'${onos_nfv_network}'.IPAddress }}')

if [ "$(docker ps -aq -f name='^miniupnpd-sdn$')" ]; then
	docker stop miniupnpd-sdn && docker rm miniupnpd-sdn
fi

vnfname=
vnfip=
for i in $(seq 1 ${vnf_num})
do
	vnfname=miniupnpd-sdn${i}
	vnfip=172.16.0.$((i+100))
	docker run -itd --name ${vnfname} --cap-add NET_ADMIN --cap-add NET_BROADCAST \
		--network ${onos_nfv_network} -e CONTROLLER_ADDRESS=${controller_address}":"${controller_igd_app_port} \
		miniupnpd-sdn:${miniupnpd_version}
	ovs-docker add-port ovs-s3 eth1 ${vnfname} --ipaddress=${vnfip}/24
	iface_num=$(docker exec ${vnfname} ip addr show | sed -n 's/\([0-9]*\): \(eth1@\).*/\1/p')
	iface=$(ip addr show | sed -n 's/\([0-9]*\): \([a-z0-9]*_l\)@if'${iface_num}'.*/\2/p')
	echo "Interface s3-"${vnfname}": "${iface}
	monitor_interfaces="${monitor_interfaces} --interface ${iface}"
	docker exec -d ${vnfname} miniupnpd -f miniupnpd.conf
done

clientname=
for i in $(seq 1 ${client_num})
do
	clientname=client${i}
	docker run -itd --name ${clientname} --net none --cap-add NET_ADMIN py_test_client
	ovs-docker add-port ovs-s1 eth0 ${clientname} --ipaddress=172.16.0.$((i+1))/24
	docker exec ${clientname} ip route add default via ${client_gateway}
	iface_num=$(docker exec ${clientname} ip addr show | sed -n 's/\([0-9]*\): \(eth0@\).*/\1/p')
	iface=$(ip addr show | sed -n 's/\([0-9]*\): \([a-z0-9]*_l\)@if'${iface_num}'.*/\2/p')
	echo "Interface s1-"${clientname}": "${iface}
	monitor_interfaces="${monitor_interfaces} --interface ${iface}"
done

for i in $(seq 1 ${attacker_num})
do
	clientname=attacker${i}
	docker run -itd --name ${clientname} --net none --cap-add NET_ADMIN py_test_attacker
	ovs-docker add-port ovs-s2 eth0 ${clientname} --ipaddress=172.16.0.$((i+10))/24
	docker exec ${clientname} ip route add default via ${client_gateway}
	iface_num=$(docker exec ${clientname} ip addr show | sed -n 's/\([0-9]*\): \(eth0@\).*/\1/p')
	iface=$(ip addr show | sed -n 's/\([0-9]*\): \([a-z0-9]*_l\)@if'${iface_num}'.*/\2/p')
	echo "Interface s2-"${clientname}": "${iface}
	monitor_interfaces="${monitor_interfaces} --interface ${iface}"
done

if [ "$1" == onos ]; then
	echo "set ovs-controller connection"
	ovs-vsctl set-controller ovs-s1 tcp:${controller_address}:${controller_port}
	ovs-vsctl set-controller ovs-s2 tcp:${controller_address}:${controller_port}
	ovs-vsctl set-controller ovs-s3 tcp:${controller_address}:${controller_port}
	ovs-vsctl set-controller ovs-r1 tcp:${controller_address}:${controller_port}
fi

echo "Arguments to monitor interfaces in monitor.py: "${monitor_interfaces}
