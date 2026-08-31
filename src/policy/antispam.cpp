// Copyright (c) 2026 The Bitcoin Knots Legacy developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <policy/antispam.h>

#include <common/args.h>
#include <script/interpreter.h>

bool g_antispam_limit_scriptpubkey_size{true};
bool g_antispam_limit_pushdata_size{true};
bool g_antispam_reject_undefined_witness_version{true};
bool g_antispam_reject_taproot_annex{true};
bool g_antispam_limit_control_block_size{true};
bool g_antispam_reject_op_success{true};
bool g_antispam_reject_tapscript_if{true};

void SetupAntiSpamArgs(ArgsManager& argsman)
{
    argsman.AddArg("-antispamscriptpubkeysize", strprintf("Treat non-OP_RETURN outputs with a scriptPubKey larger than %u bytes as non-standard (rule 1) (default: 1)", ANTISPAM_MAX_SCRIPTPUBKEY_SIZE),
                    ArgsManager::ALLOW_ANY, OptionsCategory::NODE_RELAY);
    argsman.AddArg("-antispampushdatasize", strprintf("Treat scriptSig pushes and witness stack items larger than %u bytes as non-standard, P2SH redeemScript exempt (rule 2) (default: 1)", ANTISPAM_MAX_PUSHDATA_SIZE),
                    ArgsManager::ALLOW_ANY, OptionsCategory::NODE_RELAY);
    argsman.AddArg("-antispamwitnessversion", "Treat witness versions with no defined semantics as non-standard (rule 3) (default: 1)",
                    ArgsManager::ALLOW_ANY, OptionsCategory::NODE_RELAY);
    argsman.AddArg("-antispamtaprootannex", "Treat presence of a Taproot annex as non-standard (rule 4) (default: 1)",
                    ArgsManager::ALLOW_ANY, OptionsCategory::NODE_RELAY);
    argsman.AddArg("-antispamcontrolblocksize", strprintf("Treat Taproot control blocks larger than %u bytes as non-standard (rule 5) (default: 1)", ANTISPAM_MAX_CONTROL_BLOCK_SIZE),
                    ArgsManager::ALLOW_ANY, OptionsCategory::NODE_RELAY);
    argsman.AddArg("-antispamopsuccess", "Treat unknown OP_SUCCESSx opcodes as non-standard (rule 6) (default: 1)",
                    ArgsManager::ALLOW_ANY, OptionsCategory::NODE_RELAY);
    argsman.AddArg("-antispamtapscriptif", "Treat OP_IF/OP_NOTIF inside Tapscript as non-standard; blocks the Ordinals/inscriptions-style data envelope pattern, but also affects legitimate Tapscript conditional branches (rule 7) (default: 1)",
                    ArgsManager::ALLOW_ANY, OptionsCategory::NODE_RELAY);
}

void InitAntiSpamFromArgs(const ArgsManager& args)
{
    g_antispam_limit_scriptpubkey_size = args.GetBoolArg("-antispamscriptpubkeysize", g_antispam_limit_scriptpubkey_size);
    g_antispam_limit_pushdata_size = args.GetBoolArg("-antispampushdatasize", g_antispam_limit_pushdata_size);
    g_antispam_reject_undefined_witness_version = args.GetBoolArg("-antispamwitnessversion", g_antispam_reject_undefined_witness_version);
    g_antispam_reject_taproot_annex = args.GetBoolArg("-antispamtaprootannex", g_antispam_reject_taproot_annex);
    g_antispam_limit_control_block_size = args.GetBoolArg("-antispamcontrolblocksize", g_antispam_limit_control_block_size);
    g_antispam_reject_op_success = args.GetBoolArg("-antispamopsuccess", g_antispam_reject_op_success);
    g_antispam_reject_tapscript_if = args.GetBoolArg("-antispamtapscriptif", g_antispam_reject_tapscript_if);
}

std::unordered_set<std::string> GetAntiSpamIgnoredRejects()
{
    std::unordered_set<std::string> ignored;
    if (!g_antispam_reject_undefined_witness_version) {
        ignored.insert("non-mandatory-script-verify-flag-upgradable-witness_program");
        ignored.insert("scriptpubkey-unknown-witnessversion");
    }
    if (!g_antispam_reject_taproot_annex) {
        ignored.insert("taproot-annex");
    }
    if (!g_antispam_limit_pushdata_size) {
        ignored.insert("stackitem-size");
        ignored.insert("taproot-stackitem-size");
    }
    if (!g_antispam_reject_op_success) {
        ignored.insert("non-mandatory-script-verify-flag-upgradable-op_success");
    }
    return ignored;
}

unsigned int GetAntiSpamScriptVerifyFlags()
{
    unsigned int flags{0};
    if (g_antispam_reject_tapscript_if) {
        flags |= SCRIPT_VERIFY_DISCOURAGE_TAPSCRIPT_IF;
    }
    return flags;
}
