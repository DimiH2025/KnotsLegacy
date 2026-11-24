import { storeJson } from '../fileModels/store.json'
import { i18n } from '../i18n'
import { sdk } from '../sdk'
import { addressRegex } from '../utils'

const { InputSpec, Value } = sdk

const inputSpec = InputSpec.of({
  address: Value.text({
    name: i18n('Payout Address'),
    description: i18n('Bitcoin address that receives your mining rewards'),
    required: true,
    default: null,
    patterns: [
      {
        regex: addressRegex,
        description: i18n('Enter a valid mainnet Bitcoin address'),
      },
    ],
  }),
})

export const setPayoutAddress = sdk.Action.withInput(
  // id
  'set-payout-address',

  // metadata
  async ({ effects }) => ({
    name: i18n('Set Payout Address'),
    description: i18n(
      'Enter the Bitcoin address that should receive your mining rewards. The DATUM gateway starts after you do.',
    ),
    warning: null,
    allowedStatuses: 'any',
    group: i18n('Mining'),
    visibility: 'enabled',
  }),

  // input spec
  inputSpec,

  // pre-fill with the current address
  async ({ effects }) => {
    const address = await storeJson.read((s) => s.payoutAddress).once()
    return address ? { address } : undefined
  },

  // execution function
  async ({ effects, input }) => {
    await storeJson.merge(effects, { payoutAddress: input.address.trim() })
    return {
      version: '1',
      title: i18n('Payout address saved'),
      message: i18n(
        'The DATUM gateway will use this address after the service restarts.',
      ),
      result: null,
    }
  },
)
