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
 */
void
TimerThread::addMessage(const uint64_t messageId,
                        Session* session)
{
    MessageTime messageTime;
    messageTime.messageId = messageId;
    messageTime.session = session;

    mutexLock();
    m_messageList.push_back(messageTime);
    mutexUnlock();
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
        if(it->session->sessionId == sessionId)
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
        std::vector<MessageTime>::iterator it;
        for(it = m_messageList.begin(); it != m_messageList.end(); it++)
        {
            it->timer += 0.1f;
            if(it->timer >= m_timeoutValue) {
                // TODO: send error
                removeMessageFromList(it->messageId);
            }
        }
        mutexUnlock();
    }
}

} // namespace Common
} // namespace Project
} // namespace Kitsune
