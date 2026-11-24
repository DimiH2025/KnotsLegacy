export const DEFAULT_LANG = 'en_US'

const dict = {
  // main.ts
  'Starting Knots Legacy': 0,
  'Knots Legacy node': 1,
  'The Knots Legacy RPC interface is ready': 2,
  'The Knots Legacy RPC interface is not ready': 3,
  'DATUM gateway': 4,
  'The DATUM dashboard is ready': 5,
  'The DATUM dashboard is not ready': 6,

  // interfaces.ts
  'Peer': 7,
  'Listens for connections from other Bitcoin nodes': 8,
  'Stratum': 9,
  'Point your miners at this address (stratum+tcp)': 10,
  'DATUM Dashboard': 11,
  'Status, worker stats and settings of the DATUM gateway': 12,

  // actions
  'Set Payout Address': 13,
  'Bitcoin address that receives your mining rewards': 14,
  'Payout Address': 15,
  'Mining': 16,
  'Payout address saved': 17,
  'The DATUM gateway will use this address after the service restarts.': 18,
  'Show Dashboard Password': 19,
  'Shows the password for the DATUM dashboard': 20,
  'DATUM Dashboard Password': 21,
  'Use this password to sign in to the DATUM dashboard.': 22,
  'Enter the Bitcoin address that should receive your mining rewards. The DATUM gateway starts after you do.': 23,
  'Dashboard password': 24,
  'Enter a valid mainnet Bitcoin address': 25,
} as const

/**
 * Plumbing. DO NOT EDIT.
 */
export type I18nKey = keyof typeof dict
export type LangDict = Record<(typeof dict)[I18nKey], string>
export default dict
