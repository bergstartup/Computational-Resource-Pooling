FROM ubuntu:18.04
  
COPY idle /usr/
COPY idleStarter.sh /usr/
RUN chmod +x idleStarter.sh

WORKDIR /usr/
CMD ./idleStarter.sh
