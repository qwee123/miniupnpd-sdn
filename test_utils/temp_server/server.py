from flask import Flask

app = Flask(__name__)

@app.route("/")
def home():
    return "byebye\n"

@app.route("/checkalive")
def checkalive():
    return "alive\n"

@app.route("/extipaddr")
def externalAddr():
    #According to the json standards, use double quote.
    return "\
        {\
            \"ext_ip_addr\": \"222.111.111.111\"\
        }"

@app.route("/wanstatus")
def wanstatus():
    return "\
        {\
            \"wan_status\": \"connected\"\
        }"

@app.route("/runtime/iface")
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
