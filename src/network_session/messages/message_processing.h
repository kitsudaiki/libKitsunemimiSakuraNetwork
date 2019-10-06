/**
 *  @file       message_processing.h
 *
 *  @author     Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 *  @copyright  Apache License Version 2.0
 */

#ifndef MESSAGE_PROCESSING_H
#define MESSAGE_PROCESSING_H

#include <message_ring_buffer.h>
#include <network_session/session_handler.h>
#include <network_session/messages/message_definitions.h>
#include <abstract_socket.h>

#include <network_session/messages/session_init_processing.h>
#include <network_session/messages/session_end_processing.h>
#include <logger/logger.h>

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
    Persistence::LOG_debug("process session-init");
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
        case SESSION_ID_CONFIRM_SUBTYPE:
            {
                const Session_IdConfirm_Message* message =
                        getMessageFromBuffer<Session_IdConfirm_Message>(recvBuffer);
                if(message == nullptr) {
                    break;
                }
                processSessionIdConfirm(message, socket);
                return sizeof(Session_IdConfirm_Message);
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
    Persistence::LOG_debug("process message");

     const CommonMessageHeader* header = getMessageFromBuffer<CommonMessageHeader>(recvBuffer);

    if(header == nullptr
            || header->version != 0x1)
    {
        // TODO: error if false version
        Persistence::LOG_debug("message-buffer not bug enough");
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
