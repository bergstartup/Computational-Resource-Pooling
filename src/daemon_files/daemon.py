"""
About daemon script
1. Maintain IDLE/BUSY status of machine
2. UDP port 8000 listen for inter-Dameon communication
3. Membership maitainer, process completion, Job tracker and scheduler
"""

"""
Libraries imports
1.External
2.User-defined
"""
####Externa;####
#For socket programming used for udp listner @port 8000
import socket
#To pickle list and send through socket
import pickle
#To include thread based parallelism
import threading
#For delays
import time

####User defined####
#Global variable declarations
from global import *
#Class declarations
from class_decls import *


"""
Function decl
1. Helper functions
2. Handlers
3. Threads
"""
#######Helper#######
#Create a thread
def CreateThread(func_name,arg):
    t=threading.Thread(func_name,args=(arg,))
    t.start()

#Stripping details from message
def GetDetails(data,addr):
    data_list=pickle.load(data)
    pid = data[0]
    msg = data[1]
    msg.append(addr)
    Queuejob = ControllerDT(pid,msg)
    return Queuejob



#######Handlers#######
#Local job issue handler
def HandleLocalJob(arg):
    global PORTS
    path=arg[0]
    exec=arg[1]
    no_of_replicas=arg[2]
    local_addr=arg[3]
    #Assign a port to listen
    #Only 10 remote process could exec at a time
    syscall_listner_port=-1
    for i in range(10000,10011):
        if i not in PORTS:
            syscall_listner_port=i
            break

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.sendto(syscall_listner_port,local_addr)
    sock.close()
    if assigned_port==-1:
        return None
    #Generate ID
    #scheduler
    ips=[]#Need to have no_of_replicas ips
    #Migrate
    #Make a SubmittedProcess object
    obj=SubmittedProcess(id,assigned_port,path,0,ips)
    global SubmittedJobsQueue
    SubmittedJobsQueue.append(obj)
    #Send back port number to the issue_init script

#Remote job handler
def HandleRemoteJob(arg):
    pass



#######Threads#######
#UDP listner @port 8000
def UDPListner(port=8000):
    sock=socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
    addr=("127.0.0.1",port)
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
UDPListner()
Controller()
