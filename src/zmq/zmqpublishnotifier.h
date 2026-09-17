// Copyright (c) 2015-2022 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_ZMQ_ZMQPUBLISHNOTIFIER_H
#define BITCOIN_ZMQ_ZMQPUBLISHNOTIFIER_H

#include <zmq/zmqabstractnotifier.h>

#include <cstddef>
#include <cstdint>
#include <chrono>
#include <functional>
#include <vector>

class CBlockIndex;
class CTransaction;

class CZMQAbstractPublishNotifier : public CZMQAbstractNotifier
{
private:
    uint32_t nSequence {0U}; //!< upcounting per message sequence number

public:

    /* send zmq multipart message
       parts:
          * command
          * data
          * message sequence number
    */
    bool SendZmqMessage(const char *command, const void* data, size_t size);

    bool Initialize(void *pcontext) override;
    void Shutdown() override;
};

class CZMQPublishHashBlockNotifier : public CZMQAbstractPublishNotifier
{
public:
    bool NotifyBlock(const CBlockIndex *pindex) override;
};

class CZMQPublishHashTransactionNotifier : public CZMQAbstractPublishNotifier
{
public:
    bool NotifyTransaction(const CTransaction &transaction) override;
};

class CZMQPublishHashWalletTransactionNotifier : public CZMQAbstractPublishNotifier
{
public:
    bool NotifyWalletTransaction(const CTransaction &transaction, const uint256 &hashBlock) override;
};

class CZMQPublishRawBlockNotifier : public CZMQAbstractPublishNotifier
{
private:
    const std::function<bool(std::vector<uint8_t>&, const CBlockIndex&)> m_get_block_by_index;

public:
    CZMQPublishRawBlockNotifier(std::function<bool(std::vector<uint8_t>&, const CBlockIndex&)> get_block_by_index)
        : m_get_block_by_index{std::move(get_block_by_index)} {}
    bool NotifyBlock(const CBlockIndex *pindex) override;
};

class CZMQPublishRawTransactionNotifier : public CZMQAbstractPublishNotifier
{
public:
    bool NotifyTransaction(const CTransaction &transaction) override;
};

class CZMQPublishRawWalletTransactionNotifier : public CZMQAbstractPublishNotifier
{
public:
    bool NotifyWalletTransaction(const CTransaction &transaction, const uint256 &hashBlock) override;
};

class CZMQPublishSequenceNotifier : public CZMQAbstractPublishNotifier
{
public:
    bool NotifyBlockConnect(const CBlockIndex *pindex) override;
    bool NotifyBlockDisconnect(const CBlockIndex *pindex) override;
    bool NotifyTransactionAcceptance(const CTransaction &transaction, uint64_t mempool_sequence) override;
    bool NotifyTransactionRemoval(const CTransaction &transaction, uint64_t mempool_sequence) override;
};

// Rate-limited "a fresher block template may be worth pulling" wake-up ping.
// Fires on every mempool-accepted transaction, but no-ops if fired again
// before the configured minimum interval has passed. Payload is just an
// 8-byte timestamp -- it is a wake-up signal, not a data feed; subscribers
// still fetch a real template via the normal getblocktemplate RPC.
class CZMQPublishTemplateHintNotifier : public CZMQAbstractPublishNotifier
{
private:
    std::chrono::steady_clock::time_point m_last_sent{};

public:
    // Minimum time between two published hints. Configurable via
    // -zmqpubtemplatehintinterval=<ms>; defaults to 1000ms.
    std::chrono::milliseconds m_min_interval{1000};

    bool NotifyTemplateHint() override;
};

#endif // BITCOIN_ZMQ_ZMQPUBLISHNOTIFIER_H
