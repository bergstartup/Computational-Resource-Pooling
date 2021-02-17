# Computational Resource Pooling
<br>
  A middleware that pools all compute in LAN as a virtual uni-processor. This phenomenon is achieved through remote execution. "Remote execution" goes long back to the 1980s. Many prominent systems like the Condor, DAWGS, and Butler have been made with the same idea, to provide compute resource pooling. To get to know more about proof of concept and comparison of literature read the <i>idea.pdf</i> and <i>literature_review.pdf</i>.
<br><br>
<b>Note</b><br>
This repo is under-development, since this is my final year project more time is spent on documenting rather than developing. So, will soon update my code.
<br><br><br>
<b>About Temporary script</b><br>
Temporary scripts for idle and issue machine are provided (Its a temporary scripts, soon this will be designed as a full scale service). This script also handles only file I/O. That is the remotely executing program must only access following syscalls Open, Read, Write, Close. Morover it simplify the intial migation, we can provide execution file directly to the idle script.
<br>
<b>For executing</b><br>
./issue_script [options]
<br><br>
./idle_script [options] -e ./exec_file
<br><br>
options:<br>
debug
