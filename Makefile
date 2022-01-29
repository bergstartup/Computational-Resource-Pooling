inputIncrement: sample_inpIncre install
fileWrite: sample_fileWrite install
stdout: sample_stdout install
install: issue idle PoC_testing_remoteExecution PoC_testing_faultTolerance tmp_clear

sample_inpIncre:
	gcc Test_programs/inputIncrement.c -o tmp/process

sample_fileWrite:
	gcc Test_programs/fileWrite.c -o tmp/process

sample_stdout:
	gcc Test_programs/stdout.c -o tmp/process

issue:
	g++ -g -std=c++11 -pthread src/issue_stub.cpp -o tmp/issue

idle:
	gcc src/idle_stub.cpp --static -o tmp/idle

PoC_testing_remoteExecution:
	cp tmp/idle PoC_testing/RemoteExecution/IdleNode/
	cp tmp/issue PoC_testing/RemoteExecution/IssueNode/
	cp tmp/process PoC_testing/RemoteExecution/IdleNode/

PoC_testing_faultTolerance:
	cp tmp/idle PoC_testing/FaultTolerance/IdleNode/
	cp tmp/issue PoC_testing/FaultTolerance/IssueNode/
	cp tmp/process PoC_testing/FaultTolerance/IdleNode/

tmp_clear:
	rm tmp/*
