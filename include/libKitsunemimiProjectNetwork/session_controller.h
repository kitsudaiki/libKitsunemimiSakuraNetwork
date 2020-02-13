/**
 * @file       session_controller.h
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

#ifndef SESSION_HANDLER_H
#define SESSION_HANDLER_H

#include <iostream>
#include <vector>
#include <map>
#include <atomic>

#include <libKitsunemimiProjectNetwork/session.h>

namespace Kitsunemimi
{
namespace Network {
class AbstractServer;
}
namespace Project
{

class SessionController
{
public:
    SessionController(void* sessionTarget,
                      void (*processSession)(void*, bool, Session*, const uint64_t),
                      void* streamDataTarget,
                      void (*processStreamData)(void*, Session*, const void*, const uint64_t),
                      void* standaloneDataTarget,
                      void (*processStandaloneData)(void*, Session*, const void*, const uint64_t),
                      void* errorTarget,
                      void (*processError)(void*, Session*, const uint8_t, const std::string));
    ~SessionController();

    static Kitsunemimi::Project::SessionController* m_sessionController;

    // server
    uint32_t addUnixDomainServer(const std::string socketFile);
    uint32_t addTcpServer(const uint16_t port);
    uint32_t addTlsTcpServer(const uint16_t port,
                             const std::string certFile,
                             const std::string keyFile);
    bool closeServer(const uint32_t id);
    void cloesAllServers();

    // session
    bool startUnixDomainSession(const std::string socketFile,
                                const uint64_t sessionIdentifier = 0);
    bool startTcpSession(const std::string address,
                         const uint16_t port,
                         const uint64_t sessionIdentifier = 0);
    bool startTlsTcpSession(const std::string address,
                            const uint16_t port,
                            const std::string certFile,
                            const std::string keyFile,
                            const uint64_t sessionIdentifier = 0);
    bool closeSession(const uint32_t id);
    Session* getSession(const uint32_t id);
    void closeAllSession();

private:
    uint32_t m_serverIdCounter = 0;

    bool startSession(Network::AbstractSocket* socket,
                      const uint64_t sessionIdentifier);
};

} // namespace Project
} // namespace Kitsunemimi

#endif // SESSION_HANDLER_H
