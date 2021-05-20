from flask import Flask, request
from utils import VerifyPassword
import mariadb

app = Flask(__name__)

def initDB(db_addr, db_port):
    try:
        return mariadb.connect(
            user="root",
            password=db_pass,
            host=db_addr,
            port=db_port,
            database="userdb"
        )
    except mariadb.Error as e:
        print(f"Error connecting to MariaDB Platform: {e}")
        sys.exit(1)

@app.route("/")
def home():
    return "This is a demo server to simulate a trusted authority."

@app.route("/applyToken", methods=['POST'])
def applyToken():
    if 'username' in request.form and 'password' in request.form and 'pubkey' in request.form:
        username = request.form['username']
        password = request.form['password']
        pubkey = request.form['pubkey']
    else:
        return "Bad Request", 400

    cursor = conn.cursor()
    cursor.execute("Select username, password from users where username=?", (username,))

    result = cursor.fetchall()
    if len(result) != 1 or result[0][0] != username:
        return "Error", 500
    elif VerifyPassword(result[0][1], password):
        return "Success!"
    else:
        return "login failed.", 403

# main function starts
with open("password.txt", "r") as f:
    db_pass = f.read()
    f.close()

conn = initDB("172.17.0.2", 3306)

# Only execute testcase when this script is executed directly
if __name__ == "__main__":

    with app.test_client() as client:
        print("test client.\n")
        rv = client.post('/login', data=dict(
            username="user1",
            password="testestest"
        ))

        print(rv.status_code)
        print(rv.data)