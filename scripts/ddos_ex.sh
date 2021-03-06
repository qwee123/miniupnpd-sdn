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

ConnectClient() {
	client=$1
	ovs=$2
	client_ip=$3

	docker run -itd --name ${client} --net none --cap-add NET_ADMIN -e auth_server_address=${auth_server_addr}":"${auth_server_port} py_test_client
	ovs-docker add-port ${ovs} eth0 ${client} --ipaddress=${client_ip}
	docker exec ${client} ip route add default via ${client_gateway}
	PrintIfaceInfo ${client} eth0 ${ovs} 
}

PrintIfaceInfo() {
	container=$1
	ifacename=$2
	ovs=$3

	iface_num=$(docker exec ${container} ip addr show | sed -n 's/\([0-9]*\): \('${ifacename}'@\).*/\1/p')
    iface=$(ip addr show | sed -n 's/\([0-9]*\): \([a-z0-9]*_l\)@if'${iface_num}'.*/\2/p')
    echo "Interface "${ovs}"-"${container}": "${iface}
	monitor_interfaces="${monitor_interfaces} --interface ${iface}"
}

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

ConnectClient client1 ovs-s11 172.16.0.2/24
ConnectClient client2 ovs-s1 172.16.0.3/24

clientname=attacker
docker run -itd --name ${clientname} --net none --cap-add NET_ADMIN py_test_attacker
ovs-docker add-port ovs-s2 eth0 ${clientname} --ipaddress=172.16.0.11/24
docker exec ${clientname} ip route add default via ${client_gateway}
PrintIfaceInfo ${clientname} eth0 ovs-s2

if [ "$1" == onos ]; then
	echo "set ovs-controller connection"
	ovs-vsctl set-controller ovs-s11 tcp:${controller_address}:${controller_port}
	ovs-vsctl set-controller ovs-s1 tcp:${controller_address}:${controller_port}
	ovs-vsctl set-controller ovs-s2 tcp:${controller_address}:${controller_port}
	ovs-vsctl set-controller ovs-s3 tcp:${controller_address}:${controller_port}
	ovs-vsctl set-controller ovs-r1 tcp:${controller_address}:${controller_port}
fi

echo "Arguments to monitor interfaces in monitor.py: "${monitor_interfaces}
