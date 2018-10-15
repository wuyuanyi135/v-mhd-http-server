FROM ubuntu
EXPOSE 3000
RUN apt update && apt install -y build-essential wget tar

WORKDIR /root
RUN wget https://ftp.gnu.org/gnu/libmicrohttpd/libmicrohttpd-latest.tar.gz 
RUN tar zxvf libmicrohttpd-latest.tar.gz \
    && cd libmicrohttpd-* \
    && ./configure \
    && make install \ 
    && rm /root/libmicrohttpd-latest.tar.gz \ 
    && rm /root/libmicrohttpd-latest -rf

RUN ldconfig /usr/local/lib
ADD . .
# CMD /bin/bash
CMD make execute
