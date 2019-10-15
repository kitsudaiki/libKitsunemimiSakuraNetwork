/**
 *  @file       timer_thread.cpp
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

#include <network_session/timer_thread.h>
#include <libKitsuneNetwork/abstract_socket.h>
#include <libKitsuneProjectCommon/network_session/session.h>
#include <libKitsunePersistence/logger/logger.h>

namespace Kitsune
{
namespace Project
{
namespace Common
{


/**
 * constructor
 */
TimerThread::TimerThread()
{

}

/**
 * @brief TimerThread::addMessage
 * @param messageId
 * @param sessionId
 */
void
TimerThread::addMessage(const uint8_t messageType,
                        const uint32_t sessionId,
                        const uint64_t messageId)
{
    addMessage(messageType, (messageId << 32) + sessionId);
}

/**
 * @brief TimerThread::addMessage
 * @param messageId
 */
void
TimerThread::addMessage(const uint8_t messageType,
                        const uint64_t messageId)
{
    MessageTime messageTime;
    messageTime.messageId = messageId;
    messageTime.messageType = messageType;

    mutexLock();
    m_messageList.push_back(messageTime);
    mutexUnlock();
}

/**
 * @brief TimerThread::removeMessage
 * @param messageId
 * @param sessionId
 * @return
 */
bool
TimerThread::removeMessage(const uint32_t sessionId,
                           const uint64_t messageId)
{
    return removeMessage((messageId << 32) + sessionId);
}

/**
 * @brief TimerThread::removeMessage
 * @param messageId
 * @return
 */
bool
TimerThread::removeMessage(const uint64_t messageId)
{
    bool result = false;

    mutexLock();
    result = removeMessageFromList(messageId);
    mutexUnlock();

    return result;
}

/**
 * @brief TimerThread::removeAllOfSession
 * @param sessionId
 */
void
TimerThread::removeAllOfSession(const uint32_t sessionId)
{
    mutexLock();

    std::vector<MessageTime>::iterator it;
    for(it = m_messageList.begin(); it != m_messageList.end(); it++)
    {
        if((it->messageId & 0xFFFFFFFF) == sessionId)
        {
            m_messageList.erase(it);
            continue;
        }
    }

    mutexUnlock();
}

/**
 * @brief TimerThread::removeMessage
 * @param messageId
 * @return
 */
bool
TimerThread::removeMessageFromList(const uint64_t messageId)
{
    std::vector<MessageTime>::iterator it;
    for(it = m_messageList.begin(); it != m_messageList.end(); it++)
    {
        if(it->messageId == messageId)
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

            return true;
        }
    }

    return false;
}

/**
 * @brief TimerThread::run
 */
void
TimerThread::run()
{
    while(!m_abort)
    {
        sleepThread(100000);

        if(m_abort) {
            break;
        }

        mutexLock();

        for(uint64_t i = 0; i < m_messageList.size(); i++)
        {
            MessageTime* temp = &m_messageList[i];
            temp->timer += 0.1f;
            //it.base()->timer += 0.1f;
            if(temp->timer >= m_timeoutValue)
            {
                if(removeMessageFromList(temp->messageId))
                {
                    // TODO: send error
                    LOG_ERROR("TIMEOUT of message: " + std::to_string(temp->messageId)
                              + " with type: " + std::to_string(temp->messageType));
                    i--;
                }
            }
        }
        mutexUnlock();
    }
}

} // namespace Common
} // namespace Project
} // namespace Kitsune
