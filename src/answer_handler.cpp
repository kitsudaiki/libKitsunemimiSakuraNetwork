/**
 * @file       answer_handler.cpp
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

#include "answer_handler.h"

namespace Kitsunemimi
{
namespace Project
{

AnswerHandler::AnswerHandler()
{

}

AnswerHandler::~AnswerHandler()
{
    clearList();
}

/**
 * @brief AnswerHandler::addMessage
 * @param sessionId
 * @param messageId
 */
void
AnswerHandler::addMessage(const uint32_t sessionId,
                          const uint64_t messageId)
{
    addMessage((messageId << 32) + sessionId);
}

/**
 * @brief AnswerHandler::addMessage
 * @param completeMessageId
 */
void
AnswerHandler::addMessage(const uint64_t completeMessageId)
{
    MessageBlocker* messageBlocker = new MessageBlocker();
    messageBlocker->completeMessageId = completeMessageId;

    spinLock();
    m_messageList.push_back(messageBlocker);
    spinUnlock();

    std::unique_lock<std::mutex> lock(messageBlocker->cvMutex);
    messageBlocker->cv.wait(lock);
}

/**
 * @brief AnswerHandler::removeMessage
 * @param completeMessageId
 * @return
 */
bool
AnswerHandler::removeMessage(const uint64_t completeMessageId)
{
    bool result = false;

    spinLock();
    result = removeMessageFromList(completeMessageId);
    spinUnlock();

    return result;
}

/**
 * @brief AnswerHandler::run
 */
void
AnswerHandler::run()
{

}

/**
 * @brief AnswerHandler::removeMessageFromList
 * @param completeMessageId
 * @return
 */
bool
AnswerHandler::removeMessageFromList(const uint64_t completeMessageId)
{
    std::vector<MessageBlocker*>::iterator it;
    for(it = m_messageList.begin();
        it != m_messageList.end();
        it++)
    {
        MessageBlocker* tempItem = *it;
        if(tempItem->completeMessageId == completeMessageId)
        {
            if(m_messageList.size() > 1)
            {
                // swap with last and remove the last instead of erase the element direct
                // because this was is faster
                std::iter_swap(it, m_messageList.end() - 1);
                m_messageList.pop_back();
            }
            else
            {
                m_messageList.clear();
            }

            // release thread and free memory
            tempItem->cv.notify_one();
            delete tempItem;

            return true;
        }
    }

    return false;
}

/**
 * @brief AnswerHandler::clearList
 */
void
AnswerHandler::clearList()
{
    spinLock();

    // release all threads
    std::vector<MessageBlocker*>::iterator it;
    for(it = m_messageList.begin();
        it != m_messageList.end();
        it++)
    {
        MessageBlocker* tempItem = *it;
        tempItem->cv.notify_one();
        delete tempItem;
    }

    // clear list
    m_messageList.clear();

    spinUnlock();
}

} // namespace Project
} // namespace Kitsunemimi
