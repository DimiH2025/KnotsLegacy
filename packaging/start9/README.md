# Knots Legacy for StartOS

StartOS package for [Knots Legacy](https://github.com/DimiH2025/KnotsLegacy): the node plus the OCEAN DATUM gateway.

It uses two container images built by the Knots Legacy repository (`.github/workflows/docker-images.yml`):
`ghcr.io/dimih2025/knots-legacy-node` and `ghcr.io/dimih2025/knots-legacy-datum`.

## Build

Requirements: Docker, Node.js, `start-cli`.

```
npm install
make x86      # or: make arm
```

Install the resulting `.s9pk` through **Sideload** in the StartOS web interface.

Bump the image tags in `startos/manifest/index.ts` and the version in `startos/versions/current.ts` for every release.
