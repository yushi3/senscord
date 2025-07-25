FROM mcr.microsoft.com/devcontainers/base:ubuntu-24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
	cmake \
    libxml2-dev

RUN apt-get update && apt-get install -y \
    openjdk-17-jre

RUN sudo sudo apt-get install -y ruby-full
RUN sudo gem install asciidoctor
RUN sudo gem install asciidoctor-diagram
RUN sudo gem install asciidoctor-diagram-plantuml
