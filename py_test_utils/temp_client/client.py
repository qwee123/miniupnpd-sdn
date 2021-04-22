import upnpy

upnp = upnpy.UPnP()
devices = upnp.discover()
device = upnp.get_igd()

# We will access it like a dictionary instead:
service = device['WANIPConn1']
service2 = device['WANCommonIFC1']

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
