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

MultiblockIO::MultiblockIO(Session* session,
                           const std::string &threadName)
    : Kitsunemimi::Thread(threadName)
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
MultiblockIO::createOutgoingBuffer(DataBuffer* result,
                                   const void* data,
                                   const uint64_t size,
                                   const bool answerExpected,
                                   const uint64_t blockerId)
{
    // calculate required number of blocks to allocate within the buffer
    const uint32_t numberOfBlocks = static_cast<uint32_t>(size / 4096) + 1;

    // set or create id
    const uint64_t newMultiblockId = getRandValue();
    Kitsunemimi::reset_DataBuffer(*result, numberOfBlocks);

    // init new multiblock-message
    MultiblockMessage newMultiblockMessage;
    newMultiblockMessage.multiBlockBuffer = result;
    newMultiblockMessage.messageSize = size;
    newMultiblockMessage.multiblockId = newMultiblockId;
    newMultiblockMessage.blockerId = blockerId;

    // check if memory allocation was successful
    if(newMultiblockMessage.multiBlockBuffer == nullptr) {
        return 0;
    }

    // write data, which should be send, to the temporary buffer
    Kitsunemimi::addData_DataBuffer(*newMultiblockMessage.multiBlockBuffer, data, size);

    // put buffer into message-queue to be send in the background
    while(m_outgoing_lock.test_and_set(std::memory_order_acquire)) { asm(""); }
    m_outgoing.push_back(newMultiblockMessage);
    m_outgoing_lock.clear(std::memory_order_release);

    // send init-message to initialize the transfer for the data
    send_Data_Multi_Init(m_session, newMultiblockId, size, answerExpected);

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
    newMultiblockMessage.multiBlockBuffer = new Kitsunemimi::DataBuffer(calcBytesToBlocks(size));
    newMultiblockMessage.messageSize = size;
    newMultiblockMessage.multiblockId = multiblockId;

    // check if memory allocation was successful
    if(newMultiblockMessage.multiBlockBuffer == nullptr) {
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
 * @brief send multi-block message
 *
 * @param messageBuffer message to send
 *
 * @return false, if sending message failed, else true
 */
bool
MultiblockIO::sendOutgoingData(const MultiblockMessage& messageBuffer)
{
    // counter values
    uint64_t totalSize = messageBuffer.messageSize;
    uint64_t currentMessageSize = 0;
    uint32_t partCounter = 0;

    // static values
    const uint32_t totalPartNumber = static_cast<uint32_t>(totalSize / MAX_SINGLE_MESSAGE_SIZE) + 1;
    const uint8_t* dataPointer = getBlock_DataBuffer(*messageBuffer.multiBlockBuffer, 0);

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
        send_Data_Multi_Static(m_session,
                               messageBuffer.multiblockId,
                               totalPartNumber,
                               partCounter,
                               dataPointer + (MAX_SINGLE_MESSAGE_SIZE * partCounter),
                               static_cast<uint32_t>(currentMessageSize));

        partCounter++;
    }

    // send final message to other side
    if(m_aborCurrentMessage == false)
    {
        // TODO: check return value
        send_Data_Multi_Finish(m_session,
                               messageBuffer.multiblockId,
                               messageBuffer.blockerId);
    }
    else
    {
        // TODO: check return value
        send_Data_Multi_Abort_Reply(m_session,
                                    messageBuffer.multiblockId,
                                    m_session->increaseMessageIdCounter());
    }
    m_aborCurrentMessage = false;

    // remove message from outgoing buffer
    while(m_outgoing_lock.test_and_set(std::memory_order_acquire)) { asm(""); }
    delete messageBuffer.multiBlockBuffer;
    m_outgoing.pop_front();
    m_outgoing_lock.clear(std::memory_order_release);

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

    while(m_outgoing_lock.test_and_set(std::memory_order_acquire)) { asm(""); }

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
    while(m_outgoing_lock.test_and_set(std::memory_order_acquire)) { asm(""); }

    std::map<uint64_t, MultiblockMessage>::iterator it;
    it = m_incoming.find(multiblockId);

    if(it != m_incoming.end())
    {
        result = Kitsunemimi::addData_DataBuffer(*it->second.multiBlockBuffer,
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
    while(m_outgoing_lock.test_and_set(std::memory_order_acquire)) { asm(""); }

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
    while(m_outgoing_lock.test_and_set(std::memory_order_acquire)) { asm(""); }

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
    while(newId == 0) {
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
        while(m_outgoing_lock.test_and_set(std::memory_order_acquire)) { asm(""); }

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

} // namespace Sakura
} // namespace Kitsunemimi
