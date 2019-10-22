/**
 *  @file       data_processing.h
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

#ifndef DATA_PROCESSING_H
#define DATA_PROCESSING_H

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
send_Data_Single_Static(const uint32_t sessionId,
                        const void* data,
                        uint64_t size,
                        AbstractSocket* socket)
{
    LOG_DEBUG("SEND data single static");

    Data_SingleStatic_Message message(sessionId,
                                      SessionHandler::m_sessionHandler->increaseMessageIdCounter());

    memcpy(message.payload, data, size);
    message.payloadSize = size;

    socket->sendMessage(&message, sizeof(message));
}

/**
 * @brief send_Data_Single_Dynamic
 */
inline void
send_Data_Single_Dynamic(const uint32_t sessionId,
                         const void* data,
                         const uint64_t size,
                         AbstractSocket* socket)
{
    LOG_DEBUG("SEND data single dynamic");

    const uint64_t totalMessageSize = sizeof(Data_SingleDynamic_Message)
                                    + size
                                    + sizeof(CommonMessageEnd);

    uint8_t completeMessage[totalMessageSize];
    Data_SingleDynamic_Message header(sessionId,
                                      SessionHandler::m_sessionHandler->increaseMessageIdCounter());
    header.payloadSize = size;
    CommonMessageEnd end;

    memcpy(&completeMessage[0], &header, sizeof(Data_SingleDynamic_Message));
    memcpy(&completeMessage[sizeof(Data_SingleDynamic_Message)], data, size);
    memcpy(&completeMessage[sizeof(Data_SingleDynamic_Message) + size],
           &end,
           sizeof(CommonMessageEnd));

    socket->sendMessage(completeMessage, totalMessageSize);
}

/**
 * @brief send_Data_Single_Reply
 */
inline void
send_Data_Single_Reply(const uint32_t sessionId,
                       const uint32_t messageId,
                       AbstractSocket* socket)
{
    LOG_DEBUG("SEND data single reply");

    Data_SingleReply_Message message(sessionId, messageId);
    socket->sendMessage(&message, sizeof(message));
}

/**
 * @brief process_Heartbeat_Start
 */
inline void
process_Data_Single_Static(Session* session,
                           const Data_SingleStatic_Message* message,
                           AbstractSocket* socket)
{
    LOG_DEBUG("process data single static");

    SessionHandler::m_sessionInterface->receivedData(session,
                                                     static_cast<const void*>(message->payload),
                                                     message->payloadSize);

    send_Data_Single_Reply(message->commonHeader.sessionId,
                           message->commonHeader.messageId,
                           socket);
}

/**
 * @brief process_Data_Single_Dynamic
 * @param session
 * @param message
 * @param socket
 */
inline uint64_t
process_Data_Single_Dynamic(Session* session,
                            const Data_SingleDynamic_Message* message,
                            MessageRingBuffer* recvBuffer,
                            AbstractSocket* socket)
{
    LOG_DEBUG("process data single dynamic");

    const uint64_t totalMessageSize = sizeof(Data_SingleDynamic_Message)
                                    + message->payloadSize
                                    + sizeof(CommonMessageEnd);

    const uint8_t* completeMessage = getDataPointer(*recvBuffer, totalMessageSize);
    if(completeMessage == nullptr) {
        return 0;
    }

    const void* payload = completeMessage + sizeof(Data_SingleDynamic_Message);
    SessionHandler::m_sessionInterface->receivedData(session,
                                                     payload,
                                                     message->payloadSize);

    send_Data_Single_Reply(message->commonHeader.sessionId,
                           message->commonHeader.messageId,
                           socket);

    return totalMessageSize;
}

/**
 * @brief process_Heartbeat_Reply
 */
inline void
process_Data_Single_Reply(Session*,
                          const Data_SingleReply_Message*,
                          AbstractSocket*)
{
    LOG_DEBUG("process data single reply");
}

/**
 * @brief process_Data_Type
 * @param session
 * @param header
 * @param recvBuffer
 * @param socket
 * @return
 */
inline uint64_t
process_Data_Type(Session* session,
                  const CommonMessageHeader* header,
                  MessageRingBuffer* recvBuffer,
                  AbstractSocket* socket)
{
    LOG_DEBUG("process heartbeat-type");
    switch(header->subType)
    {
        case DATA_SINGLE_STATIC_SUBTYPE:
            {
                const Data_SingleStatic_Message* message =
                        getObjectFromBuffer<Data_SingleStatic_Message>(recvBuffer);
                if(message == nullptr) {
                    break;
                }
                process_Data_Single_Static(session, message, socket);
                return sizeof(*message);
            }
        case DATA_SINGLE_DYNAMIC_SUBTYPE:
            {
                const Data_SingleDynamic_Message* messageHeader =
                        getObjectFromBuffer<Data_SingleDynamic_Message>(recvBuffer);
                if(messageHeader == nullptr) {
                    break;
                }
                const uint64_t messageSize = process_Data_Single_Dynamic(session,
                                                                         messageHeader,
                                                                         recvBuffer,
                                                                         socket);
                return messageSize;
            }
        case DATA_SINGLE_REPLY_SUBTYPE:
            {
                const Data_SingleReply_Message* message =
                        getObjectFromBuffer<Data_SingleReply_Message>(recvBuffer);
                if(message == nullptr) {
                    break;
                }
                process_Data_Single_Reply(session, message, socket);
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

#endif // DATA_PROCESSING_H
