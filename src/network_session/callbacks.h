/**
 *  @file       callbacks.h
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

#ifndef CALLBACKS_H
#define CALLBACKS_H

#include <libKitsuneNetwork/abstract_socket.h>
#include <libKitsuneNetwork/message_ring_buffer.h>
#include <libKitsuneCommon/data_buffer.h>
#include <libKitsuneProjectCommon/network_session/session_controller.h>

#include <network_session/messages_processing/session_processing.h>
#include <network_session/messages_processing/heartbeat_processing.h>
#include <network_session/messages_processing/error_processing.h>

using Kitsune::Network::MessageRingBuffer;
using Kitsune::Network::AbstractSocket;
using Kitsune::Common::DataBuffer;

namespace Kitsune
{
namespace Project
{
namespace Common
{

/**
 * process incoming data
 *
 * @param target void-pointer to the session, which had received the message
 * @param recvBuffer data-buffer with the incoming data
 * @param socket socket, which had received the data
 *
 * @return number of bytes, which were taken from the buffer
 */
uint64_t
processMessage(void* target,
               MessageRingBuffer* recvBuffer,
               AbstractSocket* socket)
{
    LOG_DEBUG("process message");

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

    // remove from timer-thread if message is reply
    if(header->flags == 0x2)
    {
        RessourceHandler::m_timerThread->removeMessage(header->sessionId,
                                                       header->messageId);
    }

    // process message by type
    switch(header->type)
    {
        case SESSION_TYPE:
            return process_Session_Type(session, header, recvBuffer, socket);
        case HEARTBEAT_TYPE:
            return process_Heartbeat_Type(session, header, recvBuffer, socket);
        case ERROR_TYPE:
            return process_Error_Type(session, header, recvBuffer, socket);
        default:
            break;
    }

    return 0;
}

/**
 * processMessageTcp-callback
 */
uint64_t processMessageTcp(void* target,
                           MessageRingBuffer* recvBuffer,
                           AbstractSocket* socket)
{
    return processMessage(target, recvBuffer, socket);
}

/**
 * processConnectionTcp-callback
 */
void processConnectionTcp(void* target,
                          AbstractSocket* socket)
{
    Session* newSession = new Session(socket);

    socket->setMessageCallback(newSession, &processMessageTcp);
    socket->start();
}

/**
 * processMessageTlsTcp-callback
 */
uint64_t processMessageTlsTcp(void* target,
                              MessageRingBuffer* recvBuffer,
                              AbstractSocket* socket)
{
    return processMessage(target, recvBuffer, socket);
}

/**
 * processConnectionTlsTcp-callback
 */
void processConnectionTlsTcp(void* target,
                             AbstractSocket* socket)
{
    socket->setMessageCallback(target, &processMessageTlsTcp);
    socket->start();
}

/**
 * processMessageUnixDomain-callback
 */
uint64_t processMessageUnixDomain(void* target,
                                  MessageRingBuffer* recvBuffer,
                                  AbstractSocket* socket)
{
    return processMessage(target, recvBuffer, socket);
}

/**
 * processConnectionUnixDomain-callback
 */
void processConnectionUnixDomain(void* target,
                                 AbstractSocket* socket)
{
    socket->setMessageCallback(target, &processMessageUnixDomain);
    socket->start();
}

} // namespace Common
} // namespace Project
} // namespace Kitsune

#endif // CALLBACKS_H
