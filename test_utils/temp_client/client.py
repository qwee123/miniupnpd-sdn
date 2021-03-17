import upnpy

upnp = upnpy.UPnP()
devices = upnp.discover()
device = upnp.get_igd()

# We will access it like a dictionary instead:
service = device['WANIPConn1']
service2 = device['WANCommonIFC1']

print("Tests for service WANIPConn1: ")
print(*(service.get_actions()), sep="\n")
print("GetExternalIPAddress: ", service.GetExternalIPAddress())
print("GetConnectionTypeInfo: ", service.GetConnectionTypeInfo())
print("\n\n")

print("Tests for service WANCommonIFC1\n")
print(*(service2.get_actions()), sep="\n")
print("GetCommonLinkProperties: ", service2.GetCommonLinkProperties())
print("GetTotalBytesSent: ", service2.GetTotalBytesSent())
print("GetTotalBytesReceived: ", service2.GetTotalBytesReceived())
print("GetTotalPacketsSent: ", service2.GetTotalPacketsSent())
print("GetTotalPacketsReceived: ", service2.GetTotalPacketsReceived())