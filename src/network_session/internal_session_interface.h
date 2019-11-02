/**
 * @file       internal_session_interface.h
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

#ifndef INTERNAL_SESSION_INTERFACE_H
#define INTERNAL_SESSION_INTERFACE_H

#include <iostream>
#include <vector>
#include <map>
#include <atomic>

// additional debug-flag to enable debug-log in performance critical sections
#define DEBUG_MODE true

namespace Kitsunemimi
{
namespace Network {
class AbstractServer;
class AbstractSocket;
}
namespace Project
{
namespace Common
{
class Session;
struct CommonMessageHeader;
using Kitsunemimi::Project::Common::CommonMessageHeader;

class InternalSessionInterface
{
public:
    InternalSessionInterface(void* sessionTarget,
                             void (*processSession)(void*,
                                                    Session*,
                                                    const uint64_t),
                             void* dataTarget,
                             void (*processData)(void*,
                                                 Session*,
                                                 const bool,
                                                 const void*,
                                                 const uint64_t),
                             void* errorTarget,
                             void (*processError)(void*,
                                                  Session*,
                                                  const uint8_t,
                                                  const std::string));
    ~InternalSessionInterface();

    // callback-forwarding
    void receivedData(Session* session,
                      const bool isStream,
                      const void* data,
                      const uint64_t dataSize);
    void receivedError(Session* session,
                       const uint8_t errorCode,
                       const std::string &message);

    // send messages
    void sendMessage(Session* session,
                     const CommonMessageHeader &header,
                     const void* data,
                     const uint64_t size);
    void sendHeartbeat(Session* session);

    // multi-block data operations
    bool initMultiblockBuffer(Session* session,
                              const uint64_t size);
    bool writeDataIntoBuffer(Session* session,
                             const void* data,
                             const uint64_t size);
    bool finishMultiblockBuffer(Session* session);
    bool isInMultiblock(Session* session);

    uint64_t getUsedBufferSize(Session* session);
    uint8_t* getDataPointer(Session* session);

    // session-control
    Session* createNewSession(Network::AbstractSocket* socket);
    bool connectiSession(Session* session,
                         const uint32_t sessionId,
                         const uint64_t sessionIdentifier,
                         const bool init = false);
    bool makeSessionReady(Session* session,
                          const uint32_t sessionId,
                          const uint64_t sessionIdentifier);

    bool endSession(Session* session,
                    const bool init = false);
    bool disconnectSession(Session* session);

private:
    // callbacks
    void* m_sessionTarget = nullptr;
    void (*m_processSession)(void*, Session*, const uint64_t);
    void* m_dataTarget = nullptr;
    void (*m_processData)(void*, Session*, const bool, const void*, const uint64_t);
    void* m_errorTarget = nullptr;
    void (*m_processError)(void*, Session*, const uint8_t, const std::string);
};

} // namespace Common
} // namespace Project
} // namespace Kitsunemimi

#endif // INTERNAL_SESSION_INTERFACE_H
