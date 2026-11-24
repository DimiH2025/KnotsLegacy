import { sdk } from './sdk'

// Knots Legacy bundles its own node, so it depends on no other service.
export const dependencies = sdk.Dependencies.of()
