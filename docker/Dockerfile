FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN RUN apt-get update && apt-get install -y

RUN build-essential \
    cmake \
    git \
    python3 \
    python3-pip \
    ca-certificates \
    wget \
    libssl-dev \
    pkg-config \
    libpq-dev

RUN rm -rf /var/lib/apt/lists/*

RUN pip3 install --no-cache-dir "conan>=2.0,<3.0" --break-system-packages \
    && ln -s $(which conan) /usr/bin/conan

RUN conan --version && conan profile detect --force

WORKDIR /app

COPY build.sh /app/build.sh
RUN chmod +x /app/build.sh

VOLUME /root/.conan
ENV CONAN_USER_HOME=/root/.conan

COPY . /app

COPY start_app.sh /app/start_app.sh
RUN chmod +x /app/start_app.sh

EXPOSE 8080
CMD ["/app/start_app.sh"]
