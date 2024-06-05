FROM ubuntu:22.04

# Set the working directory
WORKDIR /app

# Install dependencies
# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    wget \
    libfontconfig1-dev \
    zlib1g-dev \
    libpodofo-dev \
    mupdf-tools \
    pkg-config

# Copy over the source code and test files
COPY main.cpp /app
COPY CMakeLists.txt /app
COPY






