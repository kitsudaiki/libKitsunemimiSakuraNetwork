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

#ifndef KITSUNEMIMI_SAKURA_NETWORK_MULTIBLOCK_DATA_PROCESSING_H
#define KITSUNEMIMI_SAKURA_NETWORK_MULTIBLOCK_DATA_PROCESSING_H

#include <message_definitions.h>
#include <handler/session_handler.h>
#include <multiblock_io.h>

#include <libKitsunemimiNetwork/abstract_socket.h>
#include <libKitsunemimiCommon/buffer/ring_buffer.h>

#include <libKitsunemimiSakuraNetwork/session_controller.h>
#include <libKitsunemimiSakuraNetwork/session.h>

#include <libKitsunemimiCommon/logger.h>

using Kitsunemimi::RingBuffer;
using Kitsunemimi::Network::AbstractSocket;

namespace Kitsunemimi
{
namespace Sakura
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
    Data_MultiInit_Message message;

    // fill message
    message.commonHeader.sessionId = session->sessionId();
    message.commonHeader.messageId = session->increaseMessageIdCounter();
    message.multiblockId = multiblockId;
    message.totalSize = requestedSize;
    if(answerExpected) {
        message.commonHeader.flags |= 0x4;
    }

    SessionHandler::m_sessionHandler->sendMessage(session, message);
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
    Data_MultiInitReply_Message message;

    // fill message
    message.commonHeader.sessionId = session->sessionId();
    message.commonHeader.messageId = messageId;
    message.multiblockId = multiblockId;
    message.status = status;

    SessionHandler::m_sessionHandler->sendMessage(session, message);
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
                       const uint32_t size)
{
    uint8_t messageBuffer[MESSAGE_CACHE_SIZE];

    // bring message-size to a multiple of 8
    const uint32_t totalMessageSize = sizeof(Data_MultiBlock_Header)
                                      + size
                                      + (8-(size % 8)) % 8  // fill up to a multiple of 8
                                      + sizeof(CommonMessageFooter);

    CommonMessageFooter end;
    Data_MultiBlock_Header message;

    // fill message
    message.commonHeader.sessionId = session->sessionId();
    message.commonHeader.messageId = session->increaseMessageIdCounter();
    message.commonHeader.totalMessageSize = totalMessageSize;
    message.commonHeader.payloadSize = size;
    message.multiblockId = multiblockId;
    message.totalPartNumber = totalPartNumber;
    message.partId = partId;

    // fill buffer to build the complete message
    memcpy(&messageBuffer[0], &message, sizeof(Data_MultiBlock_Header));
    memcpy(&messageBuffer[sizeof(Data_MultiBlock_Header)], data, size);
    memcpy(&messageBuffer[(totalMessageSize - sizeof(CommonMessageFooter))],
           &end,
           sizeof(CommonMessageFooter));

    SessionHandler::m_sessionHandler->sendMessage(session, message);
}

/**
 * @brief send_Data_Multi_Finish
 */
inline void
send_Data_Multi_Finish(Session* session,
                       const uint64_t multiblockId,
                       const uint64_t blockerId)
{
    Data_MultiFinish_Message message;

    message.commonHeader.sessionId = session->sessionId();
    message.commonHeader.messageId = session->increaseMessageIdCounter();
    message.multiblockId = multiblockId;
    message.blockerId = blockerId;
    if(blockerId != 0) {
        message.commonHeader.flags |= 0x8;
    }

    SessionHandler::m_sessionHandler->sendMessage(session, message);
}

/**
 * @brief send_Data_Multi_Abort_Init
 */
inline void
send_Data_Multi_Abort_Init(Session* session,
                           const uint64_t multiblockId)
{
    Data_MultiAbortInit_Message message;

    message.commonHeader.sessionId = session->sessionId();
    message.commonHeader.messageId = session->increaseMessageIdCounter();
    message.multiblockId = multiblockId;

    SessionHandler::m_sessionHandler->sendMessage(session, message);
}

/**
 * @brief send_Data_Multi_Abort_Reply
 */
inline void
send_Data_Multi_Abort_Reply(Session* session,
                            const uint64_t multiblockId,
                            const uint32_t messageId)
{
    Data_MultiAbortReply_Message message;

    message.commonHeader.sessionId = session->sessionId();
    message.commonHeader.messageId = messageId;
    message.multiblockId = multiblockId;

    SessionHandler::m_sessionHandler->sendMessage(session, message);
}

/**
 * @brief process_Data_Multi_Init
 */
inline void
process_Data_Multi_Init(Session* session,
                        const Data_MultiInit_Message* message)
{
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
    if(message->status == Data_MultiInitReply_Message::OK)
    {
        session->m_multiblockIo->makeOutgoingReady(message->multiblockId);
    }
    else
    {
        // trigger callback
        session->m_processError(session,
                                Session::errorCodes::MULTIBLOCK_FAILED,
                                "unable not send multi-block-Message");
    }
}

/**
 * @brief process_Data_Multi_Static
 */
inline void
process_Data_Multiblock(Session* session,
                        const Data_MultiBlock_Header* message,
                        const void* rawMessage)
{
    const uint8_t* payloadData = static_cast<const uint8_t*>(rawMessage)
                                 + sizeof(Data_SingleBlock_Header);
    session->m_multiblockIo->writeIntoIncomingBuffer(message->multiblockId,
                                                     payloadData,
                                                     message->commonHeader.payloadSize);
}

/**
 * @brief process_Data_Multi_Finish
 */
inline void
process_Data_Multi_Finish(Session* session,
                          const Data_MultiFinish_Message* message)
{
    MultiblockIO::MultiblockMessage buffer =
            session->m_multiblockIo->getIncomingBuffer(message->multiblockId);

    // check if normal standalone-message or if message is response
    if(message->commonHeader.flags & 0x8)
    {
        // release thread, which is related to the blocker-id
        SessionHandler::m_blockerHandler->releaseMessage(message->blockerId,
                                                         buffer.multiBlockBuffer);
    }
    else
    {
        // trigger callback
        session->m_processStandaloneData(session->m_standaloneReceiver,
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
    session->m_multiblockIo->removeOutgoingMessage(message->multiblockId);

    // send reply
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
    session->m_multiblockIo->removeIncomingMessage(message->multiblockId);
}

/**
 * @brief process messages of multiblock-message-type
 *
 * @param session pointer to the session
 * @param header pointer to the common header of the message within the message-ring-buffer
 * @param rawMessage pointer to the raw data of the complete message (header + payload + end)
 */
inline void
process_MultiBlock_Data_Type(Session* session,
                             const CommonMessageHeader* header,
                             const void* rawMessage)
{
    switch(header->subType)
    {
        //------------------------------------------------------------------------------------------
        case DATA_MULTI_INIT_SUBTYPE:
            {
                const Data_MultiInit_Message* message =
                    static_cast<const Data_MultiInit_Message*>(rawMessage);
                process_Data_Multi_Init(session, message);
                break;
            }
        //------------------------------------------------------------------------------------------
        case DATA_MULTI_INIT_REPLY_SUBTYPE:
            {
                const Data_MultiInitReply_Message* message =
                    static_cast<const Data_MultiInitReply_Message*>(rawMessage);
                process_Data_Multi_Init_Reply(session, message);
                break;
            }
        //------------------------------------------------------------------------------------------
        case DATA_MULTI_STATIC_SUBTYPE:
            {
                const Data_MultiBlock_Header* message =
                    static_cast<const Data_MultiBlock_Header*>(rawMessage);
                process_Data_Multiblock(session, message, rawMessage);
                break;
            }
        //------------------------------------------------------------------------------------------
        case DATA_MULTI_FINISH_SUBTYPE:
            {
                const Data_MultiFinish_Message* message =
                    static_cast<const Data_MultiFinish_Message*>(rawMessage);
                process_Data_Multi_Finish(session, message);
                break;
            }
        //------------------------------------------------------------------------------------------
        case DATA_MULTI_ABORT_INIT_SUBTYPE:
            {
                const Data_MultiAbortInit_Message* message =
                    static_cast<const Data_MultiAbortInit_Message*>(rawMessage);
                process_Data_Multi_Abort_Init(session, message);
                break;
            }
        //------------------------------------------------------------------------------------------
        case DATA_MULTI_ABORT_REPLY_SUBTYPE:
            {
                const Data_MultiAbortReply_Message* message =
                    static_cast<const Data_MultiAbortReply_Message*>(rawMessage);
                process_Data_Multi_Abort_Reply(session, message);
                break;
            }
        //------------------------------------------------------------------------------------------
        default:
            break;
    }
}

} // namespace Sakura
} // namespace Kitsunemimi

#endif // KITSUNEMIMI_SAKURA_NETWORK_MULTIBLOCK_DATA_PROCESSING_H
