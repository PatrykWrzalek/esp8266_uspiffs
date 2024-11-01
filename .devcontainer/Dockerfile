# Start with an Ubuntu base image
FROM ubuntu:20.04

# Set non-interactive mode for apt-get
ENV DEBIAN_FRONTEND=noninteractive

# Install dependencies
RUN apt-get update && apt-get install -y \
  git \
  bison \
  libbison-dev \
  cmake \
  flex \
  libfl-dev \
  libncurses-dev \
  python3 \
  python3-pip \
  python3-venv \
  gperf \
  wget

# Create a symlink for python to point to python3
RUN ln -s /usr/bin/python3 /usr/bin/python

# Increase Git buffer size and set long timeout
RUN git config --global http.postBuffer 1048576000 \
  && git config --global http.lowSpeedLimit 1000 \
  && git config --global http.lowSpeedTime 60

# Retry mechanism to avoid clone failures
RUN mkdir -p /opt/esp \
  && cd /opt/esp \
  && (git clone --recursive https://github.com/espressif/ESP8266_RTOS_SDK.git || \
  (sleep 5 && git clone --recursive https://github.com/espressif/ESP8266_RTOS_SDK.git))
# && (git clone --recursive --depth 1 https://github.com/espressif/ESP8266_RTOS_SDK.git --jobs=1 || \
# (sleep 5 && git clone --recursive --depth 1 https://github.com/espressif/ESP8266_RTOS_SDK.git --jobs=1))

# Retry submodules in case of failure
RUN cd /opt/esp/ESP8266_RTOS_SDK \
  && (git submodule update --init --recursive || \
  (sleep 5 && git submodule update --init --recursive))

# Install Python dependencies
RUN cd /opt/esp/ESP8266_RTOS_SDK \
  && pip3 install -r requirements.txt

# Install the toolchain
RUN cd /opt/esp/ESP8266_RTOS_SDK \
  && ./install.sh

# Set environment variables
ENV IDF_PATH=/opt/esp/ESP8266_RTOS_SDK
RUN echo ". $IDF_PATH/export.sh" >> ~/.bashrc

# Define workdir
WORKDIR /workspace

# Default shell
CMD ["/bin/bash"]