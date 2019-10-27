/**
 *  @file       session_handler.cpp
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

#include "session_handler.h"

#include <network_session/timer_thread.h>
#include <network_session/session_handler.h>
#include <network_session/message_definitions.h>
#include <network_session/internal_session_interface.h>

#include <libKitsuneProjectCommon/network_session/session.h>
#include <libKitsuneProjectCommon/network_session/session_controller.h>

#include <libKitsunePersistence/logger/logger.h>

namespace Kitsune
{
namespace Project
{
namespace Common
{

TimerThread* SessionHandler::m_timerThread = nullptr;
SessionHandler* SessionHandler::m_sessionHandler = nullptr;
InternalSessionInterface* SessionHandler::m_sessionInterface = nullptr;

SessionHandler::SessionHandler(void* sessionTarget,
                               void (*processSession)(void*, Session*, const uint64_t),
                               void* dataTarget,
                               void (*processData)(void*, Session*, const bool,
                                                   const void*, const uint64_t),
                               void* errorTarget,
                               void (*processError)(void*, Session*,
                                                    const uint8_t, const std::string))
{
    if(m_sessionInterface == nullptr)
    {
        m_sessionInterface = new InternalSessionInterface(sessionTarget,
                                                          processSession,
                                                          dataTarget,
                                                          processData,
                                                          errorTarget,
                                                          processError);
    }

    if(m_timerThread == nullptr)
    {
        m_timerThread = new TimerThread();
        m_timerThread->start();
    }

    // check if messages have the size of a multiple of 8
    assert(sizeof(Session_Init_Start_Message) % 8 == 0);
    assert(sizeof(Session_Init_Reply_Message) % 8 == 0);
    assert(sizeof(Session_Close_Start_Message) % 8 == 0);
    assert(sizeof(Session_Close_Reply_Message) % 8 == 0);
    assert(sizeof(Heartbeat_Start_Message) % 8 == 0);
    assert(sizeof(Heartbeat_Reply_Message) % 8 == 0);
    assert(sizeof(Error_FalseVersion_Message) % 8 == 0);
    assert(sizeof(Error_UnknownSession_Message) % 8 == 0);
    assert(sizeof(Error_InvalidMessage_Message) % 8 == 0);
    assert(sizeof(Data_SingleStatic_Message) % 8 == 0);
    assert(sizeof(Data_SingleDynamic_Header) % 8 == 0);
    assert(sizeof(Data_SingleReply_Message) % 8 == 0);
    assert(sizeof(Data_MultiInit_Message) % 8 == 0);
    assert(sizeof(Data_MultiInitReply_Message) % 8 == 0);
    assert(sizeof(Data_MultiStatic_Message) % 8 == 0);
    assert(sizeof(Data_MultiFinish_Message) % 8 == 0);
    assert(sizeof(Data_MultiAbort_Message) % 8 == 0);
}

/**
 * @brief SessionHandler::~SessionHandler
 */
SessionHandler::~SessionHandler()
{
    while (m_sessionMap_lock.test_and_set(std::memory_order_acquire))  // acquire lock
                 ; // spin
    m_sessions.clear();
    m_servers.clear();
    m_sessionMap_lock.clear(std::memory_order_release);

    if(m_sessionInterface != nullptr)
    {
        delete m_sessionInterface;
        m_sessionInterface = nullptr;
    }

    if(m_timerThread != nullptr)
    {
        delete m_timerThread;
        m_timerThread = nullptr;
    }
}

/**
 * @brief SessionHandler::addSession
 * @param id
 * @param session
 */
void
SessionHandler::addSession(const uint32_t id, Session* session)
{
    LOG_DEBUG("add session with id: " + std::to_string(id));
    while (m_sessionMap_lock.test_and_set(std::memory_order_acquire))  // acquire lock
                 ; // spin
    m_sessions.insert(std::pair<uint32_t, Session*>(id, session));
    m_sessionMap_lock.clear(std::memory_order_release);
}

/**
 * @brief SessionHandler::removeSession
 * @param id
 */
Session*
SessionHandler::removeSession(const uint32_t id)
{
    LOG_DEBUG("remove session with id: " + std::to_string(id));

    Session* ret = nullptr;
    while (m_sessionMap_lock.test_and_set(std::memory_order_acquire))  // acquire lock
                 ; // spin
    std::map<uint32_t, Session*>::iterator it;
    it = m_sessions.find(id);

    if(it != m_sessions.end())
    {
        ret = it->second;
        m_sessions.erase(it);
    }
    m_sessionMap_lock.clear(std::memory_order_release);

    return ret;
}

/**
 * @brief SessionHandler::increaseSessionIdCounter
 * @return
 */
uint16_t
SessionHandler::increaseSessionIdCounter()
{
    uint16_t tempId = 0;
    while (m_sessionIdCounter_lock.test_and_set(std::memory_order_acquire))  // acquire lock
                 ; // spin
    m_sessionIdCounter++;
    tempId = m_sessionIdCounter;
    m_sessionIdCounter_lock.clear(std::memory_order_release);
    return tempId;
}

/**
 * @brief RessourceHandler::sendHeartBeats
 */
void
SessionHandler::sendHeartBeats()
{
    while (m_sessionMap_lock.test_and_set(std::memory_order_acquire))  // acquire lock
                 ; // spin
    std::map<uint32_t, Session*>::iterator it;
    for(it = m_sessions.begin(); it != m_sessions.end(); it++)
    {
        m_sessionInterface->sendHeartbeat(it->second);
    }
    m_sessionMap_lock.clear(std::memory_order_release);
}

} // namespace Common
} // namespace Project
} // namespace Kitsune
