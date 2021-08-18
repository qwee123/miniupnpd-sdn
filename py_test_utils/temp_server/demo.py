from flask import Flask, request
from werkzeug.serving import WSGIRequestHandler

app = Flask(__name__)

@app.route("/")
def home():
    return "byebye\n"

WSGIRequestHandler.protocol_version = "HTTP/1.1"
app.run(host = "0.0.0.0", port = 40000)