make all
port=5678
clients=3
echo -e "starting gateway "
./sensor_gateway $port $clients &
sleep 30
killall sensor_gateway
