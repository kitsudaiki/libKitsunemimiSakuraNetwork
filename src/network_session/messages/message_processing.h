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
#include <network_session/messages/session_close_processing.h>

#include <libKitsunePersistence/logger/logger.h>

using Kitsune::Network::MessageRingBuffer;
using Kitsune::Network::AbstractSocket;

namespace Kitsune
{
namespace Project
{
namespace Common
{

/**
 * @brief getMessageFromBuffer
 * @param recvBuffer
 * @return
 */
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
 * @brief process_Session_Init
 * @param recvBuffer
 * @param socket
 * @return
 */
inline uint64_t
process_Session_Init(const CommonMessageHeader* header,
                   MessageRingBuffer *recvBuffer,
                   AbstractSocket* socket)
{
    LOG_DEBUG("process session-init");
    switch(header->subType)
    {
        case SESSION_INIT_START_SUBTYPE:
            {
                const Session_Init_Start_Message*  message =
                        getMessageFromBuffer<Session_Init_Start_Message>(recvBuffer);
                if(message == nullptr) {
                    break;
                }
                process_Session_Init_Start(message, socket);
                return sizeof(Session_Init_Start_Message);
            }
        case SESSION_INIT_ID_CHANGE_SUBTYPE:
            {
                const Session_Init_IdChange_Message* message =
                        getMessageFromBuffer<Session_Init_IdChange_Message>(recvBuffer);
                if(message == nullptr) {
                    break;
                }
                process_Session_Init_IdChange(message, socket);
                return sizeof(Session_Init_IdChange_Message);
            }
        case SESSION_INIT_REPLY_SUBTYPE:
            {
                const Session_Init_Reply_Message*  message =
                        getMessageFromBuffer<Session_Init_Reply_Message>(recvBuffer);
                if(message == nullptr) {
                    break;
                }
                process_Session_Init_Reply(message, socket);
                return sizeof(Session_Init_Reply_Message);
            }
        default:
            break;
    }

    return 0;
}

/**
 * @brief process_SessionEnd
 * @param header
 * @param recvBuffer
 * @param socket
 * @return
 */
inline uint64_t
process_SessionEnd(const CommonMessageHeader* header,
                   MessageRingBuffer *recvBuffer,
                   AbstractSocket* socket)
{
    LOG_DEBUG("process session-end");
    switch(header->subType)
    {
        case SESSION_CLOSE_START_SUBTYPE:
            {
                const Session_Close_Start_Message*  message =
                        getMessageFromBuffer<Session_Close_Start_Message>(recvBuffer);
                if(message == nullptr) {
                    break;
                }
                process_Session_Close_Start(message, socket);
                return sizeof(Session_Close_Start_Message);
            }
        case SESSION_CLOSE_REPLY_SUBTYPE:
            {
                const Session_Close_Reply_Message*  message =
                        getMessageFromBuffer<Session_Close_Reply_Message>(recvBuffer);
                if(message == nullptr) {
                    break;
                }
                process_Session_Close_Reply(message, socket);
                return sizeof(Session_Close_Reply_Message);
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
            return process_Session_Init(header, recvBuffer, socket);
        default:
            break;
    }

    return 0;
}

} // namespace Common
} // namespace Project
} // namespace Kitsune

#endif // MESSAGE_PROCESSING_H
