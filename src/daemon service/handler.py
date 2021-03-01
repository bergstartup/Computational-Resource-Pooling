#Handlers
#Local job issue handler
def HandleLocalJob(arg):
    global PORTS
    path=arg[0]
    exec=arg[1]
    no_of_replicas=arg[2]
    local_addr=arg[3]
    #Assign a port to listen
    #Only 10 remote process could exec at a time
    assigned_port=-1
    for i in range(10000,10011):
        if i not in PORTS:
            assigned_port=i
            break
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.sendto(assigned_port,local_addr)
    sock.close()
    if assigned_port==-1:
        return None
    #scheduler
    ips=[]#Need to have no_of_replicas ips
    #Migrate
    #Make a SubmittedProcess object
    obj=SubmittedProcess(id,assigned_port,path,0,ips)
    global SubmittedJobsQueue
    SubmittedJobsQueue.append(obj)


#Remote job handler
def HandleRemoteJob(arg):
    pass
