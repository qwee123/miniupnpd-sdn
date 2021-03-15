from flask import Flask

app = Flask(__name__)

@app.route("/")
def home():
    return "byebye"

@app.route("/checkalive")
def checkalive():
    return "alive\n"

@app.route("/externaladdress")
def externalAddr():
    return "111.111.111.111\n"

app.run(port = 6000)
