/**
 *  @file       timer_thread.h
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

#ifndef TIMER_THREAD_H
#define TIMER_THREAD_H

#include <vector>
#include <iostream>

#include <libKitsuneCommon/thread.h>

namespace Kitsune
{
namespace Project
{
namespace Common
{
class Session;

class TimerThread : public Kitsune::Common::Thread
{
public:
    TimerThread();

    void addMessage(const uint64_t messageId,
                    Session* session);
    bool removeMessage(const uint64_t messageId);
    void removeAllOfSession(const uint32_t sessionId);

protected:
    void run();

private:
    struct MessageTime
    {
        uint64_t messageId = 0;
        float timer = 0;
        Session* session = nullptr;
    };

    float m_timeoutValue = 0.0f;
    std::vector<MessageTime> m_messageList;

    bool removeMessageFromList(const uint64_t messageId);
};

} // namespace Common
} // namespace Project
} // namespace Kitsune

#endif // TIMER_THREAD_H
