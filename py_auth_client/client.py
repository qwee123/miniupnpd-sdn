from upnpy_auth import upnpy

if __name__ == "__main__":

    with open("token.jwt", "r") as f:
        auth_token = f.read()
        f.close()

    with open("rs256.key", "r") as f:
        priv_key = f.read()
        f.close()
    
    auth_token = "Bearer " + auth_token

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
        NewInternalPort=60000,
        NewInternalClient="172.16.0.2",
        NewEnabled=True,
        NewPortMappingDescription="",
        NewLeaseDuration=10000,
        Auth=auth_token,
        SignKey=priv_key
    ))