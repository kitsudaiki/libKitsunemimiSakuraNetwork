/**
 *  @file       heartbeat_processing.h
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

#ifndef HEARTBEAT_PROCESSING_H
#define HEARTBEAT_PROCESSING_H

#include <libKitsuneProjectCommon/network_session/session_handler.h>
#include <network_session/messages/message_definitions.h>
#include <libKitsuneNetwork/abstract_socket.h>

#include <libKitsunePersistence/logger/logger.h>

using Kitsune::Network::AbstractSocket;

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
send_Heartbeat_Start(const uint32_t sessionId,
                    Network::AbstractSocket* socket)
{
    LOG_DEBUG("SEND heartbeat start");

    Heartbeat_Start_Message message(sessionId,
                                    SessionHandler::m_sessionHandler->increaseMessageIdCounter());
    socket->sendMessage(&message, sizeof(message));
}

/**
 * @brief send_Heartbeat_Reply
 * @param id
 * @param socket
 */
inline void
send_Heartbeat_Reply(const uint32_t sessionId,
                     const uint32_t messageId,
                     Network::AbstractSocket* socket)
{
    LOG_DEBUG("SEND heartbeat reply");

    Heartbeat_Reply_Message message(sessionId, messageId);
    socket->sendMessage(&message, sizeof(message));
}

/**
 * @brief process_Heartbeat_Start
 */
inline void
process_Heartbeat_Start(const Heartbeat_Start_Message* message,
                        AbstractSocket* socket)
{
    LOG_DEBUG("process heartbeat start");

    send_Heartbeat_Reply(message->commonHeader.sessionId,
                         message->commonHeader.messageId,
                         socket);
}

/**
 * @brief process_Heartbeat_Reply
 */
inline void
process_Heartbeat_Reply(const Heartbeat_Reply_Message* message,
                        AbstractSocket*)
{
    LOG_DEBUG("process heartbeat reply");

    SessionHandler::m_timerThread->removeMessage(message->commonHeader.sessionId,
                                                 message->commonHeader.messageId);
}

} // namespace Common
} // namespace Project
} // namespace Kitsune

#endif // HEARTBEAT_PROCESSING_H
