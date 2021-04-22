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

ovs-vsctl del-controller ovs-s1
ovs-vsctl del-controller ovs-s2
ovs-vsctl del-controller ovs-s3
ovs-vsctl del-controller ovs-r1

docker stop onos py_test_server
docker rm py_test_server
