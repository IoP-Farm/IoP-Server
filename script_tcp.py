import socket

# Настройки сервера
HOST = '0.0.0.0'  # Локальный хост
PORT = 5000        # Порт для прослушивания
FILE_NAME = 'output.txt'  # Файл, в который будут записываться данные

# Создаем TCP-сокет
server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_socket.bind((HOST, PORT))
server_socket.listen(10)  # Ожидаем одно подключение

print(f"Сервер запущен на {HOST}:{PORT}. Ожидание подключения...")

while True:
    # Принимаем подключение
    client_socket, client_address = server_socket.accept()
    print(f"Подключен клиент: {client_address}")

    while True:
        # Получаем данные от клиента
        data = client_socket.recv(1024).decode('utf-8')
        if data:
            print(f"Получены данные: {data}")
            # Дописываем данные в файл
            with open(FILE_NAME, 'a') as file:
                file.write(data + '\n')
            print(data)
        # Отправляем подтверждение клиенту
            client_socket.send("Данные успешно записаны!".encode('utf-8'))
        else:
            client_socket.send("Ошибка: данные не получены.".encode('utf-8'))

    # Закрываем соединение с клиентом
    client_socket.close()
