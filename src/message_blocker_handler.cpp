/**
 * @file       message_blocker_handler.cpp
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

#include "message_blocker_handler.h"
#include <libKitsunemimiProjectNetwork/session.h>

namespace Kitsunemimi
{
namespace Project
{

MessageBlockerHandler::MessageBlockerHandler()
{

}

MessageBlockerHandler::~MessageBlockerHandler()
{
    clearList();
}

/**
 * @brief AnswerHandler::addMessage
 * @param completeMessageId
 */
DataBuffer*
MessageBlockerHandler::blockMessage(const uint64_t completeMessageId,
                                    const uint64_t blockerTimeout,
                                    Session* session)
{
    MessageBlocker* messageBlocker = new MessageBlocker();
    messageBlocker->completeMessageId = completeMessageId;
    messageBlocker->timer = blockerTimeout;
    messageBlocker->session = session;

    // add to waiting-list
    spinLock();
    m_messageList.push_back(messageBlocker);
    spinUnlock();

    // release thread
    std::unique_lock<std::mutex> lock(messageBlocker->cvMutex);
    messageBlocker->cv.wait(lock);

    // remove from list and return result
    spinLock();
    DataBuffer* result = removeMessageFromList(completeMessageId);
    spinUnlock();

    return result;
}

/**
 * @brief AnswerHandler::removeMessage
 * @param completeMessageId
 * @return
 */
bool
MessageBlockerHandler::releaseMessage(const uint64_t completeMessageId,
                                      DataBuffer* data)
{
    bool result = false;

    spinLock();
    result = releaseMessageInList(completeMessageId, data);
    spinUnlock();

    return result;
}

/**
 * @brief AnswerHandler::run
 */
void
MessageBlockerHandler::run()
{
    while(!m_abort)
    {
        sleepThread(1000000);

        if(m_abort) {
            break;
        }

        makeTimerStep();
    }
}

/**
 * @brief MessageBlockerHandler::releaseMessageInList
 * @param completeMessageId
 * @param data
 * @param dataSize
 * @return
 */
bool
MessageBlockerHandler::releaseMessageInList(const uint64_t completeMessageId,
                                            DataBuffer* data)
{
    std::vector<MessageBlocker*>::iterator it;
    for(it = m_messageList.begin();
        it != m_messageList.end();
        it++)
    {
        MessageBlocker* tempItem = *it;
        if(tempItem->completeMessageId == completeMessageId)
        {
            tempItem->responseData = data;
            tempItem->cv.notify_one();
            return true;
        }
    }

    return false;
}

/**
 * @brief AnswerHandler::removeMessageFromList
 * @param completeMessageId
 * @return
 */
DataBuffer*
MessageBlockerHandler::removeMessageFromList(const uint64_t completeMessageId)
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

            DataBuffer* result = tempItem->responseData;

            tempItem->responseData = nullptr;
            delete tempItem;

            return result;
        }
    }

    return nullptr;
}

/**
 * @brief AnswerHandler::clearList
 */
void
MessageBlockerHandler::clearList()
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

/**
 * @brief MessageBlockerHandler::makeTimerStep
 */
void
MessageBlockerHandler::makeTimerStep()
{
    spinLock();

    for(uint64_t i = 0; i < m_messageList.size(); i++)
    {
        MessageBlocker* temp = m_messageList[i];
        temp->timer -= 1;

        if(temp->timer == 0)
        {
            removeMessageFromList(temp->completeMessageId);
            releaseMessageInList(temp->completeMessageId, nullptr);

            const std::string err = "TIMEOUT of request: "
                                    + std::to_string(temp->completeMessageId);

            temp->session->m_processError(temp->session->m_errorTarget,
                                          temp->session,
                                          Session::errorCodes::MESSAGE_TIMEOUT,
                                          err);
        }
    }

    spinUnlock();
}

} // namespace Project
} // namespace Kitsunemimi
