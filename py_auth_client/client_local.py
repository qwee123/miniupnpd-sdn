from upnpy_auth import upnpy
import urllib3
import requests
import os

if __name__ == "__main__":

    urllib3.disable_warnings(category=urllib3.exceptions.SubjectAltNameWarning) 
    comm_scheme = "https://"

    try:
        auth_server_addr = os.environ['auth_server_addr']
    except KeyError:
        print("Please provide the address of the auth_server through environment variables before proceeding.")
        exit(-1)

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
    r = requests.post(comm_scheme+auth_server_addr+"/applyToken", data=user_cred, verify=False)    
    auth_token = r.text

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
        NewExternalPort=55555,
        NewProtocol="tcp",
        NewInternalPort=60000,
        NewInternalClient="172.30.2.1",
        NewEnabled=True,
        NewPortMappingDescription="",
        NewLeaseDuration=10000,
        Auth=auth_token,
        SignKey=priv_key
    ))