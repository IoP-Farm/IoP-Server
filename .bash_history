ls
cd services/
sh delete_trash.sh 
sh compile_and_run.sh 
sh delete_trash.sh 
ps -ax | grep .DATA
ps -ax | grep DATA
ps -ax | grep LOG
cd logs_to_phone/
ls
nano logs
nano logs.cpp 
ls
./LOGS 
sh logs.sh 
nano logs.cpp 
sh logs.sh 
cd ..
ls
sh delete_trash.sh 
sh compile_and_run.sh 
sh delete_trash.sh 
ls
ls
ls
cd control_phone_config/
ls
ls
./CONFIG 
./CONFIG &
disown
ls
ps
ps -a
ps -x
man ps
man ps --ppid
ps --ppid
ps -ppid
ps --help
ps --help simple
ps --help output
ps --help
ps --help all
ps
ps --ppid CONFIG
ps --ppid 298714
ps
man disown
help disown
ps
disown 298714
ps
disown CONFIG
ls
cd ..
ls
sh delete_trash.sh 
cd logs_to_phone/
ls
nohup ./LOGS &
ls
ps
cd ../data_server_farm/
nohup ./DATA &
ps
nohup ./DATA &
ls
 cd ..
ls
cd control_phone_config/
nohup ./CONFIG &
ls
cd ..
cd command_phone/
nohup ./COMMAND &
cd ..
ls
cd data_server_farm/
ls
nano data.cpp
ls
nohup ./DATA &
ls
sh data.sh 
nohup ./DATA &
ps
ps
ps -ax
ls
exit
ls
cd services/
ls
cd data_server_farm/
./DATA
ps
rm data.db 
./DATA
top
ps | grep DATA
ps -ax| grep DATA
top
ls
cd services/
ls
cd data_server_farm/
ls
nohup DATA &
nohup ./DATA &
ps
ps
ps -ax | grep CONF
ps -ax | grep DA
ls
top
cd services/
cd data_server_farm/
sqlite3 data.db 
ls
sqlite3 data.db 
nano /var/log/data_to_phone.log 
tail -f /var/log/data_to_phone.log 
ls
top
ps
ps -ax | grep DATA 
kill 299145
ls
nano data.cpp
sh data.sh 
nano data.cpp
sh data.sh 
ps
top
ps -ax | grep DATA
ls
ls
ps -ax | grep DAT
l
top
free
vmstat
ps -ax | grep DAT
ps -p 300407 v
ls
history
exit
// Добавляем транспорт Serial
        auto serialTransport = std::make_shared<SerialTransport>();
        logger->addTransport(serialTransport);
        
        // Добавляем транспорт MQTT
        auto mqttTransport = std::make_shared<MQTTLogTransport>();
        logger
mosquitto_sub -h <103.137.250.154> -p <1883> -t "/farm001/log" -v
mosquitto_sub -h 103.137.250.154 -p 1883 -t "/farm001/log" -v
mosquitto_sub -h 103.137.250.154 -p 1883 -t "/farm001/log"
ls
ls
ls -la
git pull
git
git status
git pull
ls
mosquitto_sub -h 103.137.250.154 -p 1883 -t "/farm001/log"
   chmod -R u+w .git
ды
ls
git pull
git push
git commit
mkdir farm_logs
cd farm_logs/
cat > start.sh
ls
nano start.sh 
ls
sh start.sh 
cd ..
cd farm_logs && sh start.sh
clear
cd farm_logs && sh start.sh
sh start.sh 
clear
sh start.sh 
cd farm_logs/
sh start.sh 
ls
cd farm_logs/
sh start.sh 
cd farm_logs/
sh start.sh 
cd farm_logs/
sh start.sh 
cd farm_logs/
sh start.sh 
cd farm_logs/
sh start.sh 
cd farm_logs/
sh start.sh 
sh
sudo
sudo apt get
ipconfig
exit
nmap -sn 192.168.241.0/24
cd farm_logs/ && sh start.sh
ls
cd farm_logs/
ls
учше
exit
cd farm_logs/ && sh start.sh
cd farm_logs/ && sh start.sh
cd farm_logs/ && sh start.sh ; exit
cd farm_logs/ && sh start.sh ; exit
ls
cd farm_logs/
sh start.sh 
cd farm_logs/ && sh start.sh ; exit
cd farm_logs/ && sh start.sh ; exit
cd farm_logs/ && sh start.sh
cd farm_logs/ && sh start.sh
cd farm_logs/ && sh start.sh ; exit
cd farm_logs/ && sh start.sh
cd farm_logs/ && sh start.sh ; exit
cd farm_logs/ && sh start.sh
ls
mosquitto_pub -h localhost -t test -m "Hello, world!"
mosquitto_sub -h localhost -t /farm001/data
mosquitto_sub -h localhost -t /farm001/logs
mosquitto_sub -h localhost -t /farm001/log
cd farm_logs/ && sh start.sh
mosquitto_sub -h localhost -t test
ls
history
echo '{"command" : 2}' | nc localhost 1490
echo '{"command" : 2}' | nc localhost 1489
cd services/
ls
cd control_phone_config/
ls
ls
ps -ax | grep CONF
kill 299069
ps -ax | grep CONF
nohup ./CONFIG &
echo '{"command" : 2}' | nc localhost 1490
echo '{"command" : 2}' | nc localhost 1489
top
ls

учше
exit
cd services/control_phone_config/
ls
nano config.cpp
nano "/var/log/phone_command.log"
systemctl status mosquitto.service 
nano /var/log/
cd /var/log
ls
cd mosquitto/
ls
nano mosquitto.log
echo "{DSA}" | nc localhost 1489
echo "{DSA}" | nc localhost 1490
echo "{DSA}" | nc localhost 1490
echo "{DSA}" | nc localhost 1489
top
echo "{DSA}" | nc localhost 1490
echo "{DSA}" | nc localhost 1489
echo "{DSA}" | nc localhost 1489
echo "{DSA}" | nc localhost 1490
mosquitto_pub -h localhost -t /test -m "dsad"
echo "{DSA}" | nc localhost 1489
echo "{DSA}" | nc localhost 1490
mosquitto_pub -h localhost -t /farm001/command -m "dsad"
ls
cd ..
cd /home/tovarichkek/services/command_phone/
ls
nano command.cpp
echo "{DSA}" | nc localhost 1490
netstat -tulnp | grep 1489
netstat -tulnp | grep 1490
netstat -tulnp | grep 1488
echo {"command":2} | nc localhost 1490
echo '{"command":2}' | nc localhost 1490
echo '{"command":2}' | nc localhost 1490
echo '{"command":2}' | nc localhost 1490
echo '{"command":2}' | nc localhost 1490
echo '{"command":2}' | nc localhost 1490
echo '{"command":2}' | nc localhost 1490
telnet localhost 1883
mosquitto_pub -h localhost -t "/farm001/command" -m "test"
mosquitto_pub -h localhost -t "/farm001/command" -m "test"
mosquitto_pub -h localhost -t "/farm001/command" -m "test"
top
mosquitto_pub -h localhost -t "/farm001/command" -m "test"
mosquitto_sub -h localhost -t "/farm001/command" 
mosquitto_pub -h localhost -t "/farm001/command" -m "test"
echo '{"command":2}' | nc localhost 1490
mosquitto_sub -h localhost -t "#" -v
vX6pR6kH4enJ
history 10000 | grep 1448
history 1000 | grep 1448
echo "IDI NAHUI" | nc localhost 1448
echo '{"unix_time_from":0,  | nc localhost 1448

 echo '{"unix_time_from":0, "unix_time_to":100000}' | nc localhost 1448
 echo '{"unix_time_from":0, "unix_time_to":10000000000}' | nc localhost 1448
echo '{"unix_from": 1622500000, "unix_time_to": 1625100000}' | nc localhost 1488
echo '{"unix_from": 0, "unix_time_to": 1625100000000}' | nc localhost 1488
echo '{"unix_from": 1746369041, "unix_time_to": 1746369081}' | nc localhost 1488
top
echo '{"unix_time_from": 1746369041, "unix_time_to": 1746369081}' | nc localhost 1488
echo '{"comm": 2, "unix_time_to": 1746369081}' | nc localhost 1489
echo '{"comm": 2, "unix_time_to": 1746369081}' | nc localhost 1490
echo '{"comm": 2, "unix_time_to": 1746369081}' | nc localhost 1490
echo '{"comm": 2, "unix_time_to": 1746369081}' | nc localhost 1489
echo '{
  "pump_interval_days": 2,
  "pump_start": "08:00",
  "pump_volume_ml": 350,

  "heatlamp_target_temp": 28,

  "growlight_on": "07:00",
  "growlight_off": "21:00"
}' | nc localhost 1489
history 1000 | grep 1489
echo '{ "pump_interval_days": 2,"pump_start": "08:00","pump_volume_ml": 350, "heatlamp_target_temp": 28, "growlight_on": "07:00", "growlight_off": "21:00"}' | nc localhost 1489
echo '{ "pump_interval_days": 2,"pump_start": "08:00","pump_volume_ml": 350, "heatlamp_target_temp": 28, "growlight_on": "07:00", "growlight_off": "21:00"}' | nc localhost 1489
echo '{ "pump_interval_days": 2,"pump_start": "08:00","pump_volume_ml": 350, "heatlamp_target_temp": 28, "growlight_on": "07:00", "growlight_off": "21:00"}' | nc localhost 1490
ps -ax | grep CONF
echo '{"command":34}' > nc localhost 1490
echo '{"command":34}' | nc localhost 1490
mosquitto_pub localhost /farm001/command -m "test"
mosquitto_pub -h localhost -t /farm001/command -m "test"
ls
top
ls
rm *.pcapng
ls
rm *.pcapng
ls
cd nc
rm nc
ls
rm script.
rm script.cpp 
rm script.py 
ls
cd services/
ls
mosquitto_pub -h localhost -p 1883 -t "/farm001/data" -m '{
  "temperature_DHT22": 25.3,
  "temperature_DS18B20": 24.8,
  "humidity": 60,
  "water_level": 15,
  "soil_moisture": 45,
  "light_intensity": 80
}'
mosquitto_pub -h localhost -p 1883 -t "/farm001/data" -m '{
  "temperature_DHT22": 25.3,
  "temperature_DS18B20": 24.8,
  "humidity": 60,
  "water_level": 15,
  "soil_moisture": 45,
  "light_intensity": 80
}'
ls
cd services/data_server_farm/
ls
ps -ax | grep DA
kill 300407
nohup ./DATA &
ps -ax | grep DA
ps -ax | grep DA
ps -ax | grep DA
ps -ax | grep DA
ps -ax | grep CONF
mosquitto_pub -h localhost -p 1883 -t "/farm001/data" -m '{
  "temperature_DHT22": 25.3,
  "temperature_DS18B20": 24.8,
  "humidity": 60,
  "water_level": 15,
  "soil_moisture": 45,
  "light_intensity": 80
}'
cd ..
ls
cd logs_to_phone/
ls
cat nohup.out 
echo '{"unix_time_from":1746369041, "unix_time_to":1746369900}'
echo '{"unix_time_from":1746369041, "unix_time_to":1746369900}' | nc localhost 1488
echo '{"unix_time_from":1746369041, "unix_time_to":1746369900}' | nc localhost 1488
echo '{"unix_time_from":1746369041, "unix_time_to":1746369900}' | nc localhost 1490
echo '{"unix_time_from":1746369041, "unix_time_to":1746369900}' | nc localhost 1489
учше
ls
cd services/
ls
cd data_server_farm/
ls
sqlite3 data.db 
ls
ps -ax | grep .comm
ps -ax | grep COM
pkill ./COMMAND
ps -ax | grep COM
ps -ax | grep COM
ps -ax | grep COM
pkill COMMAND
ps -ax | grep COM
ls
cd services/
ls
cd command_phone/
ls
./COMMAND &
ls
ps -ax | grep COMM
kill 445841
nohup ./COMMAND &
ps -ax | grep COMM
ps -ax | grep COMM
ps -ax | grep COMM
ps -ax | grep COMM
ls
ls
mosquitto_sub -t "#" -v
cd farm_logs/ && sh start.sh
ls
ls -la 
cd farm_logs/ && sh start.sh ; exit
cd farm_logs/ && sh start.sh ; exit
cd farm_logs/ && sh start.sh ; exit
cd farm_logs/ && sh start.sh ; exit
cd farm_logs/ && sh start.sh ; exit
cd farm_logs/ && sh start.sh ; exit
cd farm_logs/ && sh start.sh ; exit
cd farm_logs/ && sh start.sh ; exit
cd farm_logs/ && sh start.sh ; exit
cd farm_logs/ && sh start.sh ; exit
cd farm_logs/ && sh start.sh ; exit
cd farm_logs/ && sh start.sh ; exit
сдуфк
cd farm_logs/ && sh start.sh ; exit
ls
cd services/
ls
sh delete_trash.sh 
sh compile_and_run.sh 
cd farm_logs/ && sh start.sh ; exit
history
netstat -an | findstr 1883
sc query mosquitto
Get-Service -Name mosquitto
mosquitto_pub -h localhost -t test -m "Hello, world!"
зы
ps
ports
cd services/
sh compile_and_run.sh 
sh delete_trash.sh 
sh compile_and_run.sh 
sh delete_trash.sh 
sh compile_and_run.sh 
mosquitto_sub -h localhost -t test
cd farm_logs/ && sh start.sh ; exit
учшч
ls
ps -ax | grep DATA
cd ..
ls
cd tovarichkek/
ps -ax | grep CONF
ps -ax | grep COMM
ps -aгx | grep COMM
ps -aux | grep COMM
echo '{"unix_time_from":1746369041, "unix_time_to":1746369900}' | nc localhost 1488
top
ls
cd services/
ls
history 1000 | grep mosq
mosquitto_pub -h localhost -t "/farm001/command" -m "test"
history 1000 | grep nc
echo '{ "pump_interval_days": 2,"pump_start": "08:00","pump_volume_ml": 350, "heatlamp_target_temp": 28, "growlight_on": "07:00", "growlight_off": "21:00"}' | nc localhost 1490
echo '{ "pump_interval_days": 2,"pump_start": "08:00","pump_volume_ml": 350, "heatlamp_target_temp": 28, "growlight_on": "07:00", "growlight_off": "21:00"}' | nc localhost 1489
echo '{ "pump_interval_days": 2,"pump_start": "08:00","pump_volume_ml": 350, "heatlamp_target_temp": 28, "growlight_on": "07:00", "growlight_off": "21:00"}' | nc localhost 1490
echo '{ "pump_interval_days": 2,"pump_start": "08:00","pump_volume_ml": 350, "heatlamp_target_temp": 28, "growlight_on": "07:00", "growlight_off": "21:00"}' | nc localhost 1490
top
history
cd farm_logs/ && sh start.sh ; exit
