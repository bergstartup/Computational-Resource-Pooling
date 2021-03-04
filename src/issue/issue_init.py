"""
About issue_init
1.Gets initiated from CLI in issue system
2.It gets file path to exec and other preference
3.Send udp packet with required details to local:8000
4.Recv the port to execute and run issue_init.py with port recv as args
script
issue -e -r -d //e for exec file, r for no of replicas,d for debug
"""

"""
Libraries imports
External only
"""
import sys
import socket
import pickle
import getopt

"""
Function descriptions
"""
def ParseCMDArgs(args):
    pass


#Main func
parameters=sys.argv[1:]
parsedData=ParseCMDArgs(parameters)#execPath,noReplica,debug
sock=socket.socket(socket.AF_INET,socket.SOCK_DGRAM)

msg=pickle.save(parsedData)
recv_addr=("127.0.0.1",8000)
sock.send_to(msg,recv_addr)
_,data=sock.recvfrom(1024)

if data is -1:
    print("System has exhauted its quota of remote processes")
    exit(0)
