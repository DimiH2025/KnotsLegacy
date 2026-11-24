import { storeJson } from '../fileModels/store.json'
import { i18n } from '../i18n'
import { sdk } from '../sdk'

export const showDashboardPassword = sdk.Action.withoutInput(
  // id
  'show-dashboard-password',

  // metadata
  async ({ effects }) => ({
    name: i18n('Show Dashboard Password'),
    description: i18n('Shows the password for the DATUM dashboard'),
    warning: null,
    allowedStatuses: 'any',
    group: i18n('Mining'),
    visibility: 'enabled',
  }),

  // execution function
  async ({ effects }) => {
    const password = await storeJson.read((s) => s.adminPassword).once()
    return {
      version: '1',
      title: i18n('DATUM Dashboard Password'),
      message: i18n('Use this password to sign in to the DATUM dashboard.'),
      result: {
        type: 'single',
        value: password ?? '',
        copyable: true,
        masked: true,
        qr: false,
        launchable: false,
      },
    }
  },
)
