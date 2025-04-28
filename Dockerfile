FROM golang:1.22.5-alpine AS builder
ARG SVC
ARG VERSION
ARG COMMIT
ARG TIME

WORKDIR /go/src/github.com/MainfluxLabs/mainflux
COPY . .
RUN apk update \
    && apk add make\
    && make $SVC \
    && mv build/mainfluxlabs-$SVC /exe