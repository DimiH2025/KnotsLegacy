# Knots Legacy

Knots Legacy is a Bitcoin full node with a DATUM gateway for mining on your own block templates.

## First steps

1. **Set your payout address.** Run the action **Set Payout Address** (Actions tab) and enter the Bitcoin address that should receive your mining rewards. The DATUM gateway starts after you do.
2. **Wait for the first sync.** The node downloads the whole blockchain. This takes a while and needs a large SSD. Mining only works once the node is fully synced.
3. **Point your miner at the gateway.** Open the **Stratum** interface on the Dashboard tab and use its address, for example `stratum+tcp://<your-server>.local:23334`. Any worker name works.
4. **Open the DATUM Dashboard** to see workers and hashrate. Run **Show Dashboard Password** for the password.

## Things to know

- Mining at home is a lottery. One rig has a very small chance of finding a block. With DATUM pooled mining, payouts follow the pool's rules.
- The node keeps a transaction index, which DATUM needs, so this package does not run pruned.
- Back up this service to keep your settings and wallet. The blockchain itself is not backed up and is downloaded again on restore.
- If you also run Bitcoin Core or Bitcoin Knots on this server, give the other node a different peer port or stop it first.
