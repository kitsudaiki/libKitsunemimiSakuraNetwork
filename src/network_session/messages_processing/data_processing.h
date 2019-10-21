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

namespace Kitsune
{
namespace Project
{
namespace Common
{

/**
 * @brief send_Heartbeat_Start
 * @param id
 * @param socket
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

    memcpy(message.message, data, size);
    message.messageSize = size;

    socket->sendMessage(&message, sizeof(message));
}

/**
 * @brief send_Heartbeat_Reply
 * @param id
 * @param socket
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

    uint8_t* data = new uint8_t[message->messageSize];
    mempcpy(data, message->message, message->messageSize);

    SessionHandler::m_sessionInterface->receivedData(session, data, message->messageSize);

    send_Data_Single_Reply(message->commonHeader.sessionId,
                           message->commonHeader.messageId,
                           socket);
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
                  MessageRingBuffer *recvBuffer,
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
