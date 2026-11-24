// Constants shared across the package.

export const dataDir = '/data'
// Directories inside the single 'main' volume. The container entrypoints create them.
export const nodeDir = `${dataDir}/node`
export const sharedDir = `${dataDir}/shared`
export const datumDir = `${dataDir}/datum`

export const rpcPort = 8332
export const p2pPort = 8333
export const stratumPort = 23334
export const dashboardPort = 7152

export const p2pHostId = 'p2p'
export const stratumHostId = 'stratum'
export const dashboardHostId = 'dashboard'

// Valid mainnet payout addresses: bech32/bech32m, or legacy P2PKH/P2SH.
export const addressRegex =
  '^(bc1[ac-hj-np-z02-9]{11,87}|[13][a-km-zA-HJ-NP-Z1-9]{25,34})$'
