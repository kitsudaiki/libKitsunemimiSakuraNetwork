/**
 *  @file       ressource_handler.cpp
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

#include "ressource_handler.h"

#include <network_session/timer_thread.h>
#include <network_session/ressource_handler.h>

#include <libKitsuneProjectCommon/network_session/session.h>
#include <libKitsuneProjectCommon/network_session/session_handler.h>

#include <libKitsuneNetwork/tcp/tcp_server.h>
#include <libKitsuneNetwork/tcp/tcp_socket.h>
#include <libKitsuneNetwork/unix/unix_domain_server.h>
#include <libKitsuneNetwork/unix/unix_domain_socket.h>
#include <libKitsuneNetwork/tls_tcp/tls_tcp_server.h>
#include <libKitsuneNetwork/tls_tcp/tls_tcp_socket.h>
#include <libKitsuneNetwork/abstract_server.h>

#include <libKitsunePersistence/logger/logger.h>

namespace Kitsune
{
namespace Project
{
namespace Common
{

TimerThread* RessourceHandler::m_timerThread = nullptr;
RessourceHandler* RessourceHandler::m_ressourceHandler = nullptr;

RessourceHandler::RessourceHandler(void* dataTarget,
                                   void (*processData)(void*, Session*,
                                                       void*, const uint32_t),
                                   void* errorTarget,
                                   void (*processError)(void*, Session*,
                                                        const uint8_t, const std::string))
{
    m_dataTarget = dataTarget;
    m_processData = processData;
    m_errorTarget = errorTarget;
    m_processError = processError;

    if(m_timerThread == nullptr)
    {
        m_timerThread = new TimerThread;
        m_timerThread->start();
    }
}

/**
 * @brief RessourceHandler::receivedData
 * @param session
 * @param data
 * @param dataSize
 */
void
RessourceHandler::receivedData(Session* session,
                               void* data,
                               const uint32_t dataSize)
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
RessourceHandler::receivedError(Session* session,
                                const uint8_t errorCode,
                                const std::string message)
{
    session->m_processError(session->m_errorTarget, session, errorCode, message);
}

/**
 * @brief SessionHandler::addSession
 * @param id
 * @param session
 */
void
RessourceHandler::addSession(const uint32_t id, Session* session)
{
    LOG_DEBUG("add session with id: " + std::to_string(id));
    m_sessions.insert(std::pair<uint32_t, Session*>(id, session));
}

/**
 * @brief SessionHandler::removeSession
 * @param id
 */
Session*
RessourceHandler::removeSession(const uint32_t id)
{
    LOG_DEBUG("remove session with id: " + std::to_string(id));
    std::map<uint32_t, Session*>::iterator it;
    it = m_sessions.find(id);

    if(it != m_sessions.end())
    {
        Session* tempSession = it->second;
        m_sessions.erase(it);
        return tempSession;
    }

    return nullptr;
}

/**
 * @brief RessourceHandler::connectiSession
 * @param session
 * @param sessionId
 * @param init
 * @return
 */
bool
RessourceHandler::connectiSession(Session* session,
                                  const uint32_t sessionId,
                                  const bool init)
{
    return session->connectiSession(sessionId, init);
}

/**
 * @brief RessourceHandler::makeSessionReady
 * @param session
 * @param sessionId
 * @return
 */
bool
RessourceHandler::makeSessionReady(Session* session,
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
RessourceHandler::endSession(Session* session,
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
RessourceHandler::disconnectSession(Session* session)
{
    return session->disconnectSession();
}

/**
 * @brief SessionHandler::isIdUsed
 * @param id
 * @return
 */
bool
RessourceHandler::isIdUsed(const uint32_t id)
{
    std::map<uint32_t, Session*>::iterator it;
    it = m_sessions.find(id);

    if(it != m_sessions.end()) {
        return true;
    }

    return false;
}

/**
 * @brief SessionHandler::increaseMessageIdCounter
 * @return
 */
uint32_t
RessourceHandler::increaseMessageIdCounter()
{
    uint32_t tempId = 0;
    while (m_messageIdCounter_lock.test_and_set(std::memory_order_acquire))  // acquire lock
                 ; // spin
    m_messageIdCounter++;
    tempId = m_messageIdCounter;
    m_messageIdCounter_lock.clear(std::memory_order_release);
    return tempId;
}

/**
 * @brief SessionHandler::increaseSessionIdCounter
 * @return
 */
uint16_t
RessourceHandler::increaseSessionIdCounter()
{
    uint16_t tempId = 0;
    while (m_sessionIdCounter_lock.test_and_set(std::memory_order_acquire))  // acquire lock
                 ; // spin
    m_sessionIdCounter++;
    tempId = m_sessionIdCounter;
    m_sessionIdCounter_lock.clear(std::memory_order_release);
    return tempId;
}

} // namespace Common
} // namespace Project
} // namespace Kitsune
