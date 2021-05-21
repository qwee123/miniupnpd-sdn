from upnpy_auth import upnpy
import urllib3
import requests

if __name__ == "__main__":

    urllib3.disable_warnings(category=urllib3.exceptions.SubjectAltNameWarning) 
    comm_scheme = "https://"
    auth_server_addr = "127.0.0.1:50000"

    with open("user_password.txt", "r") as f:
        auth_password = f.read()
        f.close()

    with open("rs256.key.pub", "r") as f:
        pub_key = f.read()
        f.close()

    user_cred = {
        'username': 'user1',
        'password': auth_password,
        'pubkey': pub_key
    }
    r = requests.post(comm_scheme+auth_server_addr+"/applyToken", data=user_cred, verify='./cert.pem')    
    user_token = r.text

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