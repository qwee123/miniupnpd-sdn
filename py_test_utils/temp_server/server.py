from flask import Flask

app = Flask(__name__)

@app.route("/")
def home():
    return "byebye\n"

@app.route("/portmapping/index", methods=['GET'])
def getPortMappingIndex():
    return "\
        {\
            \"eport\":7878,\
            \"rhost\":\"*\",\
            \"iaddr\":\"56.52.2.1\",\
            \"iport\":556,\
            \"proto\":\"udp\",\
            \"duration\":4294967295\
        }"

@app.route("/portmapping", methods=['GET'])
def getPortMapping():
    return "\
        {\
            \"eport\":5642,\
            \"rhost\":\"*\",\
            \"iaddr\":\"56.52.2.1\",\
            \"iport\":556,\
            \"proto\":\"udp\",\
            \"duration\":4294967295\
        }"

@app.route("/portmapping/portrange", methods=['GET'])
def getPortMappingRange():
    return "\
        {\
            \"portmapping\":[\
                {\"eport\":222, \"rhost\":\"*\", \"iaddr\":\"1.1.1.1\", \"iport\":22222, \"proto\":\"tcp\", \"duration\":123123},\
                {\"eport\":333, \"rhost\":\"*\", \"iaddr\":\"2.2.2.1\", \"iport\":5532, \"proto\":\"udp\", \"duration\":4294967295}\
                ]\
        }"

'''
Success: 0
Failure: -1
Conflicted: -2
Existed: -3
NoAvailable: -4
'''
@app.route("/portmapping", methods=['POST'])
def addPortMapping():
    return "\
        {\
            \"return_code\": 0,\
            \"eport\": 36\
        }"

'''
Success: 0
Failure: -1
'''
@app.route("/portmapping", methods=['DELETE'])
def deletePortMapping():
    return "\
        {\
            \"return_code\": 0\
        }"

@app.route("/portmapping/portrange", methods=['DELETE'])
def deletePortMappingRange():
    return "\
        {\
            \"success\":[\
                10, 12, 16, 25\
            ],\
            \"failure\":[\
                66\
            ]\
        }"

@app.route("/checkalive")
def checkalive():
    return "alive\n"

@app.route("/stats/extipaddr")
def externalAddr():
    #According to the json standards, use double quote.
    return "\
        {\
            \"ext_ip_addr\": \"222.111.111.111\"\
        }"

@app.route("/stats/wanconnstatus")
def wanstatus():
    return "\
        {\
            \"wan_conn_status\": \"connected\"\
        }"

@app.route("/stats/iface")
def runtimeiface():
    return "\
        {\
            \"iface_status\": \"UP\",\
            \"baudrate\": 100,\
            \"total_bytes_sent\": 99,\
            \"total_bytes_received\": 888,\
            \"total_packets_sent\": 5,\
            \"total_packets_received\": 21\
        }"

app.run(host = "0.0.0.0", port = 60000)
