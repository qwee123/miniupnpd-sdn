from upnpy_auth import upnpy
import urllib3
import requests
import os
import time
import matplotlib.pyplot as plt
import logging
import datetime
import string

OUT_DIR='.'

#sdn, sdnauth, linux
testmode='sdn'

def sdn():

    upnp = upnpy.UPnP()
    before_discover = time.time()
    devices = upnp.discover()
    after_discover = time.time()
    device = upnp.get_igd()

    # We will access it like a dictionary instead:
    service = device['WANIPConn1']

    before_action = time.time()
    print("AddAnyPortMapping: ", service.AddAnyPortMapping(
        NewRemoteHost="*",
        NewExternalPort="*",
        NewProtocol="tcp",
        NewInternalPort=60001,
        NewInternalClient="172.16.0.2",
        NewEnabled=True,
        NewPortMappingDescription="",
        NewLeaseDuration=10000
    ))

    after_action = time.time()

    SSDP_SDCP=after_discover-before_discover
    soap=after_action-before_action
    return 0, SSDP_SDCP, soap

def sdn_auth():
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
    before_auth = time.time()
    r = requests.post(comm_scheme+auth_server_addr+"/applyToken", data=user_cred, verify=False)    
    after_auth = time.time()
    auth_token = r.text

    with open("rs256.key", "r") as f:
        priv_key = f.read()
        f.close()
 
    auth_token = "Bearer " + auth_token

    upnp = upnpy.UPnP()
    before_discover = time.time()
    devices = upnp.discover()
    after_discover = time.time()
    device = upnp.get_igd()

    # We will access it like a dictionary instead:
    service = device['WANIPConn1']

    before_action = time.time()
    print("AddAnyPortMapping: ", service.AddAnyPortMapping(
        NewRemoteHost="*",
        NewExternalPort="*",
        NewProtocol="tcp",
        NewInternalPort=60000,
        NewInternalClient="172.16.0.2",
        NewEnabled=True,
        NewPortMappingDescription="",
        NewLeaseDuration=10000,
        Auth=auth_token,
        SignKey=priv_key
    ))

    after_action = time.time()

    auth=after_auth-before_auth
    SSDP_SDCP=after_discover-before_discover
    soap=after_action-before_action
    return auth, SSDP_SDCP, soap

def linux():
    
    upnp = upnpy.UPnP()
    before_discover = time.time()
    devices = upnp.discover()
    after_discover = time.time()
    device = upnp.get_igd()

    # We will access it like a dictionary instead:
    service = device['WANIPConn1']

    before_action = time.time()
    print("AddAnyPortMapping: ", service.AddAnyPortMapping(
        NewRemoteHost="*",
        NewExternalPort="*",
        NewProtocol="tcp",
        NewInternalPort=60001,
        NewInternalClient="172.16.0.2",
        NewEnabled=True,
        NewPortMappingDescription="",
        NewLeaseDuration=10000
    ))

    after_action = time.time()

    SSDP_SDCP=after_discover-before_discover
    soap=after_action-before_action
    return 0, SSDP_SDCP, soap

if __name__ == "__main__":


    if testmode == 'sdn':
        first, second, third = sdn()
    elif testmode == 'sdnauth':
        first, second, third = sdn_auth()
    elif testmode == 'linux':
        first, second, third = linux()

    template = """
        testcase: $testmode
        auth: $first
        discover&get description: $second
        soap: $third
    """

    t = string.Template(template)
    result = t.substitute(
        testmode=testmode,
        first=first,
        second=second,
        third=third
    )
    print(result)
'''
    plt.bar([1, 2], [auth, auth], width=0.2, color='red')
    plt.bar([1, 2], [SSDP_SDCP, SSDP_SDCP], bottom=[auth, auth], width=0.2, color='blue')
    plt.bar([1, 2], [soap, soap], bottom=[auth+SSDP_SDCP, auth+SSDP_SDCP], width=0.2, color='green')
    plt.xticks([1,2])
    plt.ylabel('time')
    plt.xlabel('xth connection attempts')

    filename_ext = time.strftime("%Y-%m-%d-%H-%M-%S", time.localtime())
    plt.savefig(os.sep.join([OUT_DIR, filename_ext]))
'''