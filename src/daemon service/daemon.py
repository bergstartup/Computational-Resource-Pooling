"""
Daemon script
1. Maintain IDLE/BUSY status of machine
2. UDP port 8000 listen for inter-Dameon communication
3. Membership maitainer, process completion, Job tracker and scheduler
"""

##Libraries import
import socket
import pickle
import threading
import time
from global import *
from class_decls import *
from assist_func import *
from handler import *


##Function decl
#Udp listner
def UDPListner():
    sock=socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
    addr=("127.0.0.1",8000)
    sock.bind(addr)
    global ControllerQueue
    while True:
        recv_addr,data=sock.recvfrom(1024)
        obj=GetDetails(data,addr)
        ControllerQueue.append(obj)


#Controller architecture, see reference
def Controller():
    global ControllerQueue
    global HandlerFunctions
    while True:
        if len(ControllerQueue)!=0:
            obj=ControllerQueue[0]
            CreateThread(HandlerFunctions[obj.pid],obj.msg)
            del ControllerQueue[0]
        else:
            #Preventing busy wait
            time.sleep(1)


#Main function
