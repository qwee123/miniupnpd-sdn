import socket

bind_ip = "192.168.1.11"
bind_port = 40000

server = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
server.setsockopt(socket.SOL_SOCKET, socket.SO_BINDTODEVICE, str('wan2').encode(('utf-8')))

server.bind((bind_ip,bind_port))

server.listen(5)

print("[*] Listening on %s:%d " % (bind_ip,bind_port))

while True:
    client,addr = server.accept()
    print('Connected by ', addr)

    while True:
        data = client.recv(1024)
        print("Client recv data : %s " % (data))

        client.send("ACK!")
