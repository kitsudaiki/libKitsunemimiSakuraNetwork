/**
 *  @file       session.h
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

#ifndef SESSION_H
#define SESSION_H

#include <iostream>
#include <assert.h>
#include <libKitsuneCommon/statemachine.h>

namespace Kitsune
{
namespace Common {
class Statemachine;
}
namespace Network {
class AbstractSocket;
}
namespace Project
{
namespace Common
{
class RessourceHandler;
class SessionController;

class Session
{
public:
    Session(Network::AbstractSocket* socket);
    ~Session(); 

    bool sendData(const void* data, const uint32_t size);
    bool closeSession(const bool replyExpected = false);

    uint32_t sessionId() const;

    enum errorCodes
    {
        UNDEFINED_ERROR = 0,
        FALSE_VERSION = 1,
        UNKNOWN_SESSION = 2,
        INVALID_MESSAGE_SIZE = 3,
        MESSAGE_TIMEOUT = 4,
    };

private:
    friend RessourceHandler;
    friend SessionController;

    bool connectiSession(const uint32_t sessionId,
                         const bool init = false);
    bool makeSessionReady();

    bool endSession(const bool init = false,
                    const bool replyExpected = false);
    bool disconnectSession();

    bool sendHeartbeat();
    void initStatemachine();

    Kitsune::Common::Statemachine m_statemachine;
    Network::AbstractSocket* m_socket = nullptr;
    uint32_t m_sessionId = 0;
    bool m_sessionReady = true;

    // callbacks
    void* m_dataTarget = nullptr;
    void (*m_processData)(void*, Session*, void*, const uint32_t);
    void* m_errorTarget = nullptr;
    void (*m_processError)(void*, Session*, const uint8_t, const std::string);
};

} // namespace Common
} // namespace Project
} // namespace Kitsune

#endif // SESSION_H
