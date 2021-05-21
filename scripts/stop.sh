#!/bin/bash

vnf_list=$(docker ps -a --format {{.Names}} -f name='^miniupnpd-sdn[0-9]*')
for vnf in ${vnf_list}
do
    ovs-docker del-ports ovs-s3 ${vnf}
    docker stop ${vnf} && docker rm ${vnf}
done

attacker_list=$(docker ps -a --format {{.Names}} -f name='^attacker[0-9]*')
for attacker in ${attacker_list}
do
    ovs-docker del-ports ovs-s2 ${attacker}
    docker stop ${attacker} && docker rm ${attacker}
done

client_list=$(docker ps -a --format {{.Names}} -f name='^client[0-9]*')
for client in ${client_list}
do
    ovs-docker del-ports ovs-s1 ${client}
    docker stop ${client} && docker rm ${client}
done

ovs-docker del-ports ovs-s3 auth_server 

ovs-vsctl del-controller ovs-s1
ovs-vsctl del-controller ovs-s2
ovs-vsctl del-controller ovs-s3
ovs-vsctl del-controller ovs-r1

docker stop auth_db auth_server onos py_test_server
docker rm auth_db auth_server py_test_server
