FROM alpine:3.10

LABEL description="This is a cc65 Docker container intended to be used for build pipelines."

ARG HTTP_PROXY
ARG HTTPS_PROXY
ARG http_proxy
ARG https_proxy
ARG no_proxy
ARG NO_PROXY

ENV BUILD_DIR="/tmp" \
CC65_VERSION="V2.18" \
NULIB2_VERSION="v3.1.0" \
AC_RELEASE="v1-4-0" \
AC_VERSION="1.4.0"

RUN apk add --no-cache build-base ca-certificates

COPY ./cert.pem /usr/local/share/ca-certificates/mycert.perm

COPY cc65.tar.gz ${BUILD_DIR}/cc65.tar.gz

RUN echo "Building CC65 ${CC65_VERSION}" && \
    cd ${BUILD_DIR} && \
    tar xzf cc65.tar.gz && \
    cd cc65* && \
    env PREFIX=/usr/local make -j$(nproc) && \
    env PREFIX=/usr/local make install && \
    chmod +x /usr/local/bin/* && \
    rm -rf *

RUN echo "Adding VICE" && \ 
    apk add --repository=http://dl-cdn.alpinelinux.org/alpine/edge/testing --no-cache vice 

COPY goattracker.zip ./goattracker.zip

RUN echo "Adding goattracker" && \
    apk add sdl-dev sdl && \
    unzip goattracker.zip && \
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
