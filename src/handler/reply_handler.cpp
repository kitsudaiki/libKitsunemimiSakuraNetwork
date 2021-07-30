/**
 * @file       reply_handler.cpp
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

#include <handler/reply_handler.h>
#include <handler/message_blocker_handler.h>
#include <handler/session_handler.h>

#include <libKitsunemimiSakuraNetwork/session.h>

#include <libKitsunemimiNetwork/abstract_socket.h>
#include <libKitsunemimiPersistence/logger/logger.h>

namespace Kitsunemimi
{
namespace Sakura
{

/**
 * @brief constructor
 */
ReplyHandler::ReplyHandler() {}

/**
 * @brief destructor
 */
ReplyHandler::~ReplyHandler()
{
    spinLock();
    m_messageList.clear();
    spinUnlock();
}

/**
 * @brief add a message to the internal timeout-queue
 *
 * @param messageType type of the message
 * @param sessionId is of the session of the message
 * @param messageId id of the message, which should be added
 * @param session pointer to the session-object, which had sended the message
 */
void
ReplyHandler::addMessage(const uint8_t messageType,
                         const uint32_t sessionId,
                         const uint64_t messageId,
                         Session* session)
{
    addMessage(messageType, (messageId << 32) + sessionId, session);
}

/**
 * @brief add a message to the internal timeout-queue
 *
 * @param messageType type of the message
 * @param completeMessageId completed id of the message, which should be added
 * @param session pointer to the session-object, which had sended the message
 */
void
ReplyHandler::addMessage(const uint8_t messageType,
                         const uint64_t completeMessageId,
                         Session* session)
{
    MessageTime messageTime;
    messageTime.completeMessageId = completeMessageId;
    messageTime.messageType = messageType;
    messageTime.session = session;

    spinLock();
    m_messageList.push_back(messageTime);
    spinUnlock();
}

/**
 * @brief remove a message from the internal list
 *
 * @param sessionId is of the session of the message
 * @param messageId id of the message, which should be removed
 *
 * @return false, if message-id doesn't exist in the list, else true
 */
bool
ReplyHandler::removeMessage(const uint32_t sessionId,
                            const uint64_t messageId)
{
    return removeMessage((messageId << 32) + sessionId);
}

/**
 * @brief remove a message from the internal list
 *
 * @param completeMessageId id of the message, which should be removed
 *
 * @return false, if message-id doesn't exist in the list, else true
 */
bool
ReplyHandler::removeMessage(const uint64_t completeMessageId)
{
    bool result = false;

    spinLock();
    result = removeMessageFromList(completeMessageId);
    spinUnlock();

    return result;
}

/**
 * @brief remove all messages from the internal message, which are related to a specific session
 *
 * @param sessionId id of the session
 */
void
ReplyHandler::removeAllOfSession(const uint32_t sessionId)
{
    spinLock();

    std::vector<MessageTime>::iterator it;
    for(it = m_messageList.begin();
        it != m_messageList.end();
        it++)
    {
        if((it->completeMessageId & 0xFFFFFFFF) == sessionId) {
            it->ignoreResult = true;
        }
    }

    spinUnlock();
}

/**
 * @brief remove a message from the internal list
 *
 * @param messageId id of the message, which should be removed
 *
 * @return false, if message-id doesn't exist in the list, else true
 */
bool
ReplyHandler::removeMessageFromList(const uint64_t completeMessageId)
{
    std::vector<MessageTime>::iterator it;
    for(it = m_messageList.begin();
        it != m_messageList.end();
        it++)
    {
        if(it->completeMessageId == completeMessageId)
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
 * @brief endless thread-loop the time timer
 */
void
ReplyHandler::run()
{
    uint32_t counter = 0;

    while(!m_abort)
    {
        sleepThread(100000);
        counter += 1;

        if(m_abort) {
            break;
        }

        makeTimerStep();

        if(counter % 10 == 0)
        {
            SessionHandler::m_sessionHandler->sendHeartBeats();
            counter = 0;
        }
    }
}

/**
 * @brief Increase the timer of all messages and handle timeouts
 */
void
ReplyHandler::makeTimerStep()
{
    spinLock();

    for(uint64_t i = 0; i < m_messageList.size(); i++)
    {
        MessageTime* temp = &m_messageList[i];
        temp->timer += 0.1f;

        if(temp->timer >= m_timeoutValue)
        {
            spinUnlock();
            removeMessage(temp->completeMessageId);
            if(temp->ignoreResult == false)
            {
                const std::string err = "TIMEOUT of message: "
                                        + std::to_string(temp->completeMessageId)
                                        + " with type: "
                                        + std::to_string(temp->messageType);

                temp->session->m_processError(temp->session,
                                              Session::errorCodes::MESSAGE_TIMEOUT,
                                              err);
            }

            spinLock();

            break;
        }
    }

    spinUnlock();
}

} // namespace Sakura
} // namespace Kitsunemimi
