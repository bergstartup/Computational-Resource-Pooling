echo "Starting IDLE Stub and remote process"
#gcc idle_stub.cpp -o idle
./idle -e "./process" -p ${ISSUE_PORT} -h "127.0.0.1"
