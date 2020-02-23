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

#ifndef STREAM_DATA_PROCESSING_H
#define STREAM_DATA_PROCESSING_H

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
 * @brief send_Data_Stream_Static
 */
inline void
send_Data_Stream(Session* session,
                 const void* data,
                 const uint32_t size,
                 const bool replyExpected)
{
    uint8_t messageBuffer[STATIC_PAYLOAD_SIZE];

    // bring message-size to a multiple of 8
    const uint32_t totalMessageSize = sizeof(Data_Stream_Header)
                                      + size
                                      + (size % 8)
                                      + sizeof(CommonMessageEnd);

    CommonMessageEnd end;
    Data_Stream_Header message;
    create_Data_StreamStatic_Message(message,
                                     session->sessionId(),
                                     session->increaseMessageIdCounter(),
                                     totalMessageSize,
                                     size,
                                     replyExpected);

    // fill buffer to build the complete message
    memcpy(&messageBuffer[0], &message, sizeof(Data_Stream_Header));
    memcpy(&messageBuffer[sizeof(Data_Stream_Header)], data, size);
    memcpy(&messageBuffer[(totalMessageSize - sizeof(CommonMessageEnd))],
           &end,
           sizeof(CommonMessageEnd));

    SessionHandler::m_sessionHandler->sendMessage(session,
                                                  message.commonHeader,
                                                  messageBuffer,
                                                  totalMessageSize);
}

/**
 * @brief send_Data_Stream_Reply
 */
inline void
send_Data_Stream_Reply(Session* session,
                       const uint32_t messageId)
{
    Data_StreamReply_Message message;
    create_Data_StreamReply_Message(message, session->sessionId(), messageId);
    SessionHandler::m_sessionHandler->sendMessage(session,
                                                  message.commonHeader,
                                                  &message,
                                                  sizeof(message));
}

/**
 * @brief process_Data_Stream_Static
 */
inline uint64_t
process_Data_Stream(Session* session,
                    const void* rawMessage)
{
    const Data_Stream_Header* header = static_cast<const Data_Stream_Header*>(rawMessage);
    const uint8_t* payloadData = static_cast<const uint8_t*>(rawMessage)
                                 + sizeof(Data_Stream_Header);

    session->m_processStreamData(session->m_streamDataTarget,
                                 session,
                                 static_cast<const void*>(payloadData),
                                 header->commonHeader.payloadSize);

    if(header->commonHeader.flags & 0x1) {
        send_Data_Stream_Reply(session, header->commonHeader.messageId);
    }

    return header->commonHeader.totalMessageSize;
}

/**
 * @brief process_Data_Stream_Reply
 */
inline uint64_t
process_Data_Stream_Reply(Session*,
                          const void*)
{
    return sizeof(Data_StreamReply_Message);
}

/**
 * @brief process messages of stream-message-type
 *
 * @param session pointer to the session
 * @param header pointer to the common header of the message within the message-ring-buffer
 * @param recvBuffer pointer to the message-ring-buffer
 *
 * @return number of processed bytes
 */
inline uint64_t
process_Stream_Data_Type(Session* session,
                         const CommonMessageHeader* header,
                         const void* rawMessage,
                         const uint32_t)
{
    switch(header->subType)
    {
        //------------------------------------------------------------------------------------------
        case DATA_STREAM_STATIC_SUBTYPE:
            {
                return process_Data_Stream(session, rawMessage);
            }
        //------------------------------------------------------------------------------------------
        case DATA_STREAM_REPLY_SUBTYPE:
            {
                return process_Data_Stream_Reply(session, rawMessage);
            }
        //------------------------------------------------------------------------------------------
        default:
            break;
    }

    return 0;
}

} // namespace Project
} // namespace Kitsunemimi

#endif // STREAM_DATA_PROCESSING_H
