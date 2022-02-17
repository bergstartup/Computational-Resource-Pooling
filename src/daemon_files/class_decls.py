##Class declarations
#Machine state maintainace for membeship
class Machine:
    def __init__(self):
        self.mid=0
        self.addr=""
        self.idle=False
        self.LastHeartBeat=0
        self.SubmittedJobs=0
    def isIdle(self):
        pass

    def getMid(self):
        pass


#Data structure for Submitted Process, orgininated local submitted remote
class SubmittedProcess:
    def __init__(self,id,port,dir,clones_alive,clones_addr):
        self.id=id
        self.port=port
        self.dir=dir
        self.clones_alive=clones_alive
        self.clones_addr=clones_addr
        self.is_completed=False


#Data structure for Executinfg process, orginated remote submitted local
class RemoteProcess:
    def __init__(self,id,addr,path):
        self.id=id
        self.issue_addr=addr #Must contain addr and port
        self.exec_path=path
        self.is_completed=False


#Data structure for controller queue
class ControllerDT:
    def __init__(self,pid,parameter):
        self.pid=pid
        self.parameters=parameter
