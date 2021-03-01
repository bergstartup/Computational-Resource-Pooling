#Global declarations
ControllerQueue=[] #Contains ControllerDT objects for controller processing
SubmittedJobsQueue=[] #Contains user submitted process, class : SubmittedProcess
RemoteProcessQueue=[] #Remote process running in node, class : RemoteProcess
SystemNodes={} #Membership list
HandlerFunctions=["HandleLocalJob","HandleRemoteJob"]
PORTS=[]
