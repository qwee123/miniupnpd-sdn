import upnpy

upnp = upnpy.UPnP()
devices = upnp.discover()
device = upnp.get_igd()

# We will access it like a dictionary instead:
service = device['WANIPConn1']
service2 = device['WANCommonIFC1']


print("Tests for service WANIPConn1: ")
print(*(service.get_actions()), sep="\n")

'''
print("DeletePortMappingRange", service.DeletePortMappingRange(
    NewStartPort=20,
    NewEndPort=8888,
    NewProtocol="tcp",
    NewManage=False
))
print("DeletePortMapping", service.DeletePortMapping(
    NewRemoteHost="*",
    NewExternalPort=8888,
    NewProtocol="tcp"
))

print("AddAnyPortMapping: ", service.AddAnyPortMapping(
    NewRemoteHost="*",
    NewExternalPort=6666,
    NewProtocol="udp",
    NewInternalPort=220,
    NewInternalClient="123.123.123.123",
    NewEnabled=True,
    NewPortMappingDescription="",
    NewLeaseDuration=500000
))

print("GetSpecificPortMappingEntry: ", service.GetSpecificPortMappingEntry(
    NewRemoteHost="*",
    NewExternalPort=5642,
    NewProtocol="udp"
))

print("GetGenericPortMappingEntry: ", service.GetGenericPortMappingEntry(
    NewPortMappingIndex=3
))
'''
'''
print("AddPortMapping: ", service.AddPortMapping(
    NewRemoteHost="160.160.160.160/32",
    NewExternalPort=2000,
    NewProtocol="udp",
    NewInternalPort=220,
    NewInternalClient="123.123.123.123",
    NewEnabled=True,
    NewPortMappingDescription="",
    NewLeaseDuration=20
))

print("AddPortMapping: ", service.AddPortMapping(
    NewRemoteHost="150.150.150.150/32",
    NewExternalPort=2000,
    NewProtocol="udp",
    NewInternalPort=220,
    NewInternalClient="123.123.123.123",
    NewEnabled=True,
    NewPortMappingDescription="",
    NewLeaseDuration=100
))

print("AddPortMapping: ", service.AddPortMapping(
    NewRemoteHost="150.150.150.150/32",
    NewExternalPort=5000,
    NewProtocol="udp",
    NewInternalPort=220,
    NewInternalClient="123.123.123.123",
    NewEnabled=True,
    NewPortMappingDescription="",
    NewLeaseDuration=100
))

print("AddPortMapping: ", service.AddPortMapping(
    NewRemoteHost="150.150.150.150/24",
    NewExternalPort=2001,
    NewProtocol="tcp",
    NewInternalPort=221,
    NewInternalClient="158.123.252.252",
    NewEnabled=True,
    NewPortMappingDescription="",
    NewLeaseDuration=100
))

print("AddPortMapping: ", service.AddPortMapping(
    NewRemoteHost="150.150.150.150/8",
    NewExternalPort=2001,
    NewProtocol="tcp",
    NewInternalPort=221,
    NewInternalClient="158.123.252.252",
    NewEnabled=True,
    NewPortMappingDescription="",
    NewLeaseDuration=100
))

print("AddPortMapping: ", service.AddPortMapping(
    NewRemoteHost="*",
    NewExternalPort=6667,
    NewProtocol="tcp",
    NewInternalPort=221,
    NewInternalClient="158.123.252.252",
    NewEnabled=True,
    NewPortMappingDescription="",
    NewLeaseDuration=1000
))

print("AddPortMapping: ", service.AddPortMapping(
    NewRemoteHost="*",
    NewExternalPort=3000,
    NewProtocol="tcp",
    NewInternalPort=221,
    NewInternalClient="158.123.252.252",
    NewEnabled=True,
    NewPortMappingDescription="",
    NewLeaseDuration=1000
))


print("DeletePortMappingRange", service.DeletePortMappingRange(
    NewStartPort=20,
    NewEndPort=3000,
    NewProtocol="udp",
    NewManage=False
))

print("DeletePortMappingRange", service.DeletePortMappingRange(
    NewStartPort=20,
    NewEndPort=3000,
    NewProtocol="tcp",
    NewManage=False
))
'''
'''
print("DeletePortMapping", service.DeletePortMapping(
    NewRemoteHost="150.150.150.150/24",
    NewExternalPort=2001,
    NewProtocol="tcp"
))

print("DeletePortMapping", service.DeletePortMapping(
    NewRemoteHost="*",
    NewExternalPort=6667,
    NewProtocol="tcp"
))
'''

print("AddPortMapping: ", service.AddPortMapping(
    NewRemoteHost="*",
    NewExternalPort=6667,
    NewProtocol="tcp",
    NewInternalPort=221,
    NewInternalClient="172.16.0.2",
    NewEnabled=True,
    NewPortMappingDescription="",
    NewLeaseDuration=10000
))
print("GetListOfPortMappings", service.GetListOfPortMappings(
    NewStartPort=20,
    NewEndPort=60000,
    NewProtocol="tcp",
    NewManage=False,
    NewNumberOfPorts=20
))

'''
print("GetListOfPortMappings", service.GetListOfPortMappings(
    NewStartPort=20,
    NewEndPort=60000,
    NewProtocol="tcp",
    NewManage=False,
    NewNumberOfPorts=20
))

print("AddPortMapping: ", service.AddPortMapping(
    NewRemoteHost="150.150.119.10/24",
    NewExternalPort=120,
    NewProtocol="tcp",
    NewInternalPort=221,
    NewInternalClient="158.123.252.252",
    NewEnabled=True,
    NewPortMappingDescription="",
    NewLeaseDuration=500000
))

print("AddPortMapping: ", service.AddPortMapping(
    NewRemoteHost="*",
    NewExternalPort=120,
    NewProtocol="udp",
    NewInternalPort=6600,
    NewInternalClient="158.123.252.252",
    NewEnabled=True,
    NewPortMappingDescription="",
    NewLeaseDuration=500000
))

print("GetListOfPortMappings", service.GetListOfPortMappings(
    NewStartPort=20,
    NewEndPort=60000,
    NewProtocol="udp",
    NewManage=False,
    NewNumberOfPorts=20
))

print("GetListOfPortMappings", service.GetListOfPortMappings(
    NewStartPort=20,
    NewEndPort=60000,
    NewProtocol="tcp",
    NewManage=False,
    NewNumberOfPorts=20
))

print("GetListOfPortMappings", service.GetListOfPortMappings(
    NewStartPort=20,
    NewEndPort=2000,
    NewProtocol="tcp",
    NewManage=False,
    NewNumberOfPorts=20
))

print("DeletePortMappingRange", service.DeletePortMappingRange(
    NewStartPort=20,
    NewEndPort=2000,
    NewProtocol="tcp",
    NewManage=False
))

print("DeletePortMapping", service.DeletePortMapping(
    NewRemoteHost="*",
    NewExternalPort=120,
    NewProtocol="udp"
))

print("GetListOfPortMappings", service.GetListOfPortMappings(
    NewStartPort=20,
    NewEndPort=60000,
    NewProtocol="tcp",
    NewManage=False,
    NewNumberOfPorts=20
))

print("GetListOfPortMappings", service.GetListOfPortMappings(
    NewStartPort=20,
    NewEndPort=60000,
    NewProtocol="udp",
    NewManage=False,
    NewNumberOfPorts=20
))
'''
'''
print("GetExternalIPAddress: ", service.GetExternalIPAddress())
print("GetConnectionTypeInfo: ", service.GetConnectionTypeInfo())
print("GetStatusInfo: ", service.GetStatusInfo())
print("\n\n")

print("Tests for service WANCommonIFC1\n")
print(*(service2.get_actions()), sep="\n")
print("GetCommonLinkProperties: ", service2.GetCommonLinkProperties())
print("GetTotalBytesSent: ", service2.GetTotalBytesSent())
print("GetTotalBytesReceived: ", service2.GetTotalBytesReceived())
print("GetTotalPacketsSent: ", service2.GetTotalPacketsSent())
print("GetTotalPacketsReceived: ", service2.GetTotalPacketsReceived())
'''