import struct
import socket

DB_NAME = "mqtt_data.db"

client_port = 5003

PROPERTIES_NAMES = ["targetTemperature", "waterDelay", "water", "lightDelay", "lightTime"]
PROPERTIES_NUMBER = len(PROPERTIES_NAMES)

STATISTICS_NAMES = ["temperature", "humidity_air", "soil_humidity", "water_level", "water_flow_ml", "pump_status"]
STATISTICS_NUMBER = len(STATISTICS_NAMES)



def get_last_n_records(n=30, devid=1):
    conn = sqlite3.connect(DB_NAME)
    cursor = conn.cursor()
    cursor.execute(f"""
        SELECT device_id, temperature, humidity, water_level, timestamp
        FROM farm_data
        WHERE device_id={devid}
        ORDER BY timestamp DESC
        LIMIT {n}
    """)
    records = cursor.fetchall()
    conn.close()
    return obj2bin(records);

def get_records_for_period(timefrom, timeto, devid=1):
    conn = sqlite3.connect(DB_NAME)
    cursor = conn.cursor()
    cursor.execute(f"""
        SELECT device_id, temperature, humidity, water_level, timestamp
        FROM farm_data
        WHERE device_id={devid}
        WHERE timestamp BETWEEN {timefrom} AND {timeto}
        ORDER BY timestamp DESC
    """)
    records = cursor.fetchall()
    conn.close()
    return obj2bin(records)


def obj2bin(data):
    bytes = []
    bytes = bytes + bytearray(struct.pack("d", len(data)))
    for i in data:
        for j in range(1, len(i)):
            bytes = bytes + bytearray(struct.pack("f", i[j]))
    return bytes

#TODO: add logic to push config to farm
def proceed_config(config):
    return 30, 1

def send_bytes(connection, bytearr):
    connection.send(len(bytearr))
    connection.send(bytearr)

tcp_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
 
server_address = ('localhost', client_port)
tcp_socket.bind(server_address)
 
tcp_socket.listen(1)
 
while True:
    connection, client = tcp_socket.accept()
    try:
        #TODO: change the config bytesize to match client side's + metadata
        flag = connection.recv(1)
        #config = connection.recv(4*PROPERTIES_NUMBER)
        #n, status = proceed_config(config)
        #if status==1:
        send_bytes(get_last_n_records())
        #if status==2:
    finally:
        connection.close()
