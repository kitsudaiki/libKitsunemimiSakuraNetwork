/**
 *  @file       internal_session_interface.h
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

#ifndef INTERNAL_SESSION_INTERFACE_H
#define INTERNAL_SESSION_INTERFACE_H

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

class InternalSessionInterface
{
public:
    InternalSessionInterface();

    // callback-forwarding
    void receivedData(Session* session, void* data, const uint32_t dataSize);
    void receivedError(Session* session, const uint8_t errorCode, const std::string message);

    bool sendMessage(Session* session, const void* data, const uint32_t size);

    bool connectiSession(Session* session,
                         const uint32_t sessionId,
                         const bool init = false);
    bool makeSessionReady(Session* session,
                          const uint32_t sessionId);

    bool endSession(Session* session,
                    const bool init = false,
                    const bool replyExpected = false);
    bool disconnectSession(Session* session);
};

} // namespace Common
} // namespace Project
} // namespace Kitsune

#endif // INTERNAL_SESSION_INTERFACE_H
