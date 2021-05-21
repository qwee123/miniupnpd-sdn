from flask import Flask, request
from utils import VerifyPassword, GenerateToken
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
    cursor.execute("Select id, username, password from users where username=?", (username,))
 
    result = cursor.fetchall()
    if len(result) != 1 or result[0][1] != username:
        return "login failed.", 403
    elif VerifyPassword(result[0][2], password):
        cursor.execute("Select permission from igdPermission where user_id=?", (result[0][0],))
        permission = cursor.fetchall()
        if len(permission) != 1:
            return "Fail to Generate token.", 500

        token = GenerateToken(permission[0][0], pubkey, signkey)
        if token == "":
            return "Fail to Generate token.", 500

        return token
    else:
        return "login failed.", 403

# main function starts
with open("password.txt", "r") as f:
    db_pass = f.read()
    f.close()

with open("ca_rs256.key", "r") as f:
    signkey = f.read()
    f.close()

conn = initDB("172.17.0.2", 3306)

# Only execute testcase when this script is executed directly
if __name__ == "__main__":

    with open("rs256.key.pub", "r") as f:
        pubkey = f.read()
        f.close()

    with app.test_client() as client:
        print("test client.\n")
        rv = client.post('/applyToken', data=dict(
            username="user1",
            password="testestest",
            pubkey=pubkey
        ))

        print(rv.status_code)
        print(rv.data)