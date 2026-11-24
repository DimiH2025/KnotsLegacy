#!/bin/sh
# Renders /config/config.json for the DATUM gateway and starts it.
#
# Environment (all optional):
#   KNOTSLEGACY_RPC_URL        node RPC URL            (default http://node:8332)
#   DATUM_POOL_ADDRESS      payout address; if unset, the value already in the
#                           config is kept, or a placeholder the user replaces
#                           in the dashboard (the gateway waits until it is valid)
#   DATUM_ADMIN_PASSWORD    dashboard password; if unset, one is generated once
#                           and stored in /config/admin_password
#   DATUM_COINBASE_TAG      secondary coinbase tag
set -eu

CONFIG_DIR="${DATUM_CONFIG_DIR:-/config}"
SHARED_DIR="${KNOTSLEGACY_SHARED_DIR:-/shared}"
CONFIG="$CONFIG_DIR/config.json"
PLACEHOLDER="SET-YOUR-PAYOUT-ADDRESS-IN-THE-DASHBOARD"

mkdir -p "$CONFIG_DIR"
umask 077

# Wait for the node to publish its RPC credentials.
tries=0
while [ ! -s "$SHARED_DIR/rpc.env" ]; do
    tries=$((tries + 1))
    [ "$tries" -gt 120 ] && { echo "node RPC credentials not found in $SHARED_DIR" >&2; exit 1; }
    sleep 2
done
. "$SHARED_DIR/rpc.env"

# Admin password for the dashboard.
if [ -n "${DATUM_ADMIN_PASSWORD:-}" ]; then
    ADMIN_PASS="$DATUM_ADMIN_PASSWORD"
elif [ -s "$CONFIG_DIR/admin_password" ]; then
    ADMIN_PASS="$(cat "$CONFIG_DIR/admin_password")"
else
    ADMIN_PASS="$(openssl rand -hex 12)"
    printf '%s' "$ADMIN_PASS" > "$CONFIG_DIR/admin_password"
fi

if [ ! -s "$CONFIG" ]; then
    cat > "$CONFIG" <<JSON
{
  "bitcoind": { "rpcuser": "", "rpcpassword": "", "rpcurl": "", "notify_fallback": true },
  "stratum": { "listen_port": 23334 },
  "mining": {
    "pool_address": "$PLACEHOLDER",
    "coinbase_tag_primary": "Knots Legacy",
    "coinbase_tag_secondary": "DATUM User"
  },
  "api": { "admin_password": "", "listen_port": 7152, "modify_conf": true },
  "logger": { "log_to_console": true, "log_to_file": false, "log_level_console": 2 },
  "datum": { "pool_pass_workers": true, "pool_pass_full_users": true, "pooled_mining_only": true }
}
JSON
fi

# Refresh the values this image owns on every start; keep everything the user
# changed in the dashboard.
TMP="$CONFIG.tmp"
jq \
    --arg user "$RPC_USER" --arg pass "$RPC_PASS" \
    --arg url "${KNOTSLEGACY_RPC_URL:-http://node:8332}" \
    --arg admin "$ADMIN_PASS" \
    --arg addr "${DATUM_POOL_ADDRESS:-}" \
    --arg tag "${DATUM_COINBASE_TAG:-}" \
    '.bitcoind.rpcuser = $user
     | .bitcoind.rpcpassword = $pass
     | .bitcoind.rpcurl = $url
     | .api.admin_password = $admin
     | .api.modify_conf = true
     | (if $addr != "" then .mining.pool_address = $addr else . end)
     | (if $tag  != "" then .mining.coinbase_tag_secondary = $tag else . end)' \
    "$CONFIG" > "$TMP"
mv "$TMP" "$CONFIG"

exec /app/datum_gateway --config "$CONFIG"
