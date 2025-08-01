# Dockerfile - reproducible execution environment for ReproVM + polyglot helpers
FROM debian:bookworm-slim AS build

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential gcc make ca-certificates curl git tar unzip python3 \
    ruby-full \
    gnupg2 \
    && rm -rf /var/lib/apt/lists/*

# Install Node.js 20 (LTS-ish)
RUN curl -fsSL https://deb.nodesource.com/setup_20.x | bash - && \
    apt-get update && apt-get install -y nodejs && \
    rm -rf /var/lib/apt/lists/*

# Install Go (latest stable at build time)
ENV GO_VERSION=1.22.5
RUN curl -fsSL https://go.dev/dl/go${GO_VERSION}.linux-amd64.tar.gz -o /tmp/go.tar.gz && \
    tar -C /usr/local -xzf /tmp/go.tar.gz && \
    rm /tmp/go.tar.gz
ENV PATH=/usr/local/go/bin:$PATH

# Install Rust via rustup (minimal)
RUN curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y
ENV PATH=/root/.cargo/bin:$PATH

# Create workspace
WORKDIR /workspace
COPY . /workspace

# Build ReproVM serial and parallel
RUN make && \
    gcc -std=c99 -O2 -Wall -Wextra -g reprovm_parallel.c parallel_executor.c task.c cas.c util.c -lpthread -o reprovm_parallel

# Build Go cleaning utility
RUN cd scripts && \
    if [ -f clean.go ]; then /usr/local/go/bin/go build -o ../bin/clean_go clean.go; fi

# Build Rust analyzer (assumes Cargo-less single file via rustc for simplicity)
RUN cd scripts && \
    if [ -f analyze.rs ]; then rustc analyze.rs -o ../bin/analyze_rs; fi

# Install JS dependencies if needed (none here)
RUN cd scripts && \
    if [ -f report.js ]; then npm ci --prefix . || true; fi

# Final runtime image (slim)
FROM debian:bookworm-slim
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates curl tar unzip ruby-full nodejs gcc libssl-dev \
    && rm -rf /var/lib/apt/lists/*
COPY --from=build /workspace/reprovm_parallel /usr/local/bin/reprovm_parallel
COPY --from=build /workspace/reprovm /usr/local/bin/reprovm
COPY --from=build /workspace/bin /usr/local/bin
COPY --from=build /workspace/scripts /workspace/scripts
WORKDIR /workspace

# Entry point wrapper: use run_pipeline.sh if present, else default to printing help
ENTRYPOINT ["bash", "-c", "./run_pipeline.sh || echo 'Please provide manifest and run ./reprovm_parallel'"]
