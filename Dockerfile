# Use an official Ubuntu base image
FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Etc/UTC

# Preconfigure the time zone and suppress all interactive prompts
RUN ln -fs /usr/share/zoneinfo/Etc/UTC /etc/localtime && \
    echo "Etc/UTC" > /etc/timezone && \
    apt-get update && apt-get install -y tzdata && \
    dpkg-reconfigure --frontend noninteractive tzdata

# Install necessary packages including g++
RUN apt-get update && apt-get install -y --no-install-recommends --assume-yes \
    git \
    build-essential \
    g++ \
    libfontconfig1-dev \
    libfreetype-dev \
    libxml2-dev \
    libssl-dev \
    libjpeg-dev \
    libpng-dev \
    libtiff-dev \
    libidn11-dev \
    ca-certificates \
    wget \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Determine architecture and set CMake version
ARG CMAKE_VERSION="3.29.4"

# Download and install CMake based on architecture
RUN ARCH=$(dpkg --print-architecture) && \
    if [ "$ARCH" = "amd64" ]; then \
        CMAKE_FILE="cmake-${CMAKE_VERSION}-linux-x86_64.sh"; \
    elif [ "$ARCH" = "arm64" ]; then \
        CMAKE_FILE="cmake-${CMAKE_VERSION}-linux-aarch64.sh"; \
    else \
        echo "Unsupported architecture: $ARCH"; exit 1; \
    fi && \
    apt-get remove --purge --auto-remove -y cmake && \
    cd /tmp && \
    wget https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/${CMAKE_FILE} && \
    chmod +x ${CMAKE_FILE} && \
    ./${CMAKE_FILE} --skip-license --prefix=/usr/local && \
    rm ${CMAKE_FILE}

# Set the working directory
WORKDIR /app

# Clone the PoDoFo repository
RUN git clone https://github.com/podofo/podofo.git /app/podofo

# Build PoDoFo library
WORKDIR /app/podofo
RUN mkdir build && cd build && \
    cmake .. && \
    make

WORKDIR /app

# Copy over the source code and test files
COPY main.cpp /app/
COPY dockerCMakeLists.txt /app/CMakeLists.txt

# Make the build directory
RUN mkdir -p build

COPY StartOutPDF.pdf /app/build

COPY checkBox_AP_off.txt /app/build
COPY checkBox_AP_on.txt /app/build
COPY checkBox_AP_on_D.txt /app/build
COPY checkBox_AP_off_D.txt /app/build

COPY radioButton_AP_no.txt /app/build
COPY radioButton_AP_yes.txt /app/build
COPY radioButton_AP_off.txt /app/build

# Build the application
WORKDIR /app/build
RUN cmake .. && \
    make \











