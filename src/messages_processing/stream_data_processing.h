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
send_Data_Stream_Static(Session* session,
                        const void* data,
                        uint64_t size,
                        const bool replyExpected)
{
    Data_StreamStatic_Message message;
    create_Data_StreamStatic_Message(message,
                                     session->sessionId(),
                                     session->increaseMessageIdCounter(),
                                     replyExpected);

    memcpy(message.payload, data, size);
    message.payloadSize = size;

    SessionHandler::m_sessionHandler->sendMessage(session,
                                                  message.commonHeader,
                                                  &message,
                                                  sizeof(message));
}

/**
 * @brief send_Data_Stream_Dynamic
 */
inline void
send_Data_Stream_Dynamic(Session* session,
                         const void* data,
                         const uint64_t dataSize,
                         const bool replyExpected)
{
    // calculate size of the message
    const uint64_t totalMessageSize = sizeof(Data_StreamDynamic_Header)
                                      + dataSize
                                      + sizeof(CommonMessageEnd);
    // bring message-size to a multiple of 8
    const uint64_t totalMessageSizeAligned = totalMessageSize + (totalMessageSize % 8);

    // create message-buffer
    uint8_t* completeMessage = new uint8_t[totalMessageSizeAligned];

    // create header- and end-part of the message
    Data_StreamDynamic_Header header;
    create_Data_StreamDynamic_Header(header,
                                     session->sessionId(),
                                     session->increaseMessageIdCounter(),
                                     replyExpected);
    header.commonHeader.size = static_cast<uint32_t>(totalMessageSizeAligned);
    header.payloadSize = dataSize;
    CommonMessageEnd end;

    // fill buffer to build the complete message
    memcpy(completeMessage, &header, sizeof(Data_StreamDynamic_Header));
    memcpy(completeMessage + sizeof(Data_StreamDynamic_Header), data, dataSize);
    memcpy(completeMessage + (totalMessageSizeAligned - sizeof(CommonMessageEnd)),
           &end,
           sizeof(CommonMessageEnd));

    // send message
    SessionHandler::m_sessionHandler->sendMessage(session,
                                                  header.commonHeader,
                                                  completeMessage,
                                                  totalMessageSizeAligned);

    delete[] completeMessage;
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
inline void
process_Data_Stream_Static(Session* session,
                           const Data_StreamStatic_Message* message)
{
    session->m_processStreamData(session->m_streamDataTarget,
                                 session,
                                 static_cast<const void*>(message->payload),
                                 message->payloadSize);
    send_Data_Stream_Reply(session, message->commonHeader.messageId);
}

/**
 * @brief process_Data_Stream_Dynamic
 */
inline uint64_t
process_Data_Stream_Dynamic(Session* session,
                            const Data_StreamDynamic_Header* message,
                            MessageRingBuffer* recvBuffer)
{
    const uint8_t* completeMessage = getDataPointer(*recvBuffer, message->commonHeader.size);
    if(completeMessage == nullptr) {
        return 0;
    }

    const void* payload = completeMessage + sizeof(Data_StreamDynamic_Header);
    session->m_processStreamData(session->m_streamDataTarget,
                                 session,
                                 payload,
                                 message->payloadSize);
    send_Data_Stream_Reply(session, message->commonHeader.messageId);

    return message->commonHeader.size;
}

/**
 * @brief process_Data_Stream_Reply
 */
inline void
process_Data_Stream_Reply(Session*,
                          const Data_StreamReply_Message*)
{
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
                         MessageRingBuffer* recvBuffer)
{
    switch(header->subType)
    {
        //------------------------------------------------------------------------------------------
        case DATA_STREAM_STATIC_SUBTYPE:
            {
                const Data_StreamStatic_Message* message =
                        getObjectFromBuffer<Data_StreamStatic_Message>(recvBuffer);
                if(message == nullptr) {
                    break;
                }
                process_Data_Stream_Static(session, message);
                return sizeof(*message);
            }
        //------------------------------------------------------------------------------------------
        case DATA_STREAM_DYNAMIC_SUBTYPE:
            {
                const Data_StreamDynamic_Header* messageHeader =
                        getObjectFromBuffer<Data_StreamDynamic_Header>(recvBuffer);
                if(messageHeader == nullptr) {
                    break;
                }
                const uint64_t messageSize = process_Data_Stream_Dynamic(session,
                                                                         messageHeader,
                                                                         recvBuffer);
                return messageSize;
            }
        //------------------------------------------------------------------------------------------
        case DATA_STREAM_REPLY_SUBTYPE:
            {
                const Data_StreamReply_Message* message =
                        getObjectFromBuffer<Data_StreamReply_Message>(recvBuffer);
                if(message == nullptr) {
                    break;
                }
                process_Data_Stream_Reply(session, message);
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

#endif // STREAM_DATA_PROCESSING_H
