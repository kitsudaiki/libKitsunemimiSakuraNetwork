/**
 *  @file       internal_session_interface.cpp
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

#include "internal_session_interface.h"

#include <libKitsuneProjectCommon/network_session/session.h>

#include <network_session/message_definitions.h>
#include <network_session/session_handler.h>

#include <libKitsuneNetwork/abstract_socket.h>

namespace Kitsune
{
namespace Project
{
namespace Common
{

InternalSessionInterface::InternalSessionInterface(void* sessionTarget,
                                                   void (*processSession)(void*, Session*),
                                                   void* dataTarget,
                                                   void (*processData)(void*, Session*,
                                                                       void*, const uint64_t),
                                                   void* errorTarget,
                                                   void (*processError)(void*, Session*,
                                                                        const uint8_t,
                                                                        const std::string))
{
    m_sessionTarget = sessionTarget;
    m_processSession = processSession;
    m_dataTarget = dataTarget;
    m_processData = processData;
    m_errorTarget = errorTarget;
    m_processError = processError;
}

/**
 * @brief RessourceHandler::receivedData
 * @param session
 * @param data
 * @param dataSize
 */
void
InternalSessionInterface::receivedData(Session* session,
                                       void* data,
                                       const uint64_t dataSize)
{
    session->m_processData(session->m_dataTarget, session, data, dataSize);
}

/**
 * @brief RessourceHandler::receivedError
 * @param session
 * @param errorCode
 * @param message
 */
void
InternalSessionInterface::receivedError(Session* session,
                                        const uint8_t errorCode,
                                        const std::string message)
{
    session->m_processError(session->m_errorTarget, session, errorCode, message);
}

/**
 * @brief RessourceHandler::sendMessage
 * @param session
 * @param data
 * @param size
 * @return
 */
bool
InternalSessionInterface::sendMessage(Session* session,
                                      const void* data,
                                      const uint32_t size)
{
    const CommonMessageHeader* header = static_cast<const CommonMessageHeader*>(data);

    if(header->flags == 0x1) {
        SessionHandler::m_timerThread->addMessage(header->type,
                                                  header->sessionId,
                                                  header->messageId);
    }

    return session->m_socket->sendMessage(data, size);
}

/**
 * @brief InternalSessionInterface::sendHeartbeat
 * @param session
 */
void
InternalSessionInterface::sendHeartbeat(Session *session)
{
    session->sendHeartbeat();
}

/**
 * @brief RessourceHandler::connectiSession
 * @param session
 * @param sessionId
 * @param init
 * @return
 */
bool
InternalSessionInterface::connectiSession(Session* session,
                                          const uint32_t sessionId,
                                          const bool init)
{
    session->m_sessionTarget = m_sessionTarget;
    session->m_processSession = m_processSession;
    session->m_dataTarget = m_dataTarget;
    session->m_processData = m_processData;
    session->m_errorTarget = m_errorTarget;
    session->m_processError = m_processError;

    return session->connectiSession(sessionId, init);
}

/**
 * @brief RessourceHandler::makeSessionReady
 * @param session
 * @param sessionId
 * @return
 */
bool
InternalSessionInterface::makeSessionReady(Session* session,
                                           const uint32_t sessionId)
{
    session->m_sessionId = sessionId;
    return session->makeSessionReady();
}

/**
 * @brief RessourceHandler::endSession
 * @param session
 * @param init
 * @param replyExpected
 * @return
 */
bool
InternalSessionInterface::endSession(Session* session,
                                     const bool init,
                                     const bool replyExpected)
{
    return session->endSession(init, replyExpected);
}

/**
 * @brief RessourceHandler::disconnectSession
 * @param session
 * @return
 */
bool
InternalSessionInterface::disconnectSession(Session* session)
{
    return session->disconnectSession();
}

} // namespace Common
} // namespace Project
} // namespace Kitsune
