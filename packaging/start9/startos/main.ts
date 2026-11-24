import { storeJson } from './fileModels/store.json'
import { i18n } from './i18n'
import { sdk } from './sdk'
import { dashboardPort, dataDir, datumDir, nodeDir, rpcPort, sharedDir } from './utils'

export const main = sdk.setupMain(async ({ effects }) => {
  console.info(i18n('Starting Knots Legacy'))

  // Reading with .const() restarts the service when the user changes these.
  const store = await storeJson.read().const(effects)
  const payoutAddress = store?.payoutAddress ?? null
  const adminPassword = store?.adminPassword ?? null

  const mounts = sdk.Mounts.of().mountVolume({
    volumeId: 'main',
    subpath: null,
    mountpoint: dataDir,
    readonly: false,
  })

  const nodeSub = await sdk.SubContainer.of(
    effects,
    { imageId: 'node' },
    mounts,
    'knots-legacy-node-sub',
  )

  const daemons = sdk.Daemons.of(effects).addDaemon('node', {
    subcontainer: nodeSub,
    exec: {
      command: sdk.useEntrypoint(),
      env: {
        KNOTSLEGACY_DATA_DIR: nodeDir,
        KNOTSLEGACY_SHARED_DIR: sharedDir,
      },
      sigtermTimeout: 300_000,
    },
    ready: {
      display: i18n('Knots Legacy node'),
      fn: () =>
        sdk.healthCheck.checkPortListening(effects, rpcPort, {
          successMessage: i18n('The Knots Legacy RPC interface is ready'),
          errorMessage: i18n('The Knots Legacy RPC interface is not ready'),
        }),
    },
    requires: [],
  })

  // The gateway needs a payout address; until the user has set one, only the node runs.
  if (!payoutAddress) return daemons

  const datumSub = await sdk.SubContainer.of(
    effects,
    { imageId: 'datum' },
    mounts,
    'knots-legacy-datum-sub',
  )

  return daemons.addDaemon('datum', {
    subcontainer: datumSub,
    exec: {
      command: sdk.useEntrypoint(),
      env: {
        DATUM_CONFIG_DIR: datumDir,
        KNOTSLEGACY_SHARED_DIR: sharedDir,
        KNOTSLEGACY_RPC_URL: `http://127.0.0.1:${rpcPort}`,
        DATUM_POOL_ADDRESS: payoutAddress,
        ...(adminPassword ? { DATUM_ADMIN_PASSWORD: adminPassword } : {}),
      },
    },
    ready: {
      display: i18n('DATUM gateway'),
      fn: () =>
        sdk.healthCheck.checkPortListening(effects, dashboardPort, {
          successMessage: i18n('The DATUM dashboard is ready'),
          errorMessage: i18n('The DATUM dashboard is not ready'),
        }),
    },
    requires: ['node'],
  })
})
