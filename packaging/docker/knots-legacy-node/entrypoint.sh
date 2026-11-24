#!/bin/sh
# Starts bitcoind with the DATUM preset and generates RPC credentials on first run.
# The credentials are shared with the DATUM gateway through /shared/rpc.env
# (a file, never a command-line argument visible to other containers).
set -eu

DATA_DIR="${KNOTSLEGACY_DATA_DIR:-/data}"
SHARED_DIR="${KNOTSLEGACY_SHARED_DIR:-/shared}"
RPC_USER="${KNOTSLEGACY_RPC_USER:-knotslegacy}"
ENV_FILE="$SHARED_DIR/rpc.env"
AUTH_FILE="$DATA_DIR/.rpcauth"

mkdir -p "$DATA_DIR" "$SHARED_DIR"
umask 077

# Create credentials once; later starts reuse them so the gateway keeps working.
if [ ! -s "$ENV_FILE" ] || [ ! -s "$AUTH_FILE" ]; then
    RPC_PASS="$(openssl rand -hex 32)"
    SALT="$(openssl rand -hex 16)"
    HASH="$(printf '%s' "$RPC_PASS" | openssl dgst -sha256 -hmac "$SALT" | awk '{print $NF}')"
    printf '%s:%s$%s' "$RPC_USER" "$SALT" "$HASH" > "$AUTH_FILE"
    printf 'RPC_USER=%s\nRPC_PASS=%s\n' "$RPC_USER" "$RPC_PASS" > "$ENV_FILE"
fi
# Readable by the gateway container, which runs as the same user.
chmod 0600 "$ENV_FILE" "$AUTH_FILE"

# Defaults the operator can override with extra arguments (docker command / bitcoin.conf).
# -datum turns on the settings the DATUM gateway recommends (txindex, large
# mempool, headroom for the pool coinbase). On a pruned node it skips -txindex.
exec bitcoind \
    -datadir="$DATA_DIR" \
    -datum=1 \
    -server=1 \
    -printtoconsole=1 \
    -rpcauth="$(cat "$AUTH_FILE")" \
    -rpcbind=0.0.0.0 \
    -rpcallowip=10.0.0.0/8 \
    -rpcallowip=172.16.0.0/12 \
    -rpcallowip=192.168.0.0/16 \
    -port=8333 \
    "$@"
