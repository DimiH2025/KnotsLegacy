import { sdk } from '../sdk'
import { setPayoutAddress } from './setPayoutAddress'
import { showDashboardPassword } from './showDashboardPassword'

export const actions = sdk.Actions.of()
  .addAction(setPayoutAddress)
  .addAction(showDashboardPassword)
