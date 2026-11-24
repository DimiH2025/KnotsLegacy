import { FileHelper, z } from '@start9labs/start-sdk'
import { sdk } from '../sdk'

export const shape = z.looseObject({
  // Where the DATUM gateway sends mining rewards. Null until the user sets it.
  payoutAddress: z.string().nullable().catch(null),
  // Password for the DATUM dashboard, generated once on install.
  adminPassword: z.string().nullable().catch(null),
})

export const storeJson = FileHelper.json(
  {
    base: sdk.volumes.main,
    subpath: '/store.json',
  },
  shape,
)
