import { IMPOSSIBLE, VersionInfo } from '@start9labs/start-sdk'

export const current = VersionInfo.of({
  version: '29.3.0.20261124:0',
  releaseNotes: {
    en_US: 'First StartOS release of Knots Legacy, based on Knots Legacy 29.3.knots20261124, with the DATUM gateway.',
  },
  migrations: {
    up: async ({ effects }) => {},
    down: IMPOSSIBLE,
  },
})
