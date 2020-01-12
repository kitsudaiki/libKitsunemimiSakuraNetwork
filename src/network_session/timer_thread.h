/**
 * @file       timer_thread.h
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

#ifndef TIMER_THREAD_H
#define TIMER_THREAD_H

#include <vector>
#include <iostream>

#include <libKitsunemimiCommon/threading/thread.h>

namespace Kitsunemimi
{
namespace Project
{
class Session;

class TimerThread : public Kitsunemimi::Common::Thread
{
public:
    TimerThread();
    ~TimerThread();

    void addMessage(const uint8_t messageType,
                    const uint32_t sessionId,
                    const uint64_t messageId,
                    Session* session);
    void addMessage(const uint8_t messageType,
                    const uint64_t completeMessageId,
                    Session* session);

    bool removeMessage(const uint32_t sessionId,
                       const uint64_t messageId);
    bool removeMessage(const uint64_t completeMessageId);

    void removeAllOfSession(const uint32_t sessionId);

protected:
    void run();

private:
    struct MessageTime
    {
        uint64_t completeMessageId = 0;
        float timer = 0;
        uint8_t messageType = 0;
        Session* session = nullptr;
    };

    float m_timeoutValue = 2.0f;
    std::vector<MessageTime> m_messageList;

    void makeTimerStep();
    bool removeMessageFromList(const uint64_t completeMessageId);
};

} // namespace Project
} // namespace Kitsunemimi

#endif // TIMER_THREAD_H
