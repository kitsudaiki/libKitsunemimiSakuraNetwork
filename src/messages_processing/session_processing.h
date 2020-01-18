/**
 * @file       session_processing.h
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

#ifndef SESSION_PROCESSING_H
#define SESSION_PROCESSING_H

#include <message_definitions.h>
#include <session_handler.h>
#include <multiblock_io.h>

#include <libKitsunemimiNetwork/abstract_socket.h>

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
 * @brief send_Session_Init_Start
 *
 * @param session pointer to the session
 * @param sessionIdentifier custom value, which is sended within the init-message to pre-identify
 *                          the message on server-side
 */
inline void
send_Session_Init_Start(Session* session,
                        const uint64_t sessionIdentifier)
{
    LOG_DEBUG("SEND session init start");

    Session_Init_Start_Message message(session->sessionId(),
                                       session->increaseMessageIdCounter(),
                                       sessionIdentifier);

    SessionHandler::m_sessionHandler->sendMessage(session,
                                                  message.commonHeader,
                                                  &message,
                                                  sizeof(message));
}

/**
 * @brief send_Session_Init_Reply
 *
 * @param session pointer to the session
 * @param initialSessionId initial id, which was sended by the client
 * @param messageId id of the original incoming message
 * @param completeSessionId completed session-id based on the id of the server and the client
 */
inline void
send_Session_Init_Reply(Session* session,
                        const uint32_t initialSessionId,
                        const uint32_t messageId,
                        const uint32_t completeSessionId)
{
    LOG_DEBUG("SEND session init reply");

    Session_Init_Reply_Message message(initialSessionId, messageId);
    message.completeSessionId = completeSessionId;
    message.clientSessionId = initialSessionId;

    SessionHandler::m_sessionHandler->sendMessage(session,
                                                  message.commonHeader,
                                                  &message,
                                                  sizeof(message));
}

/**
 * @brief send_Session_Close_Start
 *
 * @param session pointer to the session
 * @param replyExpected set to true to get a reply-message for the session-close-message
 */
inline void
send_Session_Close_Start(Session* session,
                         bool replyExpected)
{
    LOG_DEBUG("SEND session close start");

    Session_Close_Start_Message message(session->sessionId(),
                                        session->increaseMessageIdCounter(),
                                        replyExpected);

    SessionHandler::m_sessionHandler->sendMessage(session,
                                                  message.commonHeader,
                                                  &message,
                                                  sizeof(message));
}

/**
 * @brief send_Session_Close_Reply
 *
 * @param session pointer to the session
 * @param messageId id of the original incoming message
 */
inline void
send_Session_Close_Reply(Session* session,
                         const uint32_t messageId)
{
    LOG_DEBUG("SEND session close reply");

    Session_Close_Reply_Message message(session->sessionId(), messageId);

    SessionHandler::m_sessionHandler->sendMessage(session,
                                                  message.commonHeader,
                                                  &message,
                                                  sizeof(message));
}

/**
 * @brief process_Session_Init_Start
 *
 * @param session pointer to the session
 * @param message pointer to the complete message within the message-ring-buffer
 */
inline void
process_Session_Init_Start(Session* session,
                           const Session_Init_Start_Message* message)
{
    LOG_DEBUG("process session init start");

    // get and calculate session-id
    const uint32_t clientSessionId = message->clientSessionId;
    const uint16_t serverSessionId = SessionHandler::m_sessionHandler->increaseSessionIdCounter();
    const uint32_t sessionId = clientSessionId + (serverSessionId * 0x10000);
    const uint64_t sessionIdentifier = message->sessionIdentifier;

    // create new session and make it ready
    SessionHandler::m_sessionHandler->addSession(sessionId, session);
    session->connectiSession(sessionId, false);
    session->makeSessionReady(sessionId, sessionIdentifier);

    // confirm id
    send_Session_Init_Reply(session,
                            clientSessionId,
                            message->commonHeader.messageId,
                            sessionId);
}

/**
 * @brief process_Session_Init_Reply
 *
 * @param session pointer to the session
 * @param message pointer to the complete message within the message-ring-buffer
 */
inline void
process_Session_Init_Reply(Session* session,
                           const Session_Init_Reply_Message* message)
{
    LOG_DEBUG("process session init reply");

    const uint32_t completeSessionId = message->completeSessionId;
    const uint32_t initialId = message->clientSessionId;

    // readd session under the new complete session-id and make session ready
    SessionHandler::m_sessionHandler->removeSession(initialId);
    SessionHandler::m_sessionHandler->addSession(completeSessionId, session);
    // TODO: handle return-value of makeSessionReady
    session->makeSessionReady(completeSessionId, 0);
}

/**
 * @brief process_Session_Close_Start
 *
 * @param session pointer to the session
 * @param message pointer to the complete message within the message-ring-buffer
 */
inline void
process_Session_Close_Start(Session* session,
                            const Session_Close_Start_Message* message)
{
    LOG_DEBUG("process session close start");

    send_Session_Close_Reply(session,
                             message->commonHeader.messageId);

    // close session and disconnect session
    SessionHandler::m_sessionHandler->removeSession(message->sessionId);
    session->endSession();
    session->disconnectSession();
}

/**
 * @brief process_Session_Close_Reply
 *
 * @param session pointer to the session
 * @param message pointer to the complete message within the message-ring-buffer
 */
inline void
process_Session_Close_Reply(Session* session,
                            const Session_Close_Reply_Message* message)
{
    LOG_DEBUG("process session close reply");

    // disconnect session
    SessionHandler::m_sessionHandler->removeSession(message->sessionId);
    session->disconnectSession();
}

/**
 * @brief process messages of session-type
 *
 * @param session pointer to the session
 * @param header pointer to the common header of the message within the message-ring-buffer
 * @param recvBuffer pointer to the message-ring-buffer
 *
 * @return number of processed bytes
 */
inline uint64_t
process_Session_Type(Session* session,
                     const CommonMessageHeader* header,
                     MessageRingBuffer *recvBuffer)
{
    if(DEBUG_MODE) {
        LOG_DEBUG("process session-type");
    }

    switch(header->subType)
    {
        //------------------------------------------------------------------------------------------
        case SESSION_INIT_START_SUBTYPE:
            {
                const Session_Init_Start_Message* message =
                        getObjectFromBuffer<Session_Init_Start_Message>(recvBuffer);
                if(message == nullptr) {
                    break;
                }
                process_Session_Init_Start(session, message);
                return sizeof(*message);
            }
        //------------------------------------------------------------------------------------------
        case SESSION_INIT_REPLY_SUBTYPE:
            {
                const Session_Init_Reply_Message* message =
                        getObjectFromBuffer<Session_Init_Reply_Message>(recvBuffer);
                if(message == nullptr) {
                    break;
                }
                process_Session_Init_Reply(session, message);
                return sizeof(*message);
            }
        //------------------------------------------------------------------------------------------
        case SESSION_CLOSE_START_SUBTYPE:
            {
                const Session_Close_Start_Message* message =
                        getObjectFromBuffer<Session_Close_Start_Message>(recvBuffer);
                if(message == nullptr) {
                    break;
                }
                process_Session_Close_Start(session, message);
                return sizeof(*message);
            }
        //------------------------------------------------------------------------------------------
        case SESSION_CLOSE_REPLY_SUBTYPE:
            {
                const Session_Close_Reply_Message* message =
                        getObjectFromBuffer<Session_Close_Reply_Message>(recvBuffer);
                if(message == nullptr) {
                    break;
                }
                process_Session_Close_Reply(session, message);
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

#endif // SESSION_PROCESSING_H
