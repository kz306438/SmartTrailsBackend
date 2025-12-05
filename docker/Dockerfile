FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential cmake git python3 python3-pip python3-venv \
    ca-certificates wget libssl-dev pkg-config libpq-dev curl unzip \
    software-properties-common osmium-tool postgresql-client \
    && rm -rf /var/lib/apt/lists/*

RUN python3 -m venv /opt/venv
ENV PATH="/opt/venv/bin:$PATH"

RUN pip install --no-cache-dir "conan>=2.0,<3.0"
RUN conan profile detect --force

ENV CONAN_USER_HOME=/root/.conan
VOLUME /root/.conan

WORKDIR /app

EXPOSE 8080

CMD ["bash"]
