from flask import Flask

app = Flask(__name__)

@app.route("/")
def home():
    return "byebye\n"

app.run(host = "127.0.0.1", port = 40000)