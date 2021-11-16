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

#include "multiblock_io.h"

#include <libKitsunemimiSakuraNetwork/session.h>
#include <libKitsunemimiCommon/logger.h>
#include <messages_processing/multiblock_data_processing.h>

namespace Kitsunemimi
{
namespace Sakura
{

MultiblockIO::MultiblockIO(Session* session)
{
    m_session = session;
}

/**
 * @brief initialize multiblock-message by data-buffer for a new multiblock and bring statemachine
 *        into required state
 *
 * @param data payload of the message to send
 * @param size total size of the payload of the message (no header)
 * @param answerExpected true, if message is a request-message
 * @param blockerId blocker-id in case that the message is a response
 *
 * @return
 */
uint64_t
MultiblockIO::createOutgoingBuffer(const void* data,
                                   const uint64_t size,
                                   ErrorContainer &error,
                                   const bool answerExpected,
                                   const uint64_t blockerId)
{
    // set or create id
    const uint64_t newMultiblockId = getRandValue();

    // init new multiblock-message
    MultiblockMessage newMultiblockMessage;
    newMultiblockMessage.messageSize = size;
    newMultiblockMessage.multiblockId = newMultiblockId;
    newMultiblockMessage.blockerId = blockerId;
    newMultiblockMessage.outgoingData = data;
    newMultiblockMessage.outgoingDataSize = size;

    // put buffer into message-queue to be send in the background
    while(m_outgoing_lock.test_and_set(std::memory_order_acquire)) { asm(""); }
    m_outgoing.push_back(newMultiblockMessage);
    m_outgoing_lock.clear(std::memory_order_release);

    // send init-message to initialize the transfer for the data
    send_Data_Multi_Init(m_session, newMultiblockId, size, answerExpected, error);

    return newMultiblockId;
}

/**
 * @brief create new buffer for the message
 *
 * @param multiblockId id of the multiblock-message
 * @param size size for the new buffer
 *
 * @return false, if allocation failed, else true
 */
bool
MultiblockIO::createIncomingBuffer(const uint64_t multiblockId,
                                   const uint64_t size)
{
    // init new multiblock-message
    MultiblockMessage newMultiblockMessage;
    newMultiblockMessage.incomingData = new Kitsunemimi::DataBuffer(calcBytesToBlocks(size));
    newMultiblockMessage.messageSize = size;
    newMultiblockMessage.multiblockId = multiblockId;

    // check if memory allocation was successful
    if(newMultiblockMessage.incomingData == nullptr) {
        return false;
    }

    // put buffer into message-queue to be filled with incoming data
    while(m_incoming_lock.test_and_set(std::memory_order_acquire)) { asm(""); }
    m_incoming.insert(std::make_pair(multiblockId, newMultiblockMessage));
    m_incoming_lock.clear(std::memory_order_release);

    return true;
}

/**
 * @brief toggle flag in multi-block buffer to register, that the handshake was complete
 *
 * @param multiblockId id of the multiblock-message
 *
 * @return flase, if id is unknown, else true
 */
bool
MultiblockIO::makeOutgoingReady(const uint64_t multiblockId)
{
    bool found = false;

    while(m_outgoing_lock.test_and_set(std::memory_order_acquire)) { asm(""); }

    std::deque<MultiblockMessage>::iterator it;
    for(it = m_outgoing.begin();
        it != m_outgoing.end();
        it++)
    {
        if(it->multiblockId == multiblockId)
        {
            sendOutgoingData(*it, m_session->sessionError);
            found = true;
        }
    }

    m_outgoing_lock.clear(std::memory_order_release);

    return found;
}

/**
 * @brief send multi-block message
 *
 * @param messageBuffer message to send
 *
 * @return false, if sending message failed, else true
 */
bool
MultiblockIO::sendOutgoingData(const MultiblockMessage& messageBuffer,
                               ErrorContainer &error)
{
    // counter values
    uint64_t totalSize = messageBuffer.messageSize;
    uint64_t currentMessageSize = 0;
    uint32_t partCounter = 0;

    // static values
    const uint32_t totalPartNumber = static_cast<uint32_t>(totalSize / MAX_SINGLE_MESSAGE_SIZE) + 1;
    const uint8_t* dataPointer = static_cast<const uint8_t*>(messageBuffer.outgoingData);

    while(totalSize != 0
          && m_aborCurrentMessage == false)
    {
        // get message-size base on the rest
        currentMessageSize = MAX_SINGLE_MESSAGE_SIZE;
        if(totalSize <= MAX_SINGLE_MESSAGE_SIZE) {
            currentMessageSize = totalSize;
        }
        totalSize -= currentMessageSize;

        // send single packet
        // TODO: check return value
        if(send_Data_Multi_Static(m_session,
                                  messageBuffer.multiblockId,
                                  totalPartNumber,
                                  partCounter,
                                  dataPointer + (MAX_SINGLE_MESSAGE_SIZE * partCounter),
                                  static_cast<uint32_t>(currentMessageSize),
                                  error) == false)
        {
            return false;
        }

        partCounter++;
    }

    // send final message to other side
    if(m_aborCurrentMessage == false)
    {
        // TODO: check return value
        send_Data_Multi_Finish(m_session,
                               messageBuffer.multiblockId,
                               messageBuffer.blockerId,
                               error);
    }
    else
    {
        // TODO: check return value
        send_Data_Multi_Abort_Reply(m_session,
                                    messageBuffer.multiblockId,
                                    m_session->increaseMessageIdCounter(),
                                    error);
    }
    m_aborCurrentMessage = false;

    return true;
}

/**
 * @brief get incoming buffer by its id
 *
 * @param multiblockId id of the multiblock-message
 *
 * @return buffer, if found, else an empty-buffer-object
 */
MultiblockIO::MultiblockMessage
MultiblockIO::getIncomingBuffer(const uint64_t multiblockId)
{
    MultiblockMessage tempBuffer;

    while(m_incoming_lock.test_and_set(std::memory_order_acquire)) { asm(""); }

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
 * @param multiblockId id of the multiblock-message
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
    while(m_incoming_lock.test_and_set(std::memory_order_acquire)) { asm(""); }

    std::map<uint64_t, MultiblockMessage>::iterator it;
    it = m_incoming.find(multiblockId);

    if(it != m_incoming.end()) {
        result = Kitsunemimi::addData_DataBuffer(*it->second.incomingData, data, size);
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
    while(m_outgoing_lock.test_and_set(std::memory_order_acquire)) { asm(""); }

    std::deque<MultiblockMessage>::iterator it;
    for(it = m_outgoing.begin();
        it != m_outgoing.end();
        it++)
    {
        if(it->multiblockId == multiblockId) {
            it->abort = true;
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
    while(m_incoming_lock.test_and_set(std::memory_order_acquire)) { asm(""); }

    std::map<uint64_t, MultiblockMessage>::iterator it;
    it = m_incoming.find(multiblockId);
    if(it != m_incoming.end())
    {
        m_incoming.erase(it);
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
    while(newId == 0) {
        newId = (static_cast<uint64_t>(rand()) << 32) | static_cast<uint64_t>(rand());
    }

    return newId;
}

} // namespace Sakura
} // namespace Kitsunemimi
