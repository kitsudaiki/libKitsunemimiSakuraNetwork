/**
 * @file       multiblock_io.h
 *
 * @author     Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 * @copyright  Apache License Version 2.0
 *
 *      Copyright 2019 Tobias Anker
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *          http://www.apache.org/licenses/LICENSE-2.0
 *
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 */

#ifndef KITSUNEMIMI_SAKURA_NETWORK_MULTIBLOCK_IO_H
#define KITSUNEMIMI_SAKURA_NETWORK_MULTIBLOCK_IO_H

#include <iostream>
#include <assert.h>
#include <atomic>
#include <utility>
#include <deque>
#include <map>
#include <string>

#include <libKitsunemimiCommon/buffer/data_buffer.h>
#include <libKitsunemimiCommon/logger.h>

namespace Kitsunemimi
{
namespace Sakura
{
class Session;

class MultiblockIO
{
public:
    // multiblock-message
    struct MultiblockMessage
    {
        uint64_t blockerId = 0;
        uint64_t multiblockId = 0;
        uint64_t messageSize = 0;
        uint32_t numberOfPackages = 0;
        uint32_t courrentPackage = 0;
        bool abort = false;

        Kitsunemimi::DataBuffer* incomingData = nullptr;
        const void* outgoingData = nullptr;
        uint64_t outgoingDataSize = 0;
    };

    MultiblockIO(Session* session);

    Session* m_session = nullptr;

    // create
    uint64_t createOutgoingBuffer(const void* data,
                                  const uint64_t size,
                                  ErrorContainer &error,
                                  const bool answerExpected = false,
                                  const uint64_t blockerId = 0);
    bool createIncomingBuffer(const uint64_t multiblockId,
                              const uint64_t size);

    // process outgoing
    bool makeOutgoingReady(const uint64_t multiblockId);
    bool sendOutgoingData(const MultiblockMessage &messageBuffer, ErrorContainer &error);

    // process incoming
    MultiblockMessage getIncomingBuffer(const uint64_t multiblockId);
    bool writeIntoIncomingBuffer(const uint64_t multiblockId,
                                 const void* data,
                                 const uint64_t size);

    // remove
    bool removeOutgoingMessage(const uint64_t multiblockId=0);
    bool removeIncomingMessage(const uint64_t multiblockId);

    uint64_t getRandValue();

private:
    bool m_aborCurrentMessage = false;

    std::atomic_flag m_outgoing_lock = ATOMIC_FLAG_INIT;
    std::deque<MultiblockMessage> m_outgoing;

    std::atomic_flag m_incoming_lock = ATOMIC_FLAG_INIT;
    std::map<uint64_t, MultiblockMessage> m_incoming;
};

} // namespace Sakura
} // namespace Kitsunemimi

#endif // KITSUNEMIMI_SAKURA_NETWORK_MULTIBLOCK_IO_H
