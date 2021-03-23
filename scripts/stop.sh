ovs-docker del-ports ovs-s1 client1
ovs-docker del-ports ovs-s1 client2
ovs-docker del-ports ovs-s2 miniupnpd-sdn

ovs-vsctl del-controller ovs-s1
ovs-vsctl del-controller ovs-s2
ovs-vsctl del-controller ovs-r1


docker stop client1 client2 miniupnpd-sdn onos py_test_server
docker rm client1 client2 miniupnpd-sdn py_test_server
