# Implementation request: Monetary Nodes (spam-filtered chainstate, no consensus change)

**Status:** request for consideration — not an implementation.

This document asks the maintainer(s) of this fork to consider implementing
the "Monetary Nodes" proposal below in Bitcoin Knots. It intentionally
contains no code and no implementation plan: how (and whether) to build this
is left entirely to the maintainer's judgment.

## What it is

A node-policy proposal, written as a BIP draft, that lets a full node:

- validate every consensus rule identically to a standard full node;
- delete "non-monetary" outputs (inscriptions, Bitcoin Stamps, and
  `OP_RETURN` token-protocol markers such as BRC-20/Runes) from its
  persistent chainstate immediately after each block is validated, so they
  never accumulate in the UTXO database;
- keep two incremental cryptographic commitments per block — a "legacy
  state root" over the full UTXO set and a "monetary state root" over the
  filtered set — bound together with the block hash so neither view can be
  presented apart from, or mismatched with, the other;
- bootstrap new nodes quickly via a snapshot mechanism modeled directly on
  Bitcoin Core's `assumeutxo` (see `doc/design/assumeutxo.md` in this
  repo), with mandatory background full validation after snapshot load.

Classification is fully deterministic and syntactic (three rules, no
external indexer). No output is rendered unspendable, no block or
transaction becomes invalid under current rules, and no fork — soft or
hard — is involved. Filtered outputs remain fully spendable and are
reconstructed from local block storage or from peers when spent.

The stated motivation is UTXO set growth: independent estimates put
40-50% of the ~173M current UTXO entries as data-embedding outputs
unlikely ever to be spent, growing initial block download time, memory
requirements (`dbcache`), and the hardware cost of running a fully
validating node.

The full BIP text explicitly names Bitcoin Knots as the intended reference
implementation target, citing Knots' existing `datacarrier` limit and
inscription-envelope relay policy as the natural base to extend (relay/
mempool/indexing layer only — no consensus code is touched).

## Why this repository

This fork (`DimiH2025/KnotsLegacy`) tracks `bitcoinknots/bitcoin`, and the
proposal is written with Knots specifically in mind. Since this repository
has issues disabled, a pull request is the only available channel to raise
this for the maintainer's consideration.

## References

- Proposal site: https://sambitcoin.github.io/BitcoinMonetaryNode/
- Full BIP draft (source): https://github.com/sambitcoin/BitcoinMonetaryNode/blob/main/Monetary_Nodes.md
- Author: Neal Sampat (`sampat.neal@gmail.com`)
- Proposal author on X: [@marketanarchy21](https://x.com/marketanarchy21)
- Prior work / motivating discussion — "The Cat" (Ostrom, December 2025):
  https://groups.google.com/g/bitcoindev/c/Q6ulQb13okg
- Bitcoin Core `assumeutxo` design (precedent for the snapshot mechanism):
  `doc/design/assumeutxo.md`

## Ask

Please evaluate the linked BIP draft and, at your discretion, decide
whether and how to implement it in this fork. This request takes no
position on implementation approach, phasing, or scope — that is entirely
up to the maintainer.
