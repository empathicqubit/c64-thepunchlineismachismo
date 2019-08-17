FROM alpine:3.10

LABEL description="This is a cc65 Docker container intended to be used for build pipelines."

ENV BUILD_DIR="/tmp" \
CC65_VERSION="V2.18" \
NULIB2_VERSION="v3.1.0" \
AC_RELEASE="v1-4-0" \
AC_VERSION="1.4.0"

RUN apk add --no-cache build-base

RUN echo "Building CC65 ${CC65_VERSION}" && \
    cd ${BUILD_DIR} && \
    wget https://github.com/cc65/cc65/archive/${CC65_VERSION}.tar.gz && \
    tar xzf ${CC65_VERSION}.tar.gz && \
    cd cc65* && \
    env PREFIX=/usr/local make -j$(nproc) && \
    env PREFIX=/usr/local make install && \
    chmod +x /usr/local/bin/* && \
    rm -rf *

#RUN echo "Building NuLib2 ${NULIB2_VERSION}" && \
#    cd ${BUILD_DIR} && \
#    wget https://github.com/fadden/nulib2/archive/${NULIB2_VERSION}.tar.gz && \
#    tar xzf ${NULIB2_VERSION}.tar.gz && \
#    cd nulib2* && \
#    cd nufxlib && \
#    ./configure && \
#    make -j$(nproc) && \
#    make install && \
#    cd ../nulib2 && \
#    ./configure && \
#    make -j$(nproc) && \
#    make install && \
#    echo "Cleaning up" && \
#    cd ${BUILD_DIR} && \
#    chmod +x /usr/local/bin/* && \
#    rm -rf *

RUN echo "Adding VICE" && \ 
    apk add --repository=http://dl-cdn.alpinelinux.org/alpine/edge/testing --no-cache vice 

RUN echo "Adding goattracker" && \
    apk add sdl-dev sdl && \
    wget http://csdb.dk/getinternalfile.php/180091/GoatTracker_2.75.zip && \
    unzip GoatTracker_2.75.zip && \
    cd goattrk2/src && \
    make clean && \
    cd bme && \
    make -j$(nproc) && \
    cd .. && \
    make -j$(nproc) && \
    cp -av ../linux/* /usr/local/bin && \
    cd "${BUILD_DIR}" && \
    apk del sdl-dev && \
    rm -rf *

RUN adduser -h /mnt -u 1000 -D alpine

RUN mkdir /mnt ; chown -R alpine:alpine /mnt

WORKDIR /mnt

USER alpine

CMD ["sh", "-c", "make -j`nproc`"]
