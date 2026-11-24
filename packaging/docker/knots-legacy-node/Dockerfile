# Knots Legacy node (headless bitcoind) for appliance packages (Umbrel, StartOS).
# Build context: the repository root.
#   docker build -f packaging/docker/knots-legacy-node/Dockerfile -t knots-legacy-node .
FROM debian:bookworm-slim AS builder

RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential cmake pkgconf ca-certificates git python3 \
        libboost-dev libevent-dev libsqlite3-dev libzmq3-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src
COPY . .

# Headless build: no GUI, no tests, no benchmarks. ZMQ is on because the
# DATUM template-hint notification uses it.
RUN cmake -B build \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_GUI=OFF -DBUILD_TESTS=OFF -DBUILD_BENCH=OFF -DBUILD_FUZZ_BINARY=OFF \
        -DBUILD_TX=OFF -DBUILD_UTIL=OFF -DBUILD_WALLET_TOOL=OFF \
        -DENABLE_WALLET=ON -DWITH_ZMQ=ON -DINSTALL_MAN=OFF -DWITH_CCACHE=OFF \
    && cmake --build build -j"$(nproc)" --target bitcoind bitcoin-cli \
    && strip build/bin/bitcoind build/bin/bitcoin-cli

FROM debian:bookworm-slim

RUN apt-get update && apt-get install -y --no-install-recommends \
        ca-certificates openssl tini libevent-2.1-7 libevent-pthreads-2.1-7 libsqlite3-0 libzmq5 libstdc++6 \
    && rm -rf /var/lib/apt/lists/*

COPY --from=builder /src/build/bin/bitcoind /usr/local/bin/bitcoind
COPY --from=builder /src/build/bin/bitcoin-cli /usr/local/bin/bitcoin-cli
COPY packaging/docker/knots-legacy-node/entrypoint.sh /usr/local/bin/entrypoint.sh
COPY packaging/docker/knots-legacy-node/healthcheck.sh /usr/local/bin/healthcheck.sh
RUN chmod 0755 /usr/local/bin/entrypoint.sh /usr/local/bin/healthcheck.sh

# /data    chain data and bitcoin.conf (optional)
# /shared  RPC credentials written for the DATUM gateway container
VOLUME ["/data", "/shared"]
EXPOSE 8333 8332

ENTRYPOINT ["/usr/bin/tini", "--", "/usr/local/bin/entrypoint.sh"]
