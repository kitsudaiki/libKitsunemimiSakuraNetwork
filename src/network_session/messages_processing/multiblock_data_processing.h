/**
 * @file       multiblock_data_processing.h
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

#ifndef MULTIBLOCK_DATA_PROCESSING_H
#define MULTIBLOCK_DATA_PROCESSING_H

#include <network_session/message_definitions.h>
#include <network_session/session_handler.h>
#include <network_session/internal_session_interface.h>

#include <libKitsunemimiNetwork/abstract_socket.h>
#include <libKitsunemimiNetwork/message_ring_buffer.h>

#include <libKitsunemimiProjectCommon/network_session/session_controller.h>
#include <libKitsunemimiProjectCommon/network_session/session.h>

#include <libKitsunemimiPersistence/logger/logger.h>

using Kitsunemimi::Network::MessageRingBuffer;
using Kitsunemimi::Network::AbstractSocket;
using Kitsunemimi::Network::getObjectFromBuffer;
using Kitsunemimi::Network::getDataPointer;

namespace Kitsunemimi
{
namespace Project
{
namespace Common
{

/**
 * @brief send_Data_Multi_Init
 */
inline void
send_Data_Multi_Init(Session* session,
                     const uint64_t multiblockId,
                     const uint64_t requestedSize)
{
    if(DEBUG_MODE) {
        LOG_DEBUG("SEND data multi init");
    }

    Data_MultiInit_Message message(session->sessionId(),
                                   session->increaseMessageIdCounter(),
                                   multiblockId);
    message.totalSize = requestedSize;

    SessionHandler::m_sessionInterface->sendMessage(session,
                                                    message.commonHeader,
                                                    &message,
                                                    sizeof(message));
}

/**
 * @brief send_Data_Multi_Init_Reply
 */
inline void
send_Data_Multi_Init_Reply(Session* session,
                           const uint64_t multiblockId,
                           const uint32_t messageId,
                           const uint8_t status)
{
    if(DEBUG_MODE) {
        LOG_DEBUG("SEND data multi init reply");
    }

    Data_MultiInitReply_Message message(session->sessionId(),
                                        messageId,
                                        multiblockId);
    message.status = status;

    SessionHandler::m_sessionInterface->sendMessage(session,
                                                    message.commonHeader,
                                                    &message,
                                                    sizeof(message));
}

/**
 * @brief send_Data_Multi_Static
 */
inline void
send_Data_Multi_Static(Session* session,
                       const uint64_t multiblockId,
                       const uint32_t totalPartNumber,
                       const uint32_t partId,
                       const void* data,
                       const uint64_t size)
{
    if(DEBUG_MODE) {
        LOG_DEBUG("SEND data multi static");
    }

    Data_MultiStatic_Message message(session->sessionId(),
                                     session->increaseMessageIdCounter(),
                                     multiblockId);

    message.totalPartNumber = totalPartNumber;
    message.partId = partId;

    memcpy(message.payload, data, size);
    message.payloadSize = size;

    SessionHandler::m_sessionInterface->sendMessage(session,
                                                    message.commonHeader,
                                                    &message,
                                                    sizeof(message));
}

/**
 * @brief send_Data_Multi_Finish
 * @param session
 */
inline void
send_Data_Multi_Finish(Session* session,
                       const uint64_t multiblockId)
{
    if(DEBUG_MODE) {
        LOG_DEBUG("SEND data multi finish");
    }

    Data_MultiFinish_Message message(session->sessionId(),
                                     session->increaseMessageIdCounter(),
                                     multiblockId);
    SessionHandler::m_sessionInterface->sendMessage(session,
                                                    message.commonHeader,
                                                    &message,
                                                    sizeof(message));
}

/**
 * @brief send_Data_Multi_Abort
 * @param session
 */
inline void
send_Data_Multi_Abort(Session* session,
                      const uint64_t multiblockId)
{
    if(DEBUG_MODE) {
        LOG_DEBUG("SEND data multi abort");
    }

    Data_MultiAbort_Message message(session->sessionId(),
                                    session->increaseMessageIdCounter(),
                                    multiblockId);
}

/**
 * @brief process_Data_Multi_Init
 */
inline void
process_Data_Multi_Init(Session* session,
                        const Data_MultiInit_Message* message)
{
    if(DEBUG_MODE) {
        LOG_DEBUG("process data multi init");
    }

    const bool ret = SessionHandler::m_sessionInterface->initMultiblockBuffer(session,
                                                                              message->multiblockId,
                                                                              message->totalSize);
    if(ret)
    {
        send_Data_Multi_Init_Reply(session,
                                   message->multiblockId,
                                   message->commonHeader.messageId,
                                   Data_MultiInitReply_Message::OK);
    }
    else
    {
        send_Data_Multi_Init_Reply(session,
                                   message->multiblockId,
                                   message->commonHeader.messageId,
                                   Data_MultiInitReply_Message::FAIL);
    }
}

/**
 * @brief process_Data_Multi_Init_Reply
 */
inline void
process_Data_Multi_Init_Reply(Session* session,
                              const Data_MultiInitReply_Message* message)
{
    if(DEBUG_MODE) {
        LOG_DEBUG("process data multi init reply");
    }

    if(message->status == Data_MultiInitReply_Message::OK)
    {
        // counter values
        uint64_t totalSize =
                SessionHandler::m_sessionInterface->getUsedBufferSize(session,
                                                                      message->multiblockId);
        uint64_t currentMessageSize = 0;
        uint32_t partCounter = 0;

        // static values
        const uint32_t totalPartNumber = static_cast<uint32_t>(totalSize / 1000) + 1;
        const uint8_t* dataPointer =
                SessionHandler::m_sessionInterface->getDataPointer(session, message->multiblockId);

        while(totalSize != 0)
        {
            // get message-size base on the rest
            currentMessageSize = 1000;
            if(totalSize < 1000) {
                currentMessageSize = totalSize;
            }
            totalSize -= currentMessageSize;

            // send single packet
            send_Data_Multi_Static(session,
                                   message->multiblockId,
                                   totalPartNumber,
                                   partCounter,
                                   dataPointer + (1000 * partCounter),
                                   currentMessageSize);

            partCounter++;
        }

        // finish multi-block
        send_Data_Multi_Finish(session, message->multiblockId);
    }
    else
    {
        SessionHandler::m_sessionInterface->receivedError(session,
                                                          Session::errorCodes::MULTIBLOCK_FAILED,
                                                          "unable not send multi-block-Message");
    }

    SessionHandler::m_sessionInterface->finishMultiblockBuffer(session, message->multiblockId);
}

/**
 * @brief process_Data_Multi_Static
 */
inline void
process_Data_Multi_Static(Session* session,
                          const Data_MultiStatic_Message* message)
{
    if(DEBUG_MODE) {
        LOG_DEBUG("process data multi static");
    }

    SessionHandler::m_sessionInterface->writeDataIntoBuffer(session,
                                                            message->multiblockId,
                                                            message->payload,
                                                            message->payloadSize);
}

/**
 * @brief process_Data_Multi_Finish
 */
inline void
process_Data_Multi_Finish(Session* session,
                          const Data_MultiFinish_Message* message)
{
    if(DEBUG_MODE) {
        LOG_DEBUG("process data multi finish");
    }

    const uint64_t totalSize =
            SessionHandler::m_sessionInterface->getUsedBufferSize(session, message->multiblockId);
    const uint8_t* dataPointer =
            SessionHandler::m_sessionInterface->getDataPointer(session, message->multiblockId);

    SessionHandler::m_sessionInterface->receivedData(session,
                                                     false,
                                                     dataPointer,
                                                     totalSize);
    SessionHandler::m_sessionInterface->finishMultiblockBuffer(session, message->multiblockId);
}

/**
 * @brief process_Data_Multi_Abort
 */
inline void
process_Data_Multi_Abort(Session* session,
                         const Data_MultiAbort_Message* message)
{
    if(DEBUG_MODE) {
        LOG_DEBUG("process data multi abort");
    }

    SessionHandler::m_sessionInterface->finishMultiblockBuffer(session, message->multiblockId);
}

/**
 * @brief process messages of multiblock-message-type
 *
 * @param session pointer to the session
 * @param header pointer to the common header of the message within the message-ring-buffer
 * @param recvBuffer pointer to the message-ring-buffer
 *
 * @return number of processed bytes
 */
inline uint64_t
process_MultiBlock_Data_Type(Session* session,
                             const CommonMessageHeader* header,
                             MessageRingBuffer* recvBuffer)
{
    if(DEBUG_MODE) {
        LOG_DEBUG("process data-type");
    }

    switch(header->subType)
    {
        //------------------------------------------------------------------------------------------
        case DATA_MULTI_INIT_SUBTYPE:
            {
                const Data_MultiInit_Message* message =
                        getObjectFromBuffer<Data_MultiInit_Message>(recvBuffer);
                if(message == nullptr) {
                    break;
                }
                process_Data_Multi_Init(session, message);
                return sizeof(*message);
            }
        //------------------------------------------------------------------------------------------
        case DATA_MULTI_INIT_REPLY_SUBTYPE:
            {
                const Data_MultiInitReply_Message* message =
                        getObjectFromBuffer<Data_MultiInitReply_Message>(recvBuffer);
                if(message == nullptr) {
                    break;
                }
                process_Data_Multi_Init_Reply(session, message);
                return sizeof(*message);
            }
        //------------------------------------------------------------------------------------------
        case DATA_MULTI_STATIC_SUBTYPE:
            {
                const Data_MultiStatic_Message* message =
                        getObjectFromBuffer<Data_MultiStatic_Message>(recvBuffer);
                if(message == nullptr) {
                    break;
                }
                process_Data_Multi_Static(session, message);
                return sizeof(*message);
            }
        //------------------------------------------------------------------------------------------
        case DATA_MULTI_FINISH_SUBTYPE:
            {
                const Data_MultiFinish_Message* message =
                        getObjectFromBuffer<Data_MultiFinish_Message>(recvBuffer);
                if(message == nullptr) {
                    break;
                }
                process_Data_Multi_Finish(session, message);
                return sizeof(*message);
            }
        //------------------------------------------------------------------------------------------
        case DATA_MULTI_ABORT_SUBTYPE:
            {
                const Data_MultiAbort_Message* message =
                        getObjectFromBuffer<Data_MultiAbort_Message>(recvBuffer);
                if(message == nullptr) {
                    break;
                }
                process_Data_Multi_Abort(session, message);
                return sizeof(*message);
            }
        //------------------------------------------------------------------------------------------
        default:
            break;
    }

    return 0;
}

} // namespace Common
} // namespace Project
} // namespace Kitsunemimi

#endif // MULTIBLOCK_DATA_PROCESSING_H
