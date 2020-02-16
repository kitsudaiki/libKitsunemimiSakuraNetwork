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

#include <message_definitions.h>
#include <session_handler.h>
#include <multiblock_io.h>

#include <libKitsunemimiNetwork/abstract_socket.h>
#include <libKitsunemimiNetwork/message_ring_buffer.h>

#include <libKitsunemimiProjectNetwork/session_controller.h>
#include <libKitsunemimiProjectNetwork/session.h>

#include <libKitsunemimiPersistence/logger/logger.h>

using Kitsunemimi::Network::MessageRingBuffer;
using Kitsunemimi::Network::AbstractSocket;
using Kitsunemimi::Network::getObjectFromBuffer;
using Kitsunemimi::Network::getDataPointer;

namespace Kitsunemimi
{
namespace Project
{

/**
 * @brief send_Data_Multi_Init
 */
inline void
send_Data_Multi_Init(Session* session,
                     const uint64_t multiblockId,
                     const uint64_t requestedSize,
                     const bool answerExpected)
{
    if(DEBUG_MODE) {
        LOG_DEBUG("SEND data multi init");
    }

    Data_MultiInit_Message message(session->sessionId(),
                                   session->increaseMessageIdCounter(),
                                   multiblockId,
                                   answerExpected);
    message.totalSize = requestedSize;

    SessionHandler::m_sessionHandler->sendMessage(session,
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

    SessionHandler::m_sessionHandler->sendMessage(session,
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

    SessionHandler::m_sessionHandler->sendMessage(session,
                                                  message.commonHeader,
                                                  &message,
                                                  sizeof(message));
}

/**
 * @brief send_Data_Multi_Finish
 */
inline void
send_Data_Multi_Finish(Session* session,
                       const uint64_t multiblockId,
                       const uint64_t blockerId)
{
    if(DEBUG_MODE) {
        LOG_DEBUG("SEND data multi finish");
    }

    Data_MultiFinish_Message message(session->sessionId(),
                                     session->increaseMessageIdCounter(),
                                     multiblockId,
                                     blockerId);
    SessionHandler::m_sessionHandler->sendMessage(session,
                                                  message.commonHeader,
                                                  &message,
                                                  sizeof(message));
}

/**
 * @brief send_Data_Multi_Abort_Init
 */
inline void
send_Data_Multi_Abort_Init(Session* session,
                           const uint64_t multiblockId)
{
    if(DEBUG_MODE) {
        LOG_DEBUG("SEND data multi abort init");
    }

    Data_MultiAbortInit_Message message(session->sessionId(),
                                        session->increaseMessageIdCounter(),
                                        multiblockId);
    SessionHandler::m_sessionHandler->sendMessage(session,
                                                  message.commonHeader,
                                                  &message,
                                                  sizeof(message));
}

/**
 * @brief send_Data_Multi_Abort_Reply
 */
inline void
send_Data_Multi_Abort_Reply(Session* session,
                            const uint64_t multiblockId,
                            const uint32_t messageId)
{
    if(DEBUG_MODE) {
        LOG_DEBUG("SEND data multi abort reply");
    }

    Data_MultiAbortReply_Message message(session->sessionId(),
                                         messageId,
                                         multiblockId);
    SessionHandler::m_sessionHandler->sendMessage(session,
                                                  message.commonHeader,
                                                  &message,
                                                  sizeof(message));
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

    const bool ret = session->m_multiblockIo->createIncomingBuffer(message->multiblockId,
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
        session->m_multiblockIo->makeOutgoingReady(message->multiblockId);
    }
    else
    {
        session->m_processError(session->m_errorTarget,
                                session,
                                Session::errorCodes::MULTIBLOCK_FAILED,
                                "unable not send multi-block-Message");
    }
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

    session->m_multiblockIo->writeIntoIncomingBuffer(message->multiblockId,
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

    MultiblockIO::MultiblockMessage buffer =
            session->m_multiblockIo->getIncomingBuffer(message->multiblockId);

    // remove from answer-handler
    if(message->commonHeader.flags & 0x8)
    {
        bool found = SessionHandler::m_blockerHandler->releaseMessage(message->blockerId,
                                                                      buffer.multiBlockBuffer);
        assert(found);
    }
    else
    {
        session->m_processStandaloneData(session->m_standaloneDataTarget,
                                         session,
                                         message->multiblockId,
                                         buffer.multiBlockBuffer);
    }

    session->m_multiblockIo->removeIncomingMessage(message->multiblockId);
}

/**
 * @brief process_Data_Multi_Abort
 */
inline void
process_Data_Multi_Abort_Init(Session* session,
                              const Data_MultiAbortInit_Message* message)
{
    if(DEBUG_MODE) {
        LOG_DEBUG("process data multi abort init");
    }

    session->m_multiblockIo->removeOutgoingMessage(message->multiblockId);

    send_Data_Multi_Abort_Reply(session,
                                message->multiblockId,
                                message->commonHeader.messageId);
}

/**
 * @brief process_Data_Multi_Abort
 */
inline void
process_Data_Multi_Abort_Reply(Session* session,
                               const Data_MultiAbortReply_Message* message)
{
    if(DEBUG_MODE) {
        LOG_DEBUG("process data multi abort reply");
    }

    session->m_multiblockIo->removeIncomingMessage(message->multiblockId);
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
        case DATA_MULTI_ABORT_INIT_SUBTYPE:
            {
                const Data_MultiAbortInit_Message* message =
                        getObjectFromBuffer<Data_MultiAbortInit_Message>(recvBuffer);
                if(message == nullptr) {
                    break;
                }
                process_Data_Multi_Abort_Init(session, message);
                return sizeof(*message);
            }
        //------------------------------------------------------------------------------------------
        case DATA_MULTI_ABORT_REPLY_SUBTYPE:
            {
                const Data_MultiAbortReply_Message* message =
                        getObjectFromBuffer<Data_MultiAbortReply_Message>(recvBuffer);
                if(message == nullptr) {
                    break;
                }
                process_Data_Multi_Abort_Reply(session, message);
                return sizeof(*message);
            }
        //------------------------------------------------------------------------------------------
        default:
            break;
    }

    return 0;
}

} // namespace Project
} // namespace Kitsunemimi

#endif // MULTIBLOCK_DATA_PROCESSING_H
