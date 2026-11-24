# Knots Legacy — Changes from Upstream Bitcoin Knots

This document chronicles every change made to produce Knots Legacy from
upstream Bitcoin Knots. It's intended for the repository's `docs/` folder as
a permanent record of what was changed, why, and where — for contributors,
auditors, or anyone deciding whether to trust and run this software.

## What Knots Legacy is

Knots Legacy is a fork of [Bitcoin Knots](https://bitcoinknots.org/), based on
the last release before Knots adopted **BIP-110 / RDTS** ("Reduced Data
Temporary Softfork"): **v29.3.knots20260507**. It exists so that node
operators who do not want to adopt the RDTS consensus change have a
continuously maintained client to run instead, rather than being stuck on an
unmaintained snapshot.

**Network compatibility:** Knots Legacy makes no consensus changes of its
own. Its `src/kernel/chainparams.cpp` (network magic bytes, genesis block,
seed nodes, ports) is byte-for-byte identical to the upstream pre-RDTS tag —
verified by direct diff against `v29.3.knots20260507`. It connects to, and
validates blocks on, the same Bitcoin network as any other non-RDTS Knots or
Core node.

---

## 1. Base and versioning

- Forked from `bitcoinknots/bitcoin` at tag **v29.3.knots20260507** — the
  last release without any RDTS code, rather than reverting or patching out
  RDTS from a later commit. This was a deliberate choice: RDTS touches
  consensus-critical files (`validation.cpp`, `consensus/params.h`,
  `chainparams.cpp`, and others), and hand-editing consensus code to remove a
  soft-fork carries real risk of a subtly incomplete removal. Building from
  the actual last commit that never had the code at all avoids that risk
  entirely.
- Version string (`CMakeLists.txt`, `CLIENT_VERSION_SUFFIX`) tracks upstream
  Knots' own dating convention and is bumped independently of upstream's own
  release cadence, e.g. `.knots20260507` → `.knots20260906` →
  `.knots20261124` (the release described in Sections 9 and 10).
- `COPYRIGHT_YEAR` is `2026` (correct at time of writing).

## 2. Branding

- **`CLIENT_NAME`** (`CMakeLists.txt`) changed from `"Bitcoin Knots"` to
  `"Knots Legacy"`. This single variable drives the installer window title,
  Windows `CompanyName` version resource, and the macOS `.zip` output name
  (`Knots-Legacy.zip`) — all derived from it automatically.
- **Windows installer output renamed**: `bitcoin-win64-setup.exe` →
  `knots-legacy-win64-setup.exe`, via `cmake/module/GenerateSetupNsi.cmake`
  (the generated `.nsi` script's own filename, which NSIS uses to name its
  output when no explicit `OutFile` is set) and `cmake/module/Maintenance.cmake`
  (the `deploy` custom target's dependency path).
- **Compatibility was verified, not assumed**, before this rename: the
  Windows install directory (`$PROGRAMFILES64\Bitcoin`, hardcoded in
  `share/setup.nsi.in`) and the data directory (`GetDefaultDataDir()` in
  `src/common/args.cpp`, hardcoded to `Bitcoin`/`.bitcoin`) are **not** tied
  to `CLIENT_NAME`, so existing wallets, chainstate, and installed binaries
  from a prior non-RDTS Knots install are found and upgraded in place with no
  manual steps. The one cosmetic side effect: the Windows uninstall registry
  key *is* keyed on `CLIENT_NAME`, so Add/Remove Programs will show a new
  "Knots Legacy" entry alongside an orphaned (harmless) old "Bitcoin Knots"
  entry, rather than replacing it.
- **New "LEGACY" wordmark** added to the shared logo mark
  (`src/qt/res/src/bitcoinknots-logo.svg`). This required two iterations:
  - First attempt added the text as a sibling element outside the
    `id="logo"` group — this rendered correctly on the splash screen (a
    direct render of the whole file) but silently failed to appear on the
    actual app icon, macOS `.icns`, and the NSIS installer header, because
    all three pull in the mark via `<use xlink:href="bitcoinknots-logo.svg#logo">`,
    which only clones the referenced element, not its siblings.
  - Fixed by moving the text **inside** the `id="logo"` group, so every
    `<use>` consumer inherits it automatically. Verified against the real
    build tool (`rsvg-convert`, not a substitute) at every actual output
    size: splash screen, 256px app icon, macOS icon, and the NSIS header.
  - Known limitation, accepted deliberately: at 16–32px (taskbar/favicon
    size) the wordmark is not legible — physically too small. This is an
    inherent tradeoff of baking text into a single-source icon, not a bug.

## 3. BIP-110-content anti-spam policy (local, non-consensus)

Seven independently toggleable **local relay/mining policy** switches,
modeled on BIP-110's rule *content* but enforced purely as this node's own
standardness rules — never as a consensus soft-fork. Disabling any of them
only changes what this node itself relays and mines; it never changes which
blocks this node considers valid, and cannot cause a chain split.

New module: `src/policy/antispam.h` / `src/policy/antispam.cpp`.

| # | Rule | CLI flag | Default | Implementation note |
|---|------|----------|---------|----------------------|
| 1 | Oversized, unrecognized-shape scriptPubKeys | `-antispamscriptpubkeysize` | on | New check, scoped to `TxoutType::NONSTANDARD` only (see bug fix below) |
| 2 | Oversized script pushes / witness items | `-antispampushdatasize` | on | Toggles this codebase's existing 80-byte item-size checks, which are already stricter than BIP-110's own 256-byte figure |
| 3 | Undefined witness versions | `-antispamwitnessversion` | on | Toggles an existing unconditional check |
| 4 | Taproot annex present | `-antispamtaprootannex` | on | Toggles an existing unconditional check |
| 5 | Oversized Taproot control blocks | `-antispamcontrolblocksize` | on | New check (257-byte cap) |
| 6 | Unknown `OP_SUCCESSx` opcodes | `-antispamopsuccess` | on | Toggles an existing unconditional check |
| 7 | `OP_IF`/`OP_NOTIF` inside Tapscript | `-antispamtapscriptif` | on | New interpreter flag, `SCRIPT_VERIFY_DISCOURAGE_TAPSCRIPT_IF`. Blocks the `OP_FALSE OP_IF ... OP_ENDIF` "envelope" pattern used for embedding arbitrary data (e.g. inscriptions). Also affects legitimate Tapscript contracts using conditionals (e.g. some HTLC constructions) — an accepted tradeoff of the rule itself. |

Files touched: `src/policy/antispam.h` (new), `src/policy/antispam.cpp`
(new), `src/policy/policy.h`, `src/policy/policy.cpp`, `src/validation.cpp`,
`src/script/interpreter.h`, `src/script/interpreter.cpp`,
`src/script/script_error.h`, `src/script/script_error.cpp`, `src/init.cpp`,
`src/CMakeLists.txt`.

Confirmed safe: `SCRIPT_VERIFY_DISCOURAGE_TAPSCRIPT_IF` is never included in
`MANDATORY_SCRIPT_VERIFY_FLAGS` — it only ever reaches the mempool/relay
policy path (`PolicyScriptVerifyFlags()`), never block validation.

### Bug found and fixed (rule 1)

The first version of rule 1 rejected *any* non-OP_RETURN output over 34
bytes, with no exception for recognized standard types that are legitimately
larger — most notably bare `PUBKEY` outputs (35 bytes for a compressed key)
and bare `MULTISIG` outputs. This broke `script_p2sh_tests` in CI (two test
cases: `sign` and `set`). Fixed by scoping the rule to
`TxoutType::NONSTANDARD` only — the catch-all case for scripts that don't
match any recognized shape, which is the only case this rule can usefully add
anything on top of the codebase's existing type-specific standardness logic.

## 4. Options dialog — Anti-Spam controls

The 7 switches above were added as checkboxes to the existing, Knots-specific
**"Spam &filtering"** tab in the Options dialog (`groupBox_Spamfiltering` in
`src/qt/optionsdialog.cpp` — this tab predates Knots Legacy and already held
similar toggles like `rejecttokens` and `subdustfeepenalty`).

- Wired as **live settings** — flipping a checkbox takes effect immediately,
  no restart, matching the tab's existing sibling checkboxes. Persisted via
  `node().updateRwSetting()` to `settings.json`, which `ArgsManager` reads
  directly, so the same value is picked up automatically on next startup too.
- Files touched: `src/qt/optionsmodel.h`, `src/qt/optionsmodel.cpp`,
  `src/qt/optionsdialog.h`, `src/qt/optionsdialog.cpp`.

## 5. Removal of the upstream "adopt RDTS or your node is vulnerable" warnings

Upstream's pre-RDTS release (the exact tag this project is built from)
shipped **four separate, deliberate mechanisms** urging users to abandon this
version of the software in favor of an RDTS-adopting one. Since Knots
Legacy's entire purpose is to keep running the pre-RDTS ruleset, all four
were removed:

1. **A startup modal dialog** (`UserProtocolRulesConsent()` in
   `src/init.cpp`) — could outright abort startup if a
   `consensusrules=rdts` config flag was present (`ICON_WARNING | BTN_ABORT |
   MODAL`), or show a dismissible "please update" warning otherwise. Now a
   no-op returning `true` unconditionally.
2. **A config-flag validation gate** (`UserProtocolRulesCheck()`, same
   file) — companion check for the `-consensusrules` argument. Also now a
   no-op returning `true`.
3. **An hourly recurring log message** (`scheduler.scheduleEvery(...,
   std::chrono::hours{1})` in `src/init.cpp`) — wrote the same "please
   update" text to `debug.log` every hour, indefinitely, independent of the
   dialog above. Removed entirely.
4. **A permanent warning banner**, set unconditionally in
   `ChainstateManager`'s constructor (`src/validation.cpp`) via
   `kernel::Warning::RULES_NOT_ENFORCED` — surfaced in the GUI's warning
   indicator on every run regardless of network conditions. Removed
   entirely.

As a follow-on cleanup, `resetSettings()` (`src/node/interfaces.cpp`, the
GUI's "Reset Options" action) had a special case preserving the
`consensusrules` settings key across a reset, so a routine reset wouldn't
silently undo a user's RDTS opt-in. With the whole consent mechanism gone,
this exception no longer served a purpose and was simplified back to a plain
settings clear.

## 6. Continuous integration / release builds

Three new GitHub Actions workflows (`.github/workflows/`), building
downloadable release artifacts for all three desktop platforms from a single
`push`/`pull_request`/`workflow_dispatch` trigger on the `legacy` branch:

- **`build-linux.yml`** — native build on `ubuntu-24.04`, stripped binaries,
  packaged as a `.tar.gz`.
- **`build-windows.yml`** — cross-compiled via the `depends/` system
  (`HOST=x86_64-w64-mingw32`), producing an NSIS installer via the `deploy`
  CMake target.
- **`build-macos.yml`** — native (not cross-compiled) matrix build across
  Apple Silicon (`macos-latest`) and Intel (`macos-15-intel`), Homebrew
  dependencies, producing an unsigned, non-notarized `.zip` via the same
  `deploy` target.

All three explicitly pass `-DBUILD_GUI=ON` — this codebase defaults it
`OFF`, and it is not auto-enabled outside the Windows `depends/` toolchain,
so omitting it silently produces a build with no `bitcoin-qt` at all.

### Node.js 20 deprecation fixes

GitHub's deprecation of Node 20 action runtimes surfaced in two rounds:

- Pre-existing composite actions used by `ci.yml` (`.github/actions/`):
  `cirruslabs/cache/restore@v4` → `@v5`, `cirruslabs/cache/save@v4` → `@v5`,
  `docker/setup-buildx-action@v3` → `@v4`.
- The three new build workflows above: `actions/cache@v4` → `@v5`,
  `actions/cache/restore@v4` → `@v5`, and — easy to miss — `actions/upload-artifact@v4`
  → **`@v6`**, not `@v5`; v5's own release notes describe only "preliminary"
  Node 24 support while still defaulting to Node 20, with v6 being the
  release that actually switches the default.

Every version bump above was checked against the action's actual published
release notes or tag list before being applied, rather than assumed.

## 7. Security fix: `ReplaceAll()` / `-walletnotify` command injection

Upstream Bitcoin Core's 32.0 release notes led to a fix for a flaw in a
shared utility function, `util::ReplaceAll()` in `src/util/string.cpp`
(upstream fix: `bitcoin/bitcoin#36048`). Knots Legacy carried the same
vulnerable code, since it comes from the same code lineage.

**The flaw.** `ReplaceAll()` was implemented with `std::regex_replace()`,
whose third argument is a *replacement format string*, not literal text.
Sequences such as `$'`, `$&`, `` $` `` and `$1` inside it have special
meaning (for example, `$'` copies in the text that follows the match).
`src/wallet/wallet.cpp` uses `ReplaceAll(command, "%w", walletname_escaped)`
to build the `-walletnotify` command, and a wallet name is chosen freely by
whoever calls the `createwallet` RPC. A wallet named to contain `$'` could
therefore inject text from the surrounding command template into itself and
alter the command handed to the shell.

**Why `ShellEscape()` did not prevent it.** The wallet name is shell-escaped
before substitution, but that only protects against shell metacharacters at
the point the string reaches a shell. The `$'` expansion happens earlier,
inside the C++ string substitution, which `ShellEscape()` was never designed
to cover.

**Who could exploit it.** A caller with RPC access sufficient to call
`createwallet`, on a node with wallet support and `-walletnotify`
configured. This is not an unauthenticated remote attack, but it is a
privilege escalation from "can create a wallet" to "can influence a command
run with the node operator's privileges". POSIX builds only: on Windows the
code substitutes a fixed placeholder for the wallet name (the `#ifdef WIN32`
branch in `wallet.cpp`), so that path was never exposed there.

**The fix** (`src/util/string.cpp`). `ReplaceAll()` is now a literal,
non-recursive substring replacement. `substitute` is never interpreted as
anything but plain text, and scanning resumes after the inserted text, so a
substitute that itself contains the search string is not replaced a second
time. The `<regex>` include was removed along with its only use in the file.

**Verification performed:**

- **Call-site audit.** Every other `ReplaceAll()` caller uses safe
  substitute text: `-alertnotify` (passed through `SanitizeString`, whose
  allowed characters exclude `$` and `'`), `-blocknotify` (a hex block hash),
  the `%s`/`%b`/`%h` substitutions in `-walletnotify` (hex hashes, a number,
  or the fixed strings `unconfirmed` and `-1`), Tor control password escaping,
  and `ShellEscape()` itself. No caller passes a regex metacharacter in the
  search argument, so dropping regex matching changes nothing for them.
- **Bug reproduced.** With a `$'` substitute, the old implementation turned
  `A test "%s" string '%s'.` into `A test "" string '%s'." string ''.'.`, i.e.
  template text injected into itself. The fixed version keeps `$'` literal.
- **No regressions.** Old and new implementations produce identical output on
  every case already in the `util_ReplaceAll` unit test.
- **Real code exercised.** The actual `src/util/string.cpp` was compiled
  standalone and checked against 11 expected results, including the new
  regression cases.
- **End-to-end command string.** A simulated `-walletnotify` command built
  with a hostile wallet name showed a corrupted command (quoting broken,
  template tail duplicated) before the fix and a correctly quoted one after.
  This confirms the command text is altered; no working command-execution
  payload was constructed or tested.

**Regression tests** added to the existing `util_ReplaceAll` case in
`src/test/util_tests.cpp`: substitutes `$'`, `$&`, `` $` ``, `$1` and `$$`
must pass through verbatim, and a substitute containing the search string must
not be replaced again.

Files touched: `src/util/string.cpp`, `src/test/util_tests.cpp`.

## 8. DATUM Gateway support (opt-in, additive)

[DATUM Gateway](https://github.com/OCEAN-xyz/datum_gateway) is a separate,
standalone program that lets solo and small-pool miners build their own block
templates instead of trusting a pool operator to decide block contents. It
talks to any full node the normal way (`getblocktemplate` RPC, block
notifications, its own RPC credentials) and needs no changes to the node's
consensus or validation code. So "integrating DATUM" here means two small
additions plus a GUI toggle, not building DATUM's functionality into the node.

This work was ported from an earlier integration in another Knots-derived
client, then re-checked against this codebase's own patterns rather than copied
blindly.

**Risk profile.** Strictly additive and opt-in. Nothing touches consensus,
validation, the P2P protocol, or default behavior for anyone who leaves these
options off. The change is 111 inserted lines and no deletions across 10 files.

### 8.1 `-datum` configuration preset (`src/init.cpp`)

A single startup flag that bundles DATUM's own documented recommended
settings, following the same shape as the existing `-corepolicy` flag. It uses
`SoftSetArg`, so it only applies a value the operator has not already set
explicitly. Default: off.

| Setting | Value | Reason |
|---|---|---|
| `-txindex` | `1` | Required by DATUM |
| `-server` | `1` | RPC must be enabled |
| `-blockmaxweight` / `-blockmaxsize` | `3985000` | Deliberately below the ~4,000,000 maximum, leaving headroom for the pool's own coinbase/generation transaction |
| `-maxmempool` | `1000` (MB) | A larger mempool gives more fee-paying candidates to choose from |
| `-blockreconstructionextratxn` | `1000000` | Per DATUM's recommendations |

**Deliberately not set:** `-blocknotify` (the right value is a shell command
that depends entirely on where DATUM Gateway is installed, so a guessed default
would be worse than leaving it manual) and RPC credentials (a secret should
never be silently generated by a preset flag). Both remain manual steps; see
DATUM Gateway's own setup documentation.

**Interaction with `-corepolicy`.** `SoftSetArg` treats a value set by an
earlier `SoftSetArg` call as already set. The `-corepolicy` block runs first,
so if both flags are enabled together, `-corepolicy`'s `-blockmaxweight` and
`-blockmaxsize` (`4000000`) win over this preset's `3985000`. This is not a
bug, but anyone combining the two should set those values explicitly.

### 8.2 `-zmqpubtemplatehint` notification

A new, rate-limited, opt-in ZMQ notification, published when the mempool has
seen new activity and a fresher block template may be worth fetching. Without
it, mining software learns about new blocks from existing notifications but has
to poll for new mempool activity on a fixed timer.

- **Topic and payload.** Topic `templatehint`. The payload is an 8-byte
  little-endian millisecond timestamp. It is a wake-up ping, not a data feed:
  subscribers still fetch a real template through `getblocktemplate`.
- **Rate limiting.** At most one hint per interval, default 1000 ms. Hints
  that arrive sooner are dropped silently. A value of 0 or below for the
  interval effectively disables the limit (the value is not validated).
- **Trigger.** Dispatched from `CZMQNotificationInterface::TransactionAddedToMempool`
  only, i.e. genuinely new mempool-accepted transactions. It is deliberately not
  sent from block-connect or block-disconnect handling: those already have their
  own notifications, and transactions re-added from a disconnected block are not
  new activity in the sense this hint is meant to signal.
- **Fit with existing code.** It slots into the existing ZMQ notifier
  framework as one more factory entry (`pubtemplatehint`), with a new virtual
  `NotifyTemplateHint()` on the base notifier (default no-op) and a
  `CZMQPublishTemplateHintNotifier` implementation. The existing notifier
  construction loop needed no changes beyond applying the interval at creation.
- **New options.** `-zmqpubtemplatehint=<address>`,
  `-zmqpubtemplatehinthwm=<n>` (outbound high-water mark, same convention as the
  other ZMQ options) and `-zmqpubtemplatehintinterval=<ms>`. They are registered
  alongside the other ZMQ options, hidden in builds without ZMQ, and
  `-zmqpubtemplatehint` was added to the address-validation table in
  `init.cpp` so a malformed address fails cleanly at startup instead of
  confusingly at runtime.

**Honest caveat.** Stock DATUM Gateway does not consume this topic today; it
discovers mempool activity by polling. The hint is a forward-looking hook that
is safe to leave enabled (it does nothing if nobody is subscribed), not
something DATUM Gateway will take advantage of without further work on its own
side or a small companion script. The mechanism itself is generic and not
specific to DATUM.

### 8.3 "Mine with DATUM" checkbox (Options dialog)

A single checkbox on the existing Mempool tab, where block-template and mempool
sizing settings already live, rather than a new tab for one control. It is a
thin face on the `-datum` flag: toggling it persists `datum` to the settings
file and flags that a restart is required, since these are startup-time
settings. It follows the same pattern as the dialog's existing `Server` and
`Listen` checkboxes.

Files touched: `src/init.cpp`, `src/zmq/zmqabstractnotifier.h`,
`src/zmq/zmqabstractnotifier.cpp`, `src/zmq/zmqpublishnotifier.h`,
`src/zmq/zmqpublishnotifier.cpp`, `src/zmq/zmqnotificationinterface.cpp`,
`src/qt/optionsdialog.h`, `src/qt/optionsdialog.cpp`, `src/qt/optionsmodel.h`,
`src/qt/optionsmodel.cpp`.

Written against this codebase's existing patterns and checked for brace and
parenthesis balance, but not compiled in the environment it was authored in;
CI is the first real build.

---

## 9. Security audit against the Bitcoin Core advisories (2026-11-24)

On 10 November 2026 Bitcoin Core disclosed three advisories that had been
fixed in October 2025 (CVE-2025-62859, CVE-2025-62860, CVE-2025-62861), after
the 29.x line went end of life. A separate handoff from the Bit-Block project
listed five security fixes ported by hand onto an older Knots 29.1 base.
Knots Legacy is based on a **newer** release (29.3.knots20260507, which
includes Core's 29.2 and 29.3 backports), so each item was checked in this
code base first, rather than ported blindly.

**Result: all five fixes are already present in the 29.3-based code. No
consensus or relay code had to be changed for them.**

| CVE | Core severity | What it was | Where it is already fixed here |
|---|---|---|---|
| CVE-2024-52911 | n/a | Use-after-free in `ConnectBlock` (`txsdata` freed before the script-check threads finished) | `src/validation.cpp`, `ConnectBlock`: `txsdata` is declared before the `CCheckQueueControl` |
| CVE-2025-46598 | low | CPU DoS from crafted unconfirmed transactions (quadratic sighash work, duplicate script re-verification) | `SigHashCache` in `src/script/interpreter.cpp`; `CheckInputScripts` classifies failures from a single verification; `SpendsNonAnchorWitnessProg` in `src/policy/policy.cpp`; no `MaybePunishNodeForTx` |
| CVE-2025-62859 | medium | Compact-block witness malleation could stall block relay for up to ~10 minutes | `PartiallyDownloadedBlock::FillBlock` (`src/blockencodings.cpp`) runs the mutation check; `ProcessCompactBlockTxns` (`src/net_processing.cpp`) handles a repeated `blocktxn` with a null header |
| CVE-2025-62860 | medium | `-whitelist` address ranges applied to inbound Tor connections | `CConnman::CreateNodeFromAcceptedSocket` (`src/net.cpp`) passes no source address for inbound onion connections |
| CVE-2025-62861 | HIGH | TRUC sibling eviction ran while re-adding transactions after a reorg, leaving the mempool inconsistent so the node could not build block templates until restarted | `MaybeUpdateMempoolForReorg` calls `AcceptToMemoryPool(..., bypass_limits=true)`, which maps to an ignore-rejects set that includes `"truc"`, so the TRUC checks (and sibling eviction) are skipped for reorg re-adds (`src/validation.h`, `src/validation.cpp`) |

How this was checked: by reading the code paths above in the `legacy` branch,
and the published advisories (bitcoincore.org). The reorg fix (CVE-2025-62861)
in particular uses Knots' own mechanism rather than Core's, so it was
verified by tracing the call path, not by a dedicated reorg test.

**Non-CVE items from the same handoff**

- Wallet directory deletion safety after a failed migration, restore or dump
  (Core #34222 / #34370): present. `RestoreWallet` removes only the file and
  directory it created, and failed-migration cleanup removes only files and
  empty directories it created.
- `sendall` with unsolvable descriptors (Core 29.3 backports, #34268): present
  (`Unable to determine the size of the transaction, the wallet contains
  unsolvable descriptors`).
- `getblockheader` reports the target of the requested block: present
  (`blockheaderToJSON` uses the requested block index).
- `IsFromMe` checks per input: present (`CWallet::IsFromMe`).
- Netlink build fix (Core #34680): FreeBSD-only code, not built on any
  platform Knots Legacy ships for; the FreeBSD workaround is already present.
- `-walletnotify` command injection: fixed earlier in Section 7.
- **Deliberately not ported: refusing wallet names containing `.` or `..`**
  (Core #34544, a master-branch change that was still under review). In this
  code base, relative wallet names are supported and covered by upstream's own
  migration tests (`wallet_migration.py`), and the cleanup code is already
  limited to files the operation created, so the names are not a safety
  problem here. A blanket refusal would stop existing users from loading or
  migrating wallets stored under such names.
- Bit-Block-specific items (`mnemonicfromdice`, BIP-39 handling, the 256-byte
  anti-spam push limit) do not exist in Knots Legacy and were not applicable.

**Not ported on purpose:** Knots' BIP-110 / RDTS enforcement, and any rebase
onto Knots 29.3.knots20260508 or later (those enforce RDTS as consensus
rules). Fixes must keep being ported by hand, one at a time.

**Watch list:** Core stated that further medium and high severity issues
fixed in 30.0 will be disclosed later. Check
<https://bitcoincore.org/en/security-advisories/> regularly and port any
fix that applies to the 29.x line.

### 9.1 One behaviour change in this release

`-datum` no longer forces `-txindex` on a pruned node. Pruning and
`-txindex` cannot be combined (startup would stop with "Prune mode is
incompatible with -txindex"), and `-datum` is also a saved GUI setting, so
switching it on while pruned would have made every following start fail.
Now the node logs `parameter interaction: -datum set but -prune is enabled ->
not enabling -txindex` and starts normally. DATUM Gateway still needs an
unpruned node with the transaction index. File: `src/init.cpp`
(`InitParameterInteraction`).

### 9.2 Version

`CLIENT_VERSION_SUFFIX` is now `.knots20261124`, so the version string is
`29.3.0.knots20261124`. File: `CMakeLists.txt`.

---

## 10. Umbrel and StartOS packaging

Added `packaging/`, a container and appliance packaging adapted from the
Bit-Block project, so a home miner can run the node together with the OCEAN
DATUM gateway on Umbrel or StartOS. See `packaging/README.md` for the release
steps and install instructions.

- **Container images** (`packaging/docker/`): `knots-legacy-node` builds a
  headless `bitcoind` and `bitcoin-cli` from this repository and starts it
  with `-datum=1 -server=1`; `knots-legacy-datum` builds the pinned upstream
  DATUM Gateway v0.5.1beta. RPC credentials are generated once and handed to
  the gateway through a shared folder, never on a command line. Only the
  stratum port (23334) and peer-to-peer port (8333) are published.
- **Umbrel** (`packaging/umbrel/`): a community app store with one app,
  `knotslegacy-node` ("Knots Legacy + DATUM"). The store is meant to be copied
  into its own repository (`knotslegacy-umbrel-store`) that users add under
  Community App Stores.
- **StartOS 0.4** (`packaging/start9/`): a TypeScript SDK 4.0.0 package with
  actions "Set Payout Address" and "Show Dashboard Password" and the
  interfaces peer, stratum and dashboard. The StartOS version is
  `29.3.0.20261124:0`: StartOS versions must be dotted numbers, so the
  `knots` suffix cannot be used there.
- **Workflows**: `.github/workflows/docker-images.yml` (native amd64 and arm64
  builds, multi-arch manifest on `ghcr.io/dimih2025/`) and
  `.github/workflows/build-startos.yml` (x86 and arm `.s9pk` files as
  artifacts). Both use Node 24 action versions (checkout v5,
  setup-buildx v4, login v4, build-push v7, upload-artifact v6,
  download-artifact v7).
- **`.dockerignore`**: keeps `.git`, build folders and `node_modules` out of
  the Docker build context.

Checked in the authoring environment: the headless CMake configuration used by
the node Dockerfile configures cleanly with the exact flags, and the StartOS
package passes a TypeScript type-check against SDK 4.0.0 with a valid
`ExtendedVersion`. **Not checked:** the Docker images were not built, and
neither package has been run on a real Umbrel or StartOS device.

---

## Not yet done / known gaps

- **Not compiled or tested by an outside build** beyond the CI runs
  referenced above — always build and run the test suite yourself before
  relying on a given commit.
- **macOS builds are unsigned and unnotarized.** Gatekeeper will require a
  right-click → Open override, or `xattr -dr com.apple.quarantine` on the
  `.app`, until code signing with a paid Apple Developer ID is set up.
- **Windows uninstall registry entries are not deduplicated** across the
  `CLIENT_NAME` rename (see Section 2) — cosmetic only, does not affect
  functionality.
- **Rule 1's practical effect is narrow** in the default configuration,
  since `TxoutType::NONSTANDARD` outputs are already unconditionally
  rejected by this codebase's base standardness check independent of this
  toggle (see Section 3) — documented rather than overstated.
- **DATUM Gateway does not yet use `-zmqpubtemplatehint`** (see Section 8.2),
  and `-datum` does not configure `-blocknotify` or RPC credentials. Both are
  intentional, but a miner still has to complete those steps by hand.
- **Packaging is untested on real devices** (Section 10). Images are pinned
  by tag rather than digest, and `.s9pk` builds use a throwaway signing key.
- **Template-hint rate limiting has no trailing edge** (Section 8.2): a hint
  suppressed inside the minimum interval is not re-sent later, so the last
  transaction of a burst may not trigger a hint until the next one arrives.
