from flask import Flask

app = Flask(__name__)

@app.route("/")
def home():
    return "This is a test server for upnpd-igd sdn project.\n"

app.run(host = "0.0.0.0", port = 2000)