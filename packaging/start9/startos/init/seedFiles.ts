import { randomBytes } from 'crypto'
import { setPayoutAddress } from '../actions/setPayoutAddress'
import { storeJson } from '../fileModels/store.json'
import { i18n } from '../i18n'
import { sdk } from '../sdk'

export const seedFiles = sdk.setupOnInit(async (effects, kind) => {
  if (!kind) return

  // install, update, restore: make sure store.json exists with its defaults.
  await storeJson.merge(effects, {})

  const store = await storeJson.read().once()

  if (!store?.adminPassword) {
    await storeJson.merge(effects, {
      adminPassword: randomBytes(12).toString('hex'),
    })
  }

  // Ask the user for the payout address; the gateway cannot start without it.
  if (!store?.payoutAddress) {
    await sdk.action.createOwnTask(effects, setPayoutAddress, 'critical', {
      reason: i18n(
        'Enter the Bitcoin address that should receive your mining rewards. The DATUM gateway starts after you do.',
      ),
    })
  }
})
