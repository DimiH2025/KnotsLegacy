#!/bin/sh
# Healthy when the RPC interface answers.
set -eu
. "${KNOTSLEGACY_SHARED_DIR:-/shared}/rpc.env"
exec bitcoin-cli -rpcconnect=127.0.0.1 -rpcport=8332 -rpcuser="$RPC_USER" -rpcpassword="$RPC_PASS" getblockcount >/dev/null
