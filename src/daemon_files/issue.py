"""
About issue_init
1.Gets initiated from CLI in issue system
2.It gets file path to exec and other preference
3.Send udp packet with required details to local:8000
4.Recv the port to execute and run issue_init.py with port recv as args
script
issue -e -r //e for exec file, r for no of replicas
"""

"""
Libraries imports
External only
"""
import sys
import socket
import pickle
import getopt
import uuid


"""
Global variables
"""
Path = ""
Exec_file = ""
ReplicaCount = 2
ID = uuid.uuid4().hex #Unique JOB ID
Print("Job ID : ",ID)
"""
Function descriptions
"""
def ParseCMDArgs(args):
    pass


#Main func
parameters = sys.argv[1:]
parsedData = ParseCMDArgs(parameters)#execFile,noReplica

#Pickle the data to send to daemon
to_send_data = [0,[ID,ReplicaCount]]
msg=pickle.save(to_send_data)
daemon_addr=("127.0.0.1",8000)
sock=socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
sock.send_to(msg,daemon_addr)

_,data=sock.recvfrom(1024)#Recv the port to listen from daemon
if data is -1:
    print("System has exhauted its quota of remote processes")
    exit(0)

#Exec the issue stub
print("Starting stub")
subprocess.Popen(["./issue_stub","-p",str(data)])
