install: issue idle docker_support

issue:
	cd src
	g++ -g -std=c++11 -pthread issue_stub.cpp -o issue

idle:
	gcc idle_stub.cpp -o idle

docker_support:
	cp idle ../dockerTest/idleNode
	cp issue ../dockerTest/issueNode
