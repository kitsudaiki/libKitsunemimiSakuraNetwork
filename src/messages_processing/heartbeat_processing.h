/**
 * @file        heartbeat_processing.h
 *
 * @brief       send and handle messages of heartbeat-type
 *
 * @author      Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 * @copyright   Apache License Version 2.0
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

namespace Kitsunemimi
{
namespace Project
{

/**
 * @brief send the initial message
 *
 * @param session pointer to the session
 */
inline void
send_Heartbeat_Start(Session* session)
{
    Heartbeat_Start_Message message;
    create_Heartbeat_Start_Message(message,
                                   session->sessionId(),
                                   session->increaseMessageIdCounter());
    SessionHandler::m_sessionHandler->sendMessage(session,
                                                  message.commonHeader,
                                                  &message,
                                                  sizeof(message));
}

/**
 * @brief send reply-message
 *
 * @param session pointer to the session
 * @param id of the message of the initial heartbeat
 */
inline void
send_Heartbeat_Reply(Session* session,
                     const uint32_t messageId)
{
    Heartbeat_Reply_Message message;
    create_Heartbeat_Reply_Message(message,
                                   session->sessionId(),
                                   messageId);
    SessionHandler::m_sessionHandler->sendMessage(session,
                                                  message.commonHeader,
                                                  &message,
                                                  sizeof(message));
}

/**
 * @brief handle incoming heartbeats by sending a reply mesage
 *
 * @param session pointer to the session
 * @param message incoming message
 */
inline void
process_Heartbeat_Start(Session* session,
                        const Heartbeat_Start_Message* message)
{
    send_Heartbeat_Reply(session, message->commonHeader.messageId);
}

/**
 * @brief handle the reply-message, but do nothing here, because it is only important, that the
 *        message is arrived to be handled by the timer-thread
 */
inline void
process_Heartbeat_Reply(Session*,
                        const Heartbeat_Reply_Message*)
{
    return;
}

/**
 * @brief process messages of heartbeat-type
 *
 * @param session pointer to the session
 * @param header pointer to the common header of the message within the message-ring-buffer
 * @param recvBuffer pointer to the message-ring-buffer
 *
 * @return number of processed bytes
 */
inline void
process_Heartbeat_Type(Session* session,
                       const CommonMessageHeader* header,
                       const void* rawMessage)
{
    switch(header->subType)
    {
        //------------------------------------------------------------------------------------------
        case HEARTBEAT_START_SUBTYPE:
            {
                const Heartbeat_Start_Message* message =
                    static_cast<const Heartbeat_Start_Message*>(rawMessage);
                process_Heartbeat_Start(session, message);
                break;
            }
        //------------------------------------------------------------------------------------------
        case HEARTBEAT_REPLY_SUBTYPE:
            {
                const Heartbeat_Reply_Message* message =
                    static_cast<const Heartbeat_Reply_Message*>(rawMessage);
                process_Heartbeat_Reply(session, message);
                break;
            }
        //------------------------------------------------------------------------------------------
        default:
            break;
    }
}

} // namespace Project
} // namespace Kitsunemimi

#endif // HEARTBEAT_PROCESSING_H
