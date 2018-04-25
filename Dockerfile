FROM ubuntu:16.04

WORKDIR /app

RUN apt-get update &&\
        apt-get install -y build-essential make git gcc mingw-w64

CMD ["sh"]
