FROM ubuntu:18.04

COPY idle /usr/
COPY Idle_starter.sh /usr/
COPY process /usr/

WORKDIR /usr/
RUN chmod +x Idle_starter.sh
CMD ./Idle_starter.sh
