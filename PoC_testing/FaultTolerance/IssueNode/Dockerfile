FROM ubuntu:18.04
  
COPY issue /usr/
COPY issueStarter.sh /usr/

WORKDIR /usr/
RUN chmod +x issueStarter.sh
CMD ./issueStarter.sh
