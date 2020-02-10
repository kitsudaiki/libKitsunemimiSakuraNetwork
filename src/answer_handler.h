/**
 * @file       answer_handler.h
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

#ifndef ANSWER_HANDLER_H
#define ANSWER_HANDLER_H

#include <vector>
#include <iostream>

#include <libKitsunemimiCommon/threading/thread.h>

namespace Kitsunemimi
{
namespace Project
{
class Session;

class AnswerHandler : public Kitsunemimi::Thread
{
public:
    AnswerHandler();
    ~AnswerHandler();

    // add
    void addMessage(const uint32_t sessionId,
                    const uint64_t messageId);
    void addMessage(const uint64_t completeMessageId);

    // remove
    bool removeMessage(const uint32_t sessionId,
                       const uint64_t messageId);
    bool removeMessage(const uint64_t completeMessageId);

protected:
    void run();

private:
    struct MessageBlocker
    {
        uint64_t completeMessageId = 0;
        float timer = 0;
        std::mutex cvMutex;
        std::condition_variable cv;
    };

    std::vector<MessageBlocker*> m_messageList;

    bool removeMessageFromList(const uint64_t completeMessageId);
    void clearList();
};

} // namespace Project
} // namespace Kitsunemimi

#endif // ANSWER_HANDLER_H
