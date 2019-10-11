/**
 *  @file       message_processing.h
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

#ifndef MESSAGE_PROCESSING_H
#define MESSAGE_PROCESSING_H

#include <libKitsuneNetwork/message_ring_buffer.h>
#include <libKitsuneProjectCommon/network_session/session_handler.h>
#include <network_session/messages/message_definitions.h>
#include <libKitsuneNetwork/abstract_socket.h>

#include <network_session/messages/session_init_processing.h>
#include <network_session/messages/session_end_processing.h>

#include <libKitsunePersistence/logger/logger.h>

using Kitsune::Network::MessageRingBuffer;
using Kitsune::Network::AbstractSocket;

namespace Kitsune
{
namespace Project
{
namespace Common
{

template <typename T>
const T*
getMessageFromBuffer(MessageRingBuffer* recvBuffer)
{
    const void* data = static_cast<const void*>(getDataPointer(*recvBuffer, sizeof(T)));

    if(data == nullptr) {
        return nullptr;
    }

    return static_cast<const T*>(data);
}

/**
 * @brief processSessionInit
 * @param recvBuffer
 * @param socket
 * @return
 */
inline uint64_t
processSessionInit(const CommonMessageHeader* header,
                   MessageRingBuffer *recvBuffer,
                   AbstractSocket* socket)
{
    LOG_DEBUG("process session-init");
    switch(header->subType)
    {
        case SESSION_INIT_START_SUBTYPE:
            {
                const Session_InitStart_Message*  message =
                        getMessageFromBuffer<Session_InitStart_Message>(recvBuffer);
                if(message == nullptr) {
                    break;
                }
                processSessionInitStart(message, socket);
                return sizeof(Session_InitStart_Message);
            }
        case SESSION_ID_CHANGE_SUBTYPE:
            {
                const Session_IdChange_Message* message =
                        getMessageFromBuffer<Session_IdChange_Message>(recvBuffer);
                if(message == nullptr) {
                    break;
                }
                processSessionIdChange(message, socket);
                return sizeof(Session_IdChange_Message);
            }
        case SESSION_INIT_REPLY_SUBTYPE:
            {
                const Session_InitReply_Message*  message =
                        getMessageFromBuffer<Session_InitReply_Message>(recvBuffer);
                if(message == nullptr) {
                    break;
                }
                processSessionInitReply(message, socket);
                return sizeof(Session_InitReply_Message);
            }
        default:
            break;
    }

    return 0;
}

/**
 * @brief handleMessage
 * @param target
 * @param recvBuffer
 * @param socket
 * @return
 */
uint64_t
processMessage(void*,
               MessageRingBuffer* recvBuffer,
               AbstractSocket* socket)
{
    LOG_DEBUG("process message");

    const CommonMessageHeader* header = getMessageFromBuffer<CommonMessageHeader>(recvBuffer);

    if(header == nullptr
            || header->version != 0x1)
    {
        // TODO: error if false version
        //LOG_DEBUG("message-buffer not bug enough");
        return 0;
    }

    switch(header->type)
    {
        case SESSION_TYPE:
            return processSessionInit(header, recvBuffer, socket);
        default:
            break;
    }

    return 0;
}

} // namespace Common
} // namespace Project
} // namespace Kitsune

#endif // MESSAGE_PROCESSING_H
