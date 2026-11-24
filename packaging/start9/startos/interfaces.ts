import { i18n } from './i18n'
import { sdk } from './sdk'
import {
  dashboardHostId,
  dashboardPort,
  p2pHostId,
  p2pPort,
  stratumHostId,
  stratumPort,
} from './utils'

export const setInterfaces = sdk.setupInterfaces(async ({ effects }) => {
  // Peer-to-peer
  const p2pOrigin = await sdk.MultiHost.of(effects, p2pHostId).bindPort(p2pPort, {
    protocol: null,
    preferredExternalPort: p2pPort,
    addSsl: null,
    secure: { ssl: false },
  })
  const p2p = sdk.createInterface(effects, {
    name: i18n('Peer'),
    id: 'p2p',
    description: i18n('Listens for connections from other Bitcoin nodes'),
    type: 'p2p',
    masked: false,
    schemeOverride: { ssl: null, noSsl: null },
    username: null,
    path: '',
    query: {},
  })

  // Stratum, for the mining rigs
  const stratumOrigin = await sdk.MultiHost.of(effects, stratumHostId).bindPort(
    stratumPort,
    {
      protocol: null,
      preferredExternalPort: stratumPort,
      addSsl: null,
      secure: { ssl: false },
    },
  )
  const stratum = sdk.createInterface(effects, {
    name: i18n('Stratum'),
    id: 'stratum',
    description: i18n('Point your miners at this address (stratum+tcp)'),
    type: 'api',
    masked: false,
    schemeOverride: { ssl: null, noSsl: null },
    username: null,
    path: '',
    query: {},
  })

  // DATUM dashboard
  const dashboardOrigin = await sdk.MultiHost.of(effects, dashboardHostId).bindPort(
    dashboardPort,
    { protocol: 'http' },
  )
  const dashboard = sdk.createInterface(effects, {
    name: i18n('DATUM Dashboard'),
    id: 'dashboard',
    description: i18n('Status, worker stats and settings of the DATUM gateway'),
    type: 'ui',
    masked: false,
    schemeOverride: null,
    username: null,
    path: '',
    query: {},
  })

  return [
    await p2pOrigin.export([p2p]),
    await stratumOrigin.export([stratum]),
    await dashboardOrigin.export([dashboard]),
  ]
})
