import socket

HOST = '0.0.0.0'  # Слушаем все интерфейсы
PORT = 5000        # Порт из вашего кода ESP32

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind((HOST, PORT))
    s.listen()
    print(f"[+] Server started on port {PORT}")
    
    while True:
        conn, addr = s.accept()
        with conn:
            print(f"\nConnected by {addr}")
            while True:
                data = conn.recv(1024)
                if not data:
                    break
                message = data.decode('utf-8').strip()
                print(f"Received: {message}")
                conn.sendall(b"ACK")  # Отправляем подтверждение