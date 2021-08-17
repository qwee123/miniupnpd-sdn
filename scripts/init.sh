#!/bin/bash

onos_nfv_network=onosnfv
auth_database_network=databases
controller_address=
controller_port=
controller_igd_app_port=40000
controller_container_name=
client_gateway=172.16.0.1
miniupnpd_addr=172.16.0.100
miniupnpd_version=v11
auth_server_addr=172.16.0.150
auth_server_port=50000
auth_db_address=
auth_db_root_pass=$(cat db_pass.txt)
wan2_mac=c2:67:18:3d:bc:ca
wan2_ip=192.168.1.11/24

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
		controller_port=40000
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

#db initialization may take up to 15 secs. No connection could be made until the process is complete.
docker run --name auth_db -e MARIADB_ROOT_PASSWORD=${auth_db_root_pass} --network ${auth_database_network} -td mariadb
auth_db_address=$(docker inspect auth_db -f '{{ .NetworkSettings.Networks.'${auth_database_network}'.IPAddress }}')

docker run -itd --name miniupnpd-sdn --cap-add NET_ADMIN --cap-add NET_BROADCAST \
	-e CONTROLLER_ADDRESS=${controller_address}":"${controller_igd_app_port} \
	--network ${onos_nfv_network} miniupnpd-sdn:${miniupnpd_version}
ovs-docker add-port ovs-s3 eth1 miniupnpd-sdn --ipaddress=${miniupnpd_addr}/24
PrintIfaceInfo miniupnpd-sdn eth1 ovs-s3

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

ip netns add demo
ip link add name wan2 type veth peer name wan3
ip link set wan2 netns demo
ovs-vsctl add-port ovs-r1 wan3
ip netns exec demo ifconfig wan2 hw ether ${wan2_mac}
ip netns exec demo ip addr add ${wan2_ip} dev wan2
ip netns exec demo ip link set wan2 up
ip link set wan3 up
sysctl -w net.ipv4.conf.wan3.forwarding=1

#Disable stderr during testing database connectivity 
exec 3>&2
exec 2> /dev/null
echo "Waiting for database to complete initialization..."
for i in {1..20}
do
    result=$(mysqladmin ping -h ${auth_db_address} -uroot -p${auth_db_root_pass} | grep "mysqld is alive")
    if [ -n "${result}" ]; then
        break
    fi
    sleep 2
done
exec 2>&3

mysql -h ${auth_db_address} -uroot -p${auth_db_root_pass} < ./init_db.sql

docker run -td --name auth_server --network ${auth_database_network} -e port=${auth_server_port} -e db_addr=${auth_db_address} -e db_port=3306 demo_auth_server
ovs-docker add-port ovs-s3 eth1 auth_server --ipaddress=${auth_server_addr}/24
