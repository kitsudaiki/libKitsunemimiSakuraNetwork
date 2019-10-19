/**
 *  @file       ressource_handler.h
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

#ifndef RESSOURCE_HANDLER_H
#define RESSOURCE_HANDLER_H

#include <iostream>
#include <vector>
#include <map>
#include <atomic>

namespace Kitsune
{
namespace Network {
class AbstractServer;
}
namespace Project
{
namespace Common
{
class Session;
class TimerThread;
class SessionController;

class RessourceHandler
{
public:

    static Kitsune::Project::Common::TimerThread* m_timerThread;
    static Kitsune::Project::Common::SessionController* m_sessionController;
    static Kitsune::Project::Common::RessourceHandler* m_ressourceHandler;

    RessourceHandler(void* dataTarget,
                     void (*processData)(void*, Session*, void*, const uint32_t),
                     void* errorTarget,
                     void (*processError)(void*, Session*, const uint8_t, const std::string));

    // callback-forwarding
    void receivedData(Session* session, void* data, const uint32_t dataSize);
    void receivedError(Session* session, const uint8_t errorCode, const std::string message);

    // session-control
    void addSession(const uint32_t id, Session* session);
    Session* removeSession(const uint32_t id);
    bool connectiSession(Session* session,
                         const uint32_t sessionId,
                         const bool init = false);
    bool makeSessionReady(Session* session,
                          const uint32_t sessionId);

    bool endSession(Session* session,
                    const bool init = false,
                    const bool replyExpected = false);
    bool disconnectSession(Session* session);

    // counter
    uint32_t increaseMessageIdCounter();
    uint16_t increaseSessionIdCounter();
    uint32_t m_serverIdCounter = 0;

    // callbacks
    void* m_dataTarget = nullptr;
    void (*m_processData)(void*, Session*, void*, const uint32_t);
    void* m_errorTarget = nullptr;
    void (*m_processError)(void*, Session*, const uint8_t, const std::string);

    // object-holder
    std::map<uint32_t, Session*> m_sessions;
    std::map<uint32_t, Network::AbstractServer*> m_servers;

private:
    // counter
    std::atomic_flag m_messageIdCounter_lock = ATOMIC_FLAG_INIT;
    uint32_t m_messageIdCounter = 0;
    std::atomic_flag m_sessionIdCounter_lock = ATOMIC_FLAG_INIT;
    uint16_t m_sessionIdCounter = 0;

    bool isIdUsed(const uint32_t id);
};

} // namespace Common
} // namespace Project
} // namespace Kitsune

#endif // RESSOURCE_HANDLER_H
