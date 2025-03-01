FROM ubuntu:22.04 AS base
MAINTAINER Zane Fink <zanef2@illinois.edu>
WORKDIR /
ENV DEBIAN_FRONTEND=noninteractive
RUN ln -fs /usr/share/zoneinfo/UTC /etc/localtime && \
    echo "Etc/UTC" > /etc/timezone && \
    apt-get update && \
    apt-get install -y python3 gnupg curl software-properties-common cmake vim git zip wget unzip xz-utils autoconf automake libssl-dev unzip libcurl4-openssl-dev xz-utils patch bzip2 gfortran file \
      build-essential gdb lcov pkg-config \
      libbz2-dev libffi-dev libgdbm-dev libgdbm-compat-dev liblzma-dev \
      libncurses5-dev libreadline6-dev libsqlite3-dev libssl-dev \
      lzma lzma-dev tk-dev uuid-dev zlib1g-dev libmpdec-dev && \
    apt-get upgrade -y && \
    apt-get clean
RUN \
	git clone https://github.com/python/cpython.git && \
	cd cpython && \
	git checkout v3.13.1 && \
	./configure --with-pydebug --enable-shared --prefix=/cpython-install && \
	make install -j4

env LD_LIBRARY_PATH=/cpython-install/lib:$LD_LIBRARY_PATH
env PATH=/cpython-install/bin:$PATH

RUN \
	git clone https://github.com/ZwFink/greenlet.git /greenlet &&\
	cd /greenlet && python3 -m pip install . && \
	git clone https://github.com/ZwFink/sauerkraut.git /sauerkraut &&\
	python3 -m pip install numpy && \
	cd /sauerkraut && python3 -m pip install .

ENTRYPOINT ["/entrypoint.sh"]
CMD ["/bin/bash"]
