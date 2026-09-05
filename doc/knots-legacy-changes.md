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
  release cadence, e.g. `.knots20260507` → `.knots20260906`.
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
