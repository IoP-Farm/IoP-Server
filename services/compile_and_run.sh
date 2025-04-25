cd command_phone
sh command.sh
cd ..

cd control_phone_config
sh config.sh
cd ..

cd data_server_farm
sh data.sh
cd ..

cd logs_to_phone
sh logs.sh
cd ..

./command_phone/COMMAND &
./control_phone_config/CONFIG &
./data_server_farm/DATA &
./logs_to_phone/LOGS &
