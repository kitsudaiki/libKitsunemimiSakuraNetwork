/**
 * @file       multiblock_io.cpp
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

#ifndef MULTIBLOCK_IO_H
#define MULTIBLOCK_IO_H

#include <iostream>
#include <assert.h>
#include <atomic>
#include <utility>
#include <deque>
#include <map>
#include <string>

#include <libKitsunemimiCommon/data_buffer.h>
#include <libKitsunemimiCommon/threading/thread.h>

namespace Kitsunemimi
{
namespace Project
{
class Session;

class MultiblockIO
        : public Kitsunemimi::Common::Thread
{
public:
    // multiblock-message
    struct MultiblockMessage
    {
        bool isReady = false;
        uint64_t multiblockId = 0;
        uint64_t messageSize = 0;
        uint32_t numberOfPackages = 0;
        uint32_t courrentPackage = 0;
        Kitsunemimi::Common::DataBuffer* multiBlockBuffer = nullptr;
    };

    MultiblockIO(Session* session);

    Session* m_session = nullptr;

    // handle multiblock-buffer
    uint64_t createBacklogBuffer(const void* data,
                                 const uint64_t size);
    bool createIncomingBuffer(const uint64_t multiblockId,
                              const uint64_t size);

    bool writeDataIntoBuffer(const uint64_t multiblockId,
                             const void* data,
                             const uint64_t size);
    bool makeMultiblockReady(const uint64_t multiblockId);
    MultiblockMessage getIncomingBuffer(const uint64_t multiblockId,
                                        const bool eraseFromMap = false);

    bool abortMultiblockDataTransfer(const uint64_t multiblockId=0);
    bool finishMultiblockDataTransfer();

    uint64_t getRandValue();

protected:
    void run();

private:
    bool sendOutgoingData(const MultiblockMessage &messageBuffer);

    std::atomic_flag m_backlog_lock = ATOMIC_FLAG_INIT;
    std::deque<MultiblockMessage> m_backlog;

    std::atomic_flag m_incoming_lock = ATOMIC_FLAG_INIT;
    std::map<uint64_t, MultiblockMessage> m_incoming;
};

} // namespace Project
} // namespace Kitsunemimi

#endif // MULTIBLOCK_IO_H
