# Knots Legacy: Umbrel and StartOS packaging

This folder runs the Knots Legacy node together with the [OCEAN DATUM gateway](https://github.com/OCEAN-xyz/datum_gateway) on a home server, so a miner can point a rig at their own device and mine on block templates built by their own node.

**Status: test release.** The packages have been checked for build and syntax only. They have NOT been run on a real Umbrel or StartOS device yet. Expect small fixes after the first real installs.

## Layout

```
packaging/
  README.md                         this file
  docker/knots-legacy-node/         Dockerfile, entrypoint.sh, healthcheck.sh
  docker/knots-legacy-datum/        Dockerfile, entrypoint.sh
  umbrel/                           source copy of the Umbrel community app store
  start9/                           StartOS 0.4 package (TypeScript SDK 4.0.0)
.github/workflows/docker-images.yml builds and publishes the two images
.github/workflows/build-startos.yml builds the .s9pk files
```

## Container images

Published to `ghcr.io/dimih2025/` (lower case), tag = the release version, for example `29.3.knots20261124`:

- `knots-legacy-node`: headless `bitcoind` and `bitcoin-cli` built from this repository. The entrypoint creates RPC credentials once (rpcauth), writes them to the shared folder `/shared/rpc.env`, and starts `bitcoind -datum=1 -server=1`. DATUM needs the transaction index, so the node cannot run pruned (on a pruned node the `-datum` preset skips `-txindex` instead of stopping the node from starting).
- `knots-legacy-datum`: upstream DATUM Gateway v0.5.1beta, built from source and pinned. It reads the node's RPC login from the shared folder (read only), so no password sits on a command line. Its dashboard can edit the payout address.

Only two ports are published: stratum `23334` and peer-to-peer `8333`. RPC (`8332`) stays inside the app network.

## Release steps

1. Bump the version in `CMakeLists.txt` (`CLIENT_VERSION_SUFFIX`) and commit.
2. GitHub, **Actions**, **Container images (Umbrel / StartOS)**, **Run workflow**, enter the tag (for example `29.3.knots20261124`). Native amd64 and arm64 runners are used, no emulation.
3. After the first run only: GitHub profile, **Packages**, open both packages, **Package settings**, **Change visibility**, **Public**. Without this the home servers cannot pull the images.
4. Update the image tag in `umbrel/knotslegacy-node/docker-compose.yml` and `start9/startos/manifest/index.ts`, and the versions in `umbrel/knotslegacy-node/umbrel-app.yml` and `start9/startos/versions/current.ts`. The image tag must match the workflow tag exactly.
5. StartOS: run **Build StartOS package (.s9pk)** and download the artifacts (`knots-legacy-s9pk-x86`, `knots-legacy-s9pk-arm`; each is a zip).
6. Umbrel: copy the contents of `packaging/umbrel/` into the root of the separate repository `knotslegacy-umbrel-store` and commit.

## Install on Umbrel

1. Open the App Store, the menu at the top right, **Community App Stores**.
2. Paste the link of the `knotslegacy-umbrel-store` repository and click **Add**.
3. Install **Knots Legacy + DATUM**. Open it, sign in to the DATUM dashboard with the password shown on the app page, and set your payout address.
4. Point your miners at `stratum+tcp://<device>:23334`.

This app uses the same ports as Umbrel's own Bitcoin node apps. Do not run both.

## Install on StartOS 0.4

1. StartOS web interface, **Sideload**, upload the `.s9pk` (x86 for PCs, arm for Raspberry Pi), **Install**, **Start**.
2. Run the action **Show Dashboard Password**, open the **DATUM Dashboard** interface and sign in.
3. Run the action **Set Payout Address**.
4. Point your miners at `stratum+tcp://<device>:23334`.

## Good to know

- The first sync downloads the whole blockchain: days on a home device, on a large SSD.
- Mining at home is a lottery. With DATUM pooled mining, payouts follow the pool's rules.
- The node cannot run pruned.
- If you also run another Bitcoin node service on the same server, give it a different peer port or stop it first.

## Known limits

- Not run on real devices yet. On Umbrel check that `deterministicPassword` provides `APP_PASSWORD`, that the shipped `data/` folders are writable by user 1000, and that publishing port 8333 does not clash with other apps.
- Images are pinned by tag, not by digest. A tag can be moved by re-running the workflow.
- The `.s9pk` files are built with a throwaway signing key on every run, so they are test builds. For wider publication use a real signing key stored as a GitHub secret, build from the release tag, and publish SHA256 checksums.
- `build-startos.yml` uses Start9's setup action from `@master`.
