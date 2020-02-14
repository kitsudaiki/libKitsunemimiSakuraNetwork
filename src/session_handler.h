/**
 * @file       session_handler.h
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

#ifndef RESSOURCE_HANDLER_H
#define RESSOURCE_HANDLER_H

#include <iostream>
#include <vector>
#include <map>
#include <atomic>
#include <message_definitions.h>

namespace Kitsunemimi
{
namespace Network {
class AbstractServer;
}
namespace Project
{
class Session;
class ReplyHandler;
class MessageBlockerHandler;
class SessionController;

class SessionHandler
{
public:

    static Kitsunemimi::Project::ReplyHandler* m_replyHandler;
    static Kitsunemimi::Project::MessageBlockerHandler* m_blockerHandler;
    static Kitsunemimi::Project::SessionController* m_sessionController;
    static Kitsunemimi::Project::SessionHandler* m_sessionHandler;

    SessionHandler(void* sessionTarget,
                   void (*processSession)(void*, bool, Session*, const uint64_t),
                   void* streamDataTarget,
                   void (*processStreamData)(void*, Session*, const void*, const uint64_t),
                   void* standaloneDataTarget,
                   void (*processData)(void*, Session*, const uint64_t,
                                       const void*, const uint64_t),
                   void* errorTarget,
                   void (*processError)(void*, Session*, const uint8_t, const std::string));
    ~SessionHandler();

    // session-control
    void addSession(const uint32_t id, Session* session);
    Session* removeSession(const uint32_t id);
    void sendHeartBeats();

    // counter
    uint16_t increaseSessionIdCounter();

    void lockSessionMap();
    void unlockSessionMap();
    void lockServerMap();
    void unlockServerMap();

    // object-holder
    std::map<uint32_t, Session*> m_sessions;
    std::map<uint32_t, Network::AbstractServer*> m_servers;

    void sendMessage(Session *session,
                     const CommonMessageHeader &header,
                     const void* data,
                     const uint64_t size);
private:
    // counter
    uint16_t m_sessionIdCounter = 0;
    std::atomic_flag m_sessionMap_lock = ATOMIC_FLAG_INIT;
    std::atomic_flag m_serverMap_lock = ATOMIC_FLAG_INIT;
    std::atomic_flag m_sessionIdCounter_lock = ATOMIC_FLAG_INIT;

    // callbacks
    void* m_sessionTarget = nullptr;
    void (*m_processSession)(void*, bool, Session*, const uint64_t);
    void* m_streamDataTarget = nullptr;
    void (*m_processStreamData)(void*, Session*, const void*, const uint64_t);
    void* m_standaloneDataTarget = nullptr;
    void (*m_processStandaloneData)(void*, Session*, const uint64_t, const void*, const uint64_t);
    void* m_errorTarget = nullptr;
    void (*m_processError)(void*, Session*, const uint8_t, const std::string);
};

} // namespace Project
} // namespace Kitsunemimi

#endif // RESSOURCE_HANDLER_H
