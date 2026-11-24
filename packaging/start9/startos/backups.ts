import { sdk } from './sdk'

// Back up settings and the wallet; the blockchain can always be downloaded again.
export const { createBackup, restoreInit } = sdk.setupBackups(async () =>
  sdk.Backups.ofVolumes('main').setOptions({
    exclude: [
      'node/blocks/',
      'node/chainstate/',
      'node/indexes/',
      'node/.lock',
      'node/**/*-journal',
    ],
  }),
)
