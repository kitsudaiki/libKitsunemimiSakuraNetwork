/**
 *  @file       singleblock_data_processing.h
 *
 *  @author     Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 *  @copyright  Apache License Version 2.0
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

#include <network_session/message_definitions.h>
#include <network_session/session_handler.h>
#include <network_session/internal_session_interface.h>

#include <libKitsuneNetwork/abstract_socket.h>
#include <libKitsuneNetwork/message_ring_buffer.h>

#include <libKitsuneProjectCommon/network_session/session_controller.h>
#include <libKitsuneProjectCommon/network_session/session.h>

#include <libKitsunePersistence/logger/logger.h>

using Kitsune::Network::MessageRingBuffer;
using Kitsune::Network::AbstractSocket;
using Kitsune::Network::getObjectFromBuffer;
using Kitsune::Network::getDataPointer;

namespace Kitsune
{
namespace Project
{
namespace Common
{

/**
 * @brief send_Data_Single_Static
 */
inline void
send_Data_Single_Static(Session* session,
                        const void* data,
                        uint64_t size,
                        const bool replyExpected)
{
    if(DEBUG_MODE) {
        LOG_DEBUG("SEND data single static");
    }

    Data_SingleStatic_Message message(session->sessionId(),
                                      session->increaseMessageIdCounter(),
                                      replyExpected);

    memcpy(message.payload, data, size);
    message.payloadSize = size;

    SessionHandler::m_sessionInterface->sendMessage(session,
                                                    message.commonHeader,
                                                    &message,
                                                    sizeof(message));
}

/**
 * @brief send_Data_Single_Dynamic
 */
inline void
send_Data_Single_Dynamic(Session* session,
                         const void* data,
                         const uint64_t dataSize,
                         const bool replyExpected)
{
    if(DEBUG_MODE) {
        LOG_DEBUG("SEND data single dynamic");
    }

    uint64_t totalMessageSize = sizeof(Data_SingleDynamic_Header)
                                + dataSize
                                + sizeof(CommonMessageEnd);
    totalMessageSize += totalMessageSize % 8;

    uint8_t completeMessage[totalMessageSize];
    Data_SingleDynamic_Header header(session->sessionId(),
                                     session->increaseMessageIdCounter(),
                                     replyExpected);
    header.commonHeader.size = static_cast<uint32_t>(totalMessageSize);
    header.payloadSize = dataSize;
    CommonMessageEnd end;

    memcpy(&completeMessage[0], &header, sizeof(Data_SingleDynamic_Header));
    memcpy(&completeMessage[sizeof(Data_SingleDynamic_Header)], data, dataSize);
    memcpy(&completeMessage[totalMessageSize - sizeof(CommonMessageEnd)],
           &end,
           sizeof(CommonMessageEnd));

    SessionHandler::m_sessionInterface->sendMessage(session,
                                                    header.commonHeader,
                                                    &completeMessage,
                                                    sizeof(completeMessage));
}

/**
 * @brief send_Data_Single_Reply
 */
inline void
send_Data_Single_Reply(Session* session,
                       const uint32_t messageId)
{
    if(DEBUG_MODE) {
        LOG_DEBUG("SEND data single reply");
    }

    Data_SingleReply_Message message(session->sessionId(), messageId);
    SessionHandler::m_sessionInterface->sendMessage(session,
                                                    message.commonHeader,
                                                    &message,
                                                    sizeof(message));
}

/**
 * @brief process_Data_Single_Static
 */
inline void
process_Data_Single_Static(Session* session,
                           const Data_SingleStatic_Message* message)
{
    if(DEBUG_MODE) {
        LOG_DEBUG("process data single static");
    }

    SessionHandler::m_sessionInterface->receivedData(session,
                                                     true,
                                                     static_cast<const void*>(message->payload),
                                                     message->payloadSize);
    send_Data_Single_Reply(session, message->commonHeader.messageId);
}

/**
 * @brief process_Data_Single_Dynamic
 */
inline uint64_t
process_Data_Single_Dynamic(Session* session,
                            const Data_SingleDynamic_Header* message,
                            MessageRingBuffer* recvBuffer)
{
    if(DEBUG_MODE) {
        LOG_DEBUG("process data single dynamic");
    }

    const uint8_t* completeMessage = getDataPointer(*recvBuffer, message->commonHeader.size);
    if(completeMessage == nullptr) {
        return 0;
    }

    const void* payload = completeMessage + sizeof(Data_SingleDynamic_Header);
    SessionHandler::m_sessionInterface->receivedData(session,
                                                     true,
                                                     payload,
                                                     message->payloadSize);
    send_Data_Single_Reply(session, message->commonHeader.messageId);

    return message->commonHeader.size;
}

/**
 * @brief process_Data_Single_Reply
 */
inline void
process_Data_Single_Reply(Session*,
                          const Data_SingleReply_Message*)
{
    if(DEBUG_MODE) {
        LOG_DEBUG("process data single reply");
    }
}

/**
 * @brief process_Data_Type
 * @param session
 * @param header
 * @param recvBuffer
 * @return
 */
inline uint64_t
process_SingleBlock_Data_Type(Session* session,
                              const CommonMessageHeader* header,
                              MessageRingBuffer* recvBuffer)
{
    if(DEBUG_MODE) {
        LOG_DEBUG("process data-type");
    }

    switch(header->subType)
    {
        case DATA_SINGLE_STATIC_SUBTYPE:
            {
                const Data_SingleStatic_Message* message =
                        getObjectFromBuffer<Data_SingleStatic_Message>(recvBuffer);
                if(message == nullptr) {
                    break;
                }
                process_Data_Single_Static(session, message);
                return sizeof(*message);
            }
        case DATA_SINGLE_DYNAMIC_SUBTYPE:
            {
                const Data_SingleDynamic_Header* messageHeader =
                        getObjectFromBuffer<Data_SingleDynamic_Header>(recvBuffer);
                if(messageHeader == nullptr) {
                    break;
                }
                const uint64_t messageSize = process_Data_Single_Dynamic(session,
                                                                         messageHeader,
                                                                         recvBuffer);
                return messageSize;
            }
        case DATA_SINGLE_REPLY_SUBTYPE:
            {
                const Data_SingleReply_Message* message =
                        getObjectFromBuffer<Data_SingleReply_Message>(recvBuffer);
                if(message == nullptr) {
                    break;
                }
                process_Data_Single_Reply(session, message);
                return sizeof(*message);
            }
        default:
            break;
    }

    return 0;
}

} // namespace Common
} // namespace Project
} // namespace Kitsune

#endif // SINGLE_DATA_PROCESSING_H
