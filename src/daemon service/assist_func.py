#Create a thread
def CreateThread(func_name,arg):
    t=threading.Thread(func_name,args=(arg,))
    t.start()

#stripping details from message
def GetDetails(data,addr):
    data_list=pickle.load(data)
    pid = data[0]
    msg=data[1]
    msg.append(addr)
    Queuejob=ControllerDT(pid,msg)
    return Queuejob
