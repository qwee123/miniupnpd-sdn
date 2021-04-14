from flask import Flask

app = Flask(__name__)

@app.route("/")
def home():
    return "byebye\n"

app.run(host = "0.0.0.0", port = 60000)