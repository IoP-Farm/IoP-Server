#!/usr/bin/env python3
import socket
from threading import Thread

def handle_client(conn, addr):
    print(f"\n[+] Подключение от {addr}")
    try:
        while True:
            data = conn.recv(1024)
            if not data:
                break
            
            print(f"Получено от {addr}:")
            try:
                # Пробуем декодировать как текст
                text_data = data.decode('utf-8').strip()
                print(text_data)
                
                # Можно добавить анализ содержимого
                if "GET /" in text_data:
                    print("Обнаружен HTTP-запрос")
                elif "MQTT" in text_data:
                    print("Обнаружен MQTT-пакет")
                    
            except UnicodeDecodeError:
                # Если данные бинарные
                print(f"Бинарные данные ({len(data)} байт):")
                print(data.hex(' ', 1))
                
    except Exception as e:
        print(f"Ошибка с клиентом {addr}: {e}")
    finally:
        conn.close()
        print(f"[-] Отключение {addr}")

def start_server():
    host = '0.0.0.0'  # Слушаем все интерфейсы
    port = 1488
    
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s.bind((host, port))
        s.listen(5)
        
        print(f"[*] Слушаем TCP-порт {port}...")
        
        while True:
            conn, addr = s.accept()
            client_thread = Thread(target=handle_client, args=(conn, addr))
            client_thread.daemon = True
            client_thread.start()

if __name__ == '__main__':
    start_server()
