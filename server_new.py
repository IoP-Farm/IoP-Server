import socket

HOST = '0.0.0.0'
PORT = 5000

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.bind((HOST, PORT))
    s.listen()
    print(f"[+] Server started on port {PORT}")
    
    while True:
        conn, addr = s.accept()
        print(f"\nConnected by {addr}")
        try:
            while True:
                data = conn.recv(1024)
                if not data:
                    break
                message = data.decode('utf-8').strip()
                if message:
                    print(f"Received: {message}")
                    conn.sendall(b"ACK\n")  # Ответ с \n
        except:
            pass
        finally:
            conn.close()