# DATUM gateway (OCEAN, MIT licence), built from a pinned tag.
# Build context: packaging/docker/knots-legacy-datum
FROM debian:bookworm-slim AS builder

ARG DATUM_VERSION=v0.5.1beta

RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential cmake pkgconf git ca-certificates \
        libjansson-dev libmicrohttpd-dev libsodium-dev libcurl4-openssl-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /build
RUN git clone --depth 1 --branch "${DATUM_VERSION}" https://github.com/OCEAN-xyz/datum_gateway.git . \
    && cmake -DCMAKE_BUILD_TYPE=Release . \
    && make -j"$(nproc)"

FROM debian:bookworm-slim

RUN apt-get update && apt-get install -y --no-install-recommends \
        ca-certificates tini jq openssl netcat-openbsd \
        libjansson4 libmicrohttpd12 libsodium23 libcurl4 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY --from=builder /build/datum_gateway /app/datum_gateway
COPY --from=builder /build/www /app/www
COPY entrypoint.sh /usr/local/bin/entrypoint.sh
RUN chmod 0755 /usr/local/bin/entrypoint.sh

# /config  config.json (edited by the gateway dashboard) and the admin password
# /shared  RPC credentials from the node container (read only)
VOLUME ["/config", "/shared"]
EXPOSE 23334 7152

HEALTHCHECK --interval=30s --timeout=5s --start-period=60s --retries=3 \
    CMD nc -z 127.0.0.1 7152 || exit 1

ENTRYPOINT ["/usr/bin/tini", "--", "/usr/local/bin/entrypoint.sh"]
