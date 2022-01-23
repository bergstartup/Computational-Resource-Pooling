FROM ubuntu:18.04
  
COPY issue /usr/
COPY issueStarter.sh /usr/
RUN chmod +x issueStarter.sh

WORKDIR /usr/
CMD ./issueStarter.sh
