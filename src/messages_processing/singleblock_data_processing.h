/**
 * @file       singleblock_data_processing.h
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

#ifndef SINGLE_DATA_PROCESSING_H
#define SINGLE_DATA_PROCESSING_H

#include <message_definitions.h>
#include <handler/session_handler.h>
#include <multiblock_io.h>

#include <libKitsunemimiNetwork/abstract_socket.h>
#include <libKitsunemimiNetwork/message_ring_buffer.h>

#include <libKitsunemimiProjectNetwork/session_controller.h>
#include <libKitsunemimiProjectNetwork/session.h>

#include <libKitsunemimiPersistence/logger/logger.h>

using Kitsunemimi::Network::MessageRingBuffer;
using Kitsunemimi::Network::AbstractSocket;
using Kitsunemimi::Network::getObjectFromBuffer;
using Kitsunemimi::Network::getDataPointer;

namespace Kitsunemimi
{
namespace Project
{

/**
 * @brief send_Data_SingleBlock
 */
inline void
send_Data_SingleBlock(Session* session,
                      const uint64_t multiblockId,
                      const void* data,
                      uint64_t size,
                      const uint64_t blockerId=0)
{
    Data_SingleBlock_Message message;
    create_Data_SingleBlock_Message(message,
                                    session->sessionId(),
                                    session->increaseMessageIdCounter(),
                                    multiblockId,
                                    blockerId);

    memcpy(message.payload, data, size);
    message.payloadSize = size;

    SessionHandler::m_sessionHandler->sendMessage(session,
                                                  message.commonHeader,
                                                  &message,
                                                  sizeof(message));
}

/**
 * @brief send_Data_SingleBlock_Reply
 */
inline void
send_Data_SingleBlock_Reply(Session* session,
                            const uint32_t messageId)
{
    Data_SingleBlockReply_Message message;
    create_Data_SingleBlockReply_Message(message, session->sessionId(), messageId);
    SessionHandler::m_sessionHandler->sendMessage(session,
                                                  message.commonHeader,
                                                  &message,
                                                  sizeof(message));
}

/**
 * @brief process_Data_SingleBlock
 */
inline void
process_Data_SingleBlock(Session* session,
                         const Data_SingleBlock_Message* message)
{
    DataBuffer* buffer = new DataBuffer(1);
    addDataToBuffer(buffer, message->payload, message->payloadSize);

    session->m_processStandaloneData(session->m_standaloneDataTarget,
                                     session,
                                     message->multiblockId,
                                     buffer);
    send_Data_SingleBlock_Reply(session, message->commonHeader.messageId);
}

/**
 * @brief process_Data_SingleBlock_Reply
 */
inline void
process_Data_SingleBlock_Reply(Session*,
                               const Data_SingleBlockReply_Message*)
{
}

/**
 * @brief process messages of singleblock-message-type
 *
 * @param session pointer to the session
 * @param header pointer to the common header of the message within the message-ring-buffer
 * @param recvBuffer pointer to the message-ring-buffer
 *
 * @return number of processed bytes
 */
inline uint64_t
process_SingleBlock_Data_Type(Session* session,
                              const CommonMessageHeader* header,
                              MessageRingBuffer* recvBuffer)
{
    switch(header->subType)
    {
        //------------------------------------------------------------------------------------------
        case DATA_SINGLE_DATA_SUBTYPE:
            {
                const Data_SingleBlock_Message* message =
                        getObjectFromBuffer<Data_SingleBlock_Message>(recvBuffer);
                if(message == nullptr
                        || message->commonEnd.end != MESSAGE_DELIMITER)
                {
                    break;
                }
                process_Data_SingleBlock(session, message);
                return sizeof(*message);
            }
        //------------------------------------------------------------------------------------------
        case DATA_SINGLE_REPLY_SUBTYPE:
            {
                const Data_SingleBlockReply_Message* message =
                        getObjectFromBuffer<Data_SingleBlockReply_Message>(recvBuffer);
                if(message == nullptr
                        || message->commonEnd.end != MESSAGE_DELIMITER)
                {
                    break;
                }
                process_Data_SingleBlock_Reply(session, message);
                return sizeof(*message);
            }
        //------------------------------------------------------------------------------------------
        default:
            break;
    }

    return 0;
}

} // namespace Project
} // namespace Kitsunemimi

#endif // SINGLE_DATA_PROCESSING_H
