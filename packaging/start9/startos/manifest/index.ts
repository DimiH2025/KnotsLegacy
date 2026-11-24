import { setupManifest } from '@start9labs/start-sdk'
import { long, short } from './i18n'

export const manifest = setupManifest({
  id: 'knots-legacy',
  title: 'Knots Legacy',
  license: 'MIT',
  packageRepo: 'https://github.com/DimiH2025/KnotsLegacy',
  upstreamRepo: 'https://github.com/DimiH2025/KnotsLegacy',
  marketingUrl: 'https://github.com/DimiH2025/KnotsLegacy',
  donationUrl: null,
  description: { short, long },
  volumes: ['main'],
  images: {
    node: {
      source: { dockerTag: 'ghcr.io/dimih2025/knots-legacy-node:29.3.knots20261124' },
      arch: ['x86_64', 'aarch64'],
    },
    datum: {
      source: { dockerTag: 'ghcr.io/dimih2025/knots-legacy-datum:29.3.knots20261124' },
      arch: ['x86_64', 'aarch64'],
    },
  },
})
