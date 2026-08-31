// Copyright (c) 2026 The Bitcoin Knots Legacy developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_POLICY_ANTISPAM_H
#define BITCOIN_POLICY_ANTISPAM_H

#include <cstddef>
#include <string>
#include <unordered_set>

class ArgsManager;

/**
 * Local, non-consensus relay/mining policy switches modeled on BIP-110's
 * ("Reduced Data Temporary Softfork") seven anti-spam rule *contents* — but
 * enforced purely as this node's own standardness/relay policy, never as a
 * consensus soft-fork. Disabling any of these only affects what THIS node
 * relays and mines; it never changes which blocks this node considers valid,
 * and it never causes this node to reject a block another node accepts.
 *
 * Rules 3, 4, and 6 have exact unconditional equivalents already present in
 * this codebase's standardness checks; for those, this module simply makes
 * the existing check optional (reusing it, not duplicating it). Rules 1, 2,
 * 5, and 7 are genuinely new checks added by this module, since no existing
 * check in this codebase enforces those specific thresholds.
 *
 * All 7 default to true (anti-spam filtering ON).
 */

//! Rule 1 (NEW CHECK): non-OP_RETURN outputs with a scriptPubKey above this size are non-standard.
extern bool g_antispam_limit_scriptpubkey_size;
static constexpr unsigned int ANTISPAM_MAX_SCRIPTPUBKEY_SIZE{34};

//! Rule 2 (reuses existing checks): witness stack items above a size threshold are
//! non-standard. NOTE: this codebase's existing MAX_STANDARD_P2WSH_STACK_ITEM_SIZE and
//! MAX_STANDARD_TAPSCRIPT_STACK_ITEM_SIZE are already 80 bytes by default — stricter
//! than BIP-110 rule 2's 256-byte figure — so this toggle governs whether those
//! existing (already stricter) checks apply at all, rather than enforcing 256 bytes
//! specifically. ANTISPAM_MAX_PUSHDATA_SIZE is kept here for reference/GUI display only.
extern bool g_antispam_limit_pushdata_size;
static constexpr unsigned int ANTISPAM_MAX_PUSHDATA_SIZE{256};

//! Rule 3 (reuses existing check): witness versions with no defined semantics are non-standard.
extern bool g_antispam_reject_undefined_witness_version;

//! Rule 4 (reuses existing check): presence of a Taproot annex is non-standard.
extern bool g_antispam_reject_taproot_annex;

//! Rule 5 (NEW CHECK): Taproot control blocks above this size are non-standard.
extern bool g_antispam_limit_control_block_size;
static constexpr unsigned int ANTISPAM_MAX_CONTROL_BLOCK_SIZE{257};

//! Rule 6 (reuses existing check): unknown OP_SUCCESSx opcodes are non-standard.
extern bool g_antispam_reject_op_success;

//! Rule 7 (NEW CHECK, new interpreter flag SCRIPT_VERIFY_DISCOURAGE_TAPSCRIPT_IF):
//! OP_IF/OP_NOTIF inside Tapscript is non-standard. Blocks the OP_FALSE OP_IF
//! ... OP_ENDIF "envelope" pattern used to embed arbitrary data (e.g. Ordinals/
//! inscriptions). NOTE: this also affects legitimate contracts that use
//! conditional branches inside Tapscript (e.g. some HTLC constructions) — that
//! collateral effect is inherent to this rule, not a bug.
extern bool g_antispam_reject_tapscript_if;

/** Register the -antispam* startup arguments with the ArgsManager. Call from SetupServerArgs(). */
void SetupAntiSpamArgs(ArgsManager& argsman);

/** Read the -antispam* startup arguments and set the globals above. Call from AppInitParameterInteraction(). */
void InitAntiSpamFromArgs(const ArgsManager& args);

/**
 * Returns the set of existing policy "reject reason" keys (see MaybeReject()
 * in policy.cpp / PolicyScriptVerifyFlags() in validation.cpp) that should be
 * treated as ignored given the current rule 3/4/6 globals. Merge this into
 * the ignore_rejects set used for mempool acceptance and block-template
 * construction so disabling a rule actually takes effect.
 */
std::unordered_set<std::string> GetAntiSpamIgnoredRejects();

/** Extra script verify flags (currently just rule 7) to OR into the standard flags. */
unsigned int GetAntiSpamScriptVerifyFlags();

#endif // BITCOIN_POLICY_ANTISPAM_H
