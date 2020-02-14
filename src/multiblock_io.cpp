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

#include "multiblock_io.h"

#include <libKitsunemimiProjectNetwork/session.h>
#include <libKitsunemimiPersistence/logger/logger.h>
#include <messages_processing/multiblock_data_processing.h>

namespace Kitsunemimi
{
namespace Project
{

MultiblockIO::MultiblockIO(Session* session)
    : Kitsunemimi::Thread()
{
    m_session = session;
}

/**
 * @brief initialize multiblock-message by data-buffer for a new multiblock and bring statemachine
 *        into required state
 *
 * @param size total size of the payload of the message (no header)
 *
 * @return false, if session is already in send/receive of a multiblock-message
 */
const std::pair<void*, uint64_t>
MultiblockIO::createOutgoingBuffer(const void* data,
                                   const uint64_t size,
                                   const bool answerExpected,
                                   const uint64_t blockerTimeout,
                                   const uint64_t blockerId)
{
    const uint32_t numberOfBlocks = static_cast<uint32_t>(size / 4096) + 1;

    // set or create id
    const uint64_t newMultiblockId = getRandValue();

    // init new multiblock-message
    MultiblockMessage newMultiblockMessage;
    newMultiblockMessage.multiBlockBuffer = new Kitsunemimi::DataBuffer(numberOfBlocks);
    newMultiblockMessage.messageSize = size;
    newMultiblockMessage.multiblockId = newMultiblockId;
    newMultiblockMessage.answerExpected = answerExpected;
    newMultiblockMessage.blockerId = blockerId;

    Kitsunemimi::addDataToBuffer(newMultiblockMessage.multiBlockBuffer,
                                         data,
                                         size);

    // TODO: check if its really possible and if the memory can not be allocated, return 0

    while(m_outgoing_lock.test_and_set(std::memory_order_acquire)) {
        asm("");
    }

    m_outgoing.push_back(newMultiblockMessage);
    m_outgoing_lock.clear(std::memory_order_release);

    send_Data_Multi_Init(m_session, newMultiblockId, size, answerExpected);

    if(answerExpected)
    {
        return SessionHandler::m_blockerHandler->blockMessage(newMultiblockId,
                                                             blockerTimeout,
                                                             m_session);
    }

    std::pair<void*, uint64_t> result;
    result.first = nullptr;
    result.second = newMultiblockId;

    return result;
}

/**
 * @brief MultiblockIO::createIncomingBuffer
 * @param multiblockId
 * @param size
 * @return
 */
bool
MultiblockIO::createIncomingBuffer(const uint64_t multiblockId,
                                   const uint64_t size)
{
    const uint32_t numberOfBlocks = static_cast<uint32_t>(size / 4096) + 1;

    // init new multiblock-message
    MultiblockMessage newMultiblockMessage;
    newMultiblockMessage.multiBlockBuffer = new Kitsunemimi::DataBuffer(numberOfBlocks);
    newMultiblockMessage.messageSize = size;
    newMultiblockMessage.multiblockId = multiblockId;

    // TODO: check if its really possible and if the memory can not be allocated, return false

    while(m_incoming_lock.test_and_set(std::memory_order_acquire)) {
        asm("");
    }

    m_incoming.insert(std::pair<uint64_t, MultiblockMessage>(multiblockId, newMultiblockMessage));
    m_incoming_lock.clear(std::memory_order_release);

    return true;
}

/**
 * @brief MultiblockIO::makeMultiblockReady
 * @param multiblockId
 * @return
 */
bool
MultiblockIO::makeOutgoingReady(const uint64_t multiblockId)
{
    bool found = false;

    while(m_outgoing_lock.test_and_set(std::memory_order_acquire)) {
        asm("");
    }

    std::deque<MultiblockMessage>::iterator it;
    for(it = m_outgoing.begin();
        it != m_outgoing.end();
        it++)
    {
        if(it->multiblockId == multiblockId)
        {
            it->isReady = true;
            found = true;
        }
    }

    m_outgoing_lock.clear(std::memory_order_release);

    if(found) {
        continueThread();
    }

    return found;
}

/**
 * @brief MultiblockIO::sendOutgoingData
 * @param messageBuffer
 * @return
 */
bool
MultiblockIO::sendOutgoingData(const MultiblockMessage& messageBuffer)
{
    // counter values
    uint64_t totalSize = messageBuffer.messageSize;
    uint64_t currentMessageSize = 0;
    uint32_t partCounter = 0;

    // static values
    const uint32_t totalPartNumber = static_cast<uint32_t>(totalSize / 1000) + 1;
    const uint8_t* dataPointer = getBlock(messageBuffer.multiBlockBuffer, 0);

    while(totalSize != 0
          && m_aborCurrentMessage == false)
    {
        // get message-size base on the rest
        currentMessageSize = 1000;
        if(totalSize < 1000) {
            currentMessageSize = totalSize;
        }
        totalSize -= currentMessageSize;

        // send single packet
        send_Data_Multi_Static(m_session,
                               messageBuffer.multiblockId,
                               totalPartNumber,
                               partCounter,
                               dataPointer + (1000 * partCounter),
                               currentMessageSize);

        partCounter++;
    }

    // send final message to other side
    if(m_aborCurrentMessage == false)
    {
        send_Data_Multi_Finish(m_session,
                               messageBuffer.multiblockId,
                               messageBuffer.blockerId);
    }
    else
    {
        send_Data_Multi_Abort_Reply(m_session,
                                    messageBuffer.multiblockId,
                                    m_session->increaseMessageIdCounter());
    }
    m_aborCurrentMessage = false;

    // remove message from outgoing buffer
    while(m_outgoing_lock.test_and_set(std::memory_order_acquire)) {
        asm("");
    }

    delete messageBuffer.multiBlockBuffer;
    m_outgoing.pop_front();
    m_outgoing_lock.clear(std::memory_order_release);

    return true;
}

/**
 * @brief MultiblockIO::getIncomingBuffer
 * @param multiblockId
 * @param eraseFromMap
 * @return
 */
MultiblockIO::MultiblockMessage
MultiblockIO::getIncomingBuffer(const uint64_t multiblockId)
{
    MultiblockMessage tempBuffer;

    while(m_incoming_lock.test_and_set(std::memory_order_acquire)) {
        asm("");
    }

    std::map<uint64_t, MultiblockMessage>::iterator it;
    it = m_incoming.find(multiblockId);

    if(it != m_incoming.end()) {
        tempBuffer = it->second;
    }

    m_incoming_lock.clear(std::memory_order_release);

    return tempBuffer;
}

/**
 * @brief append data to the data-buffer for the multiblock-message
 *
 * @param multiblockId
 * @param data pointer to the data
 * @param size number of bytes
 *
 * @return false, if session is not in the multiblock-transfer-state
 */
bool
MultiblockIO::writeIntoIncomingBuffer(const uint64_t multiblockId,
                                      const void* data,
                                      const uint64_t size)
{
    bool result = false;
    while(m_incoming_lock.test_and_set(std::memory_order_acquire)) {
        asm("");
    }

    std::map<uint64_t, MultiblockMessage>::iterator it;
    it = m_incoming.find(multiblockId);

    if(it != m_incoming.end())
    {
        result = Kitsunemimi::addDataToBuffer(it->second.multiBlockBuffer,
                                              data,
                                              size);
    }

    m_incoming_lock.clear(std::memory_order_release);

    return result;
}

/**
 * @brief remove message form the outgoing-message-buffer
 *
 * @param multiblockId it of the multiblock-message
 *
 * @return true, if multiblock-id was found within the buffer, else false
 */
bool
MultiblockIO::removeOutgoingMessage(const uint64_t multiblockId)
{
    bool result = false;

    while(m_outgoing_lock.test_and_set(std::memory_order_acquire)) {
        asm("");
    }

    std::deque<MultiblockMessage>::iterator it;
    for(it = m_outgoing.begin();
        it != m_outgoing.end();
        it++)
    {
        if(it->multiblockId == multiblockId)
        {
            if(it->currentSend)
            {
                m_aborCurrentMessage = true;
            }
            else
            {
                m_outgoing.erase(it);
                delete it->multiBlockBuffer;
                result = true;
            }
        }
    }

    m_outgoing_lock.clear(std::memory_order_release);

    return result;
}

/**
 * @brief remove message form the incomind-message-buffer, but without deleting the internal
 *        allocated memory.
 *
 * @param multiblockId it of the multiblock-message

 * @return true, if multiblock-id was found within the buffer, else false
 */
bool
MultiblockIO::removeIncomingMessage(const uint64_t multiblockId)
{
    bool result = false;

    while(m_incoming_lock.test_and_set(std::memory_order_acquire)) {
        asm("");
    }

    std::map<uint64_t, MultiblockMessage>::iterator it;
    it = m_incoming.find(multiblockId);

    if(it != m_incoming.end())
    {
        if(it->second.currentSend) {
            m_aborCurrentMessage = true;
        } else {
            m_incoming.erase(it);
        }
        result = true;
    }

    m_incoming_lock.clear(std::memory_order_release);

    return result;
}

/**
 * @brief generate a new random 64bit-value, which is not 0
 *
 * @return new 64bit-value
 */
uint64_t
MultiblockIO::getRandValue()
{
    uint64_t newId = 0;

    // 0 is the undefined value and should never be allowed
    while(newId == 0)
    {
        newId = (static_cast<uint64_t>(rand()) << 32) | static_cast<uint64_t>(rand());
    }

    return newId;
}

/**
 * @brief Main-loop to send data async, if some exist within the outgoing-message-buffer. If no
 *        messages exist within the buffer, the loop is blocked until the next incoming
 *        init-reply-message.
 */
void
MultiblockIO::run()
{
    while(m_abort == false)
    {
        MultiblockMessage tempBuffer;

        while(m_outgoing_lock.test_and_set(std::memory_order_acquire)) {
            asm("");
        }

        if(m_outgoing.empty() == false)
        {
            tempBuffer = m_outgoing.front();

            if(tempBuffer.isReady)
            {
                tempBuffer.currentSend = true;
            }
            else
            {
                m_outgoing.pop_front();
                m_outgoing.push_back(tempBuffer);
            }

            m_outgoing_lock.clear(std::memory_order_release);
        }
        else
        {
            // if buffer is emply, then block the thread
            m_outgoing_lock.clear(std::memory_order_release);
            blockThread();
        }

        // if a valid message was taken, then send the message
        if(tempBuffer.multiBlockBuffer != nullptr) {
            sendOutgoingData(tempBuffer);
        }
    }
}

} // namespace Project
} // namespace Kitsunemimi
