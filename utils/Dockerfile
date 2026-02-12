FROM debian:bookworm

WORKDIR /usr/strace

RUN apt-get update && apt-get upgrade -y
RUN apt-get install gcc make strace procps -y
