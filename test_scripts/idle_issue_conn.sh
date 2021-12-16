cd ../src/
gcc issue_system_script.cpp -o issue_worker
./issue_worker -p 5000 -r 1 -d

gcc idle_system_script.cpp -o idle
./idle -e "./sample" -p 5000 -h "127.0.0.1" -d
