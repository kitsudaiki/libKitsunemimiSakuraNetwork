/**
 *  @file       message_creation.h
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

#ifndef MESSAGE_CREATION_H
#define MESSAGE_CREATION_H

#include <network_session/messages/message_definitions.h>
#include <libKitsuneProjectCommon/network_session/session_handler.h>
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
 * @brief sendSessionInitStart
 * @param initialId
 * @param socket
 */
inline void
sendSessionInitStart(const uint32_t initialId,
                     Network::AbstractSocket* socket)
{
    LOG_DEBUG("SEND session init start");

    // create message
    SessionInit_Start_Message message;
    message.offeredSessionId = initialId;

    // update common-header
    message.commonHeader.sessionId = initialId;
    message.commonHeader.messageId = SessionHandler::m_sessionHandler->increaseMessageIdCounter();

    // send
    socket->sendMessage(&message, sizeof(SessionInit_Start_Message));
}

/**
 * @brief sendSessionIdChange
 * @param oldId
 * @param newId
 * @param socket
 */
inline void
sendSessionIdChange(const uint32_t oldId,
                    const uint32_t newId,
                    Network::AbstractSocket* socket)
{
    LOG_DEBUG("SEND session id change");

    // create message
    SessionInit_IdChange_Message message;
    message.oldOfferedSessionId = oldId;
    message.newOfferedSessionId = newId;

    // update common-header
    message.commonHeader.sessionId = newId;
    message.commonHeader.messageId = SessionHandler::m_sessionHandler->increaseMessageIdCounter();

    // send
    socket->sendMessage(&message, sizeof(SessionInit_Start_Message));
}

/**
 * @brief sendSessionIniReply
 * @param id
 * @param socket
 */
inline void
sendSessionInitReply(const uint32_t id,
                    Network::AbstractSocket* socket)
{
    LOG_DEBUG("SEND session init reply");

    SessionInit_Reply_Message message;
    message.sessionId = id;

    // update common-header
    message.commonHeader.sessionId = id;
    message.commonHeader.messageId = SessionHandler::m_sessionHandler->increaseMessageIdCounter();

    // send
    socket->sendMessage(&message, sizeof(SessionInit_Start_Message));
}

} // namespace Common
} // namespace Project
} // namespace Kitsune

#endif // MESSAGE_CREATION_H
