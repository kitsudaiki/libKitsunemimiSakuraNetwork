/**
 * @file       callbacks.h
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

#ifndef CALLBACKS_H
#define CALLBACKS_H

#include <libKitsunemimiNetwork/abstract_socket.h>
#include <libKitsunemimiNetwork/message_ring_buffer.h>

#include <libKitsunemimiCommon/data_buffer.h>

#include <libKitsunemimiProjectNetwork/session_controller.h>

#include <messages_processing/session_processing.h>
#include <messages_processing/heartbeat_processing.h>
#include <messages_processing/error_processing.h>
#include <messages_processing/singleblock_data_processing.h>
#include <messages_processing/multiblock_data_processing.h>

using Kitsunemimi::Network::MessageRingBuffer;
using Kitsunemimi::Network::AbstractSocket;
using Kitsunemimi::DataBuffer;

namespace Kitsunemimi
{
namespace Project
{

/**
 * process incoming data
 *
 * @param target void-pointer to the session, which had received the message
 * @param recvBuffer data-buffer with the incoming data
 *
 * @return number of bytes, which were taken from the buffer
 */
inline uint64_t
processMessage(void* target,
               MessageRingBuffer* recvBuffer)
{
    if(DEBUG_MODE) {
        LOG_DEBUG("process message");
    }

    const CommonMessageHeader* header = getObjectFromBuffer<CommonMessageHeader>(recvBuffer);
    Session* session = static_cast<Session*>(target);

    // precheck
    if(header == nullptr) {
        return 0;
    }

    // check version
    if(header->version != 0x1)
    {
        LOG_ERROR("false message-version");
        send_ErrorMessage(session, Session::errorCodes::FALSE_VERSION, "++++++++++++++++++FAIL");
        return 0;
    }

    // check if there are enough data in the buffer for the complete message
    if(header->size > recvBuffer->readWriteDiff) {
        return 0;
    }

    // remove from reply-handler if message is reply
    if(header->flags & 0x2)
    {
        SessionHandler::m_replyHandler->removeMessage(header->sessionId,
                                                      header->messageId);
    }

    // remove from answer-handler
    if(header->flags & 0x4)
    {
        SessionHandler::m_answerHandler->removeMessage(header->sessionId,
                                                       header->messageId);
    }

    // process message by type
    switch(header->type)
    {
        case SINGLEBLOCK_DATA_TYPE:
            return process_SingleBlock_Data_Type(session, header, recvBuffer);
        case MULTIBLOCK_DATA_TYPE:
            return process_MultiBlock_Data_Type(session, header, recvBuffer);
        case SESSION_TYPE:
            return process_Session_Type(session, header, recvBuffer);
        case HEARTBEAT_TYPE:
            return process_Heartbeat_Type(session, header, recvBuffer);
        case ERROR_TYPE:
            return process_Error_Type(session, header, recvBuffer);
        default:
            break;
    }

    return 0;
}

/**
 * process incoming data
 *
 * @param target void-pointer to the session, which had received the message
 * @param recvBuffer data-buffer with the incoming data
 *
 * @return number of bytes, which were taken from the buffer
 */
uint64_t
processMessage_callback(void* target,
                        MessageRingBuffer* recvBuffer,
                        AbstractSocket*)
{
    return processMessage(target, recvBuffer);
}

/**
 * @brief triggered for a new incoming connection
 *
 * @param socket socket for the new session
 */
void
processConnection_Callback(void*,
                           AbstractSocket* socket)
{
    Session* newSession = new Session(socket);
    socket->setMessageCallback(newSession, &processMessage_callback);
    socket->startThread();
}

} // namespace Project
} // namespace Kitsunemimi

#endif // CALLBACKS_H
