/**
 *  @file       data_processing.h
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

#ifndef DATA_PROCESSING_H
#define DATA_PROCESSING_H

#include <network_session/message_definitions.h>
#include <network_session/session_handler.h>
#include <network_session/internal_session_interface.h>

#include <libKitsuneNetwork/abstract_socket.h>
#include <libKitsuneNetwork/message_ring_buffer.h>

#include <libKitsuneProjectCommon/network_session/session_controller.h>
#include <libKitsuneProjectCommon/network_session/session.h>

#include <libKitsunePersistence/logger/logger.h>

using Kitsune::Network::MessageRingBuffer;
using Kitsune::Network::AbstractSocket;
using Kitsune::Network::getObjectFromBuffer;
using Kitsune::Network::getDataPointer;

namespace Kitsune
{
namespace Project
{
namespace Common
{

/**
 * @brief send_Data_Single_Static
 */
inline void
send_Data_Single_Static(Session* session,
                        const void* data,
                        uint64_t size)
{
    LOG_DEBUG("SEND data single static");

    Data_SingleStatic_Message message(session->sessionId(),
                                      SessionHandler::m_sessionHandler->increaseMessageIdCounter());

    memcpy(message.payload, data, size);
    message.payloadSize = size;

    session->socket()->sendMessage(&message, sizeof(message));
}

/**
 * @brief send_Data_Single_Dynamic
 */
inline void
send_Data_Single_Dynamic(Session* session,
                         const void* data,
                         const uint64_t size)
{
    LOG_DEBUG("SEND data single dynamic");

    const uint64_t totalMessageSize = sizeof(Data_SingleDynamic_Message)
                                    + size
                                    + sizeof(CommonMessageEnd);

    uint8_t completeMessage[totalMessageSize];
    Data_SingleDynamic_Message header(session->sessionId(),
                                      SessionHandler::m_sessionHandler->increaseMessageIdCounter());
    header.payloadSize = size;
    CommonMessageEnd end;

    memcpy(&completeMessage[0], &header, sizeof(Data_SingleDynamic_Message));
    memcpy(&completeMessage[sizeof(Data_SingleDynamic_Message)], data, size);
    memcpy(&completeMessage[sizeof(Data_SingleDynamic_Message) + size],
           &end,
           sizeof(CommonMessageEnd));

    session->socket()->sendMessage(completeMessage, totalMessageSize);
}

/**
 * @brief send_Data_Single_Reply
 */
inline void
send_Data_Single_Reply(Session* session,
                       const uint32_t messageId)
{
    LOG_DEBUG("SEND data single reply");

    Data_SingleReply_Message message(session->sessionId(), messageId);
    session->socket()->sendMessage(&message, sizeof(message));
}

/**
 * @brief send_Data_Multi_Init
 */
inline void
send_Data_Multi_Init(Session* session,
                     const uint64_t requestedSize)
{
    LOG_DEBUG("SEND data multi init");

    Data_MultiInit_Message message(session->sessionId(),
                                   SessionHandler::m_sessionHandler->increaseMessageIdCounter());
    message.totalSize = requestedSize;

    session->socket()->sendMessage(&message, sizeof(message));
}

/**
 * @brief send_Data_Multi_Init_Reply
 */
inline void
send_Data_Multi_Init_Reply(Session* session,
                           const uint32_t messageId,
                           const uint8_t status)
{
    LOG_DEBUG("SEND data multi init reply");

    Data_MultiInitReply_Message message(session->sessionId(), messageId);
    message.status = status;

    session->socket()->sendMessage(&message, sizeof(message));
}

/**
 * @brief send_Data_Multi_Static
 */
inline void
send_Data_Multi_Static(Session* session,
                       const uint32_t totalPartNumber,
                       const uint32_t partId,
                       const void* data,
                       const uint64_t size)
{
    LOG_DEBUG("SEND data multi static");

    Data_MultiStatic_Message message(session->sessionId(),
                                     SessionHandler::m_sessionHandler->increaseMessageIdCounter());

    message.totalPartNumber = totalPartNumber;
    message.partId = partId;

    memcpy(message.payload, data, size);
    message.payloadSize = size;

    session->socket()->sendMessage(&message, sizeof(message));
}

/**
 * @brief send_Data_Multi_Finish
 * @param session
 */
inline void
send_Data_Multi_Finish(Session* session)
{
    LOG_DEBUG("SEND data multi finish");

    Data_MultiFinish_Message message(session->sessionId(),
                                     SessionHandler::m_sessionHandler->increaseMessageIdCounter());
    session->socket()->sendMessage(&message, sizeof(message));
    std::cout<<"poi"<<std::endl;
}

/**
 * @brief process_Data_Single_Static
 */
inline void
process_Data_Single_Static(Session* session,
                           const Data_SingleStatic_Message* message)
{
    LOG_DEBUG("process data single static");

    SessionHandler::m_sessionInterface->receivedData(session,
                                                     true,
                                                     static_cast<const void*>(message->payload),
                                                     message->payloadSize);

    send_Data_Single_Reply(session,
                           message->commonHeader.messageId);
}

/**
 * @brief process_Data_Single_Dynamic
 */
inline uint64_t
process_Data_Single_Dynamic(Session* session,
                            const Data_SingleDynamic_Message* message,
                            MessageRingBuffer* recvBuffer)
{
    LOG_DEBUG("process data single dynamic");

    const uint64_t totalMessageSize = sizeof(Data_SingleDynamic_Message)
                                    + message->payloadSize
                                    + sizeof(CommonMessageEnd);

    const uint8_t* completeMessage = getDataPointer(*recvBuffer, totalMessageSize);
    if(completeMessage == nullptr) {
        return 0;
    }

    const void* payload = completeMessage + sizeof(Data_SingleDynamic_Message);
    SessionHandler::m_sessionInterface->receivedData(session,
                                                     true,
                                                     payload,
                                                     message->payloadSize);

    send_Data_Single_Reply(session,
                           message->commonHeader.messageId);

    return totalMessageSize;
}

/**
 * @brief process_Data_Single_Reply
 */
inline void
process_Data_Single_Reply(Session*,
                          const Data_SingleReply_Message*)
{
    LOG_DEBUG("process data single reply");
}

/**
 * @brief process_Data_Multi_Init
 */
inline void
process_Data_Multi_Init(Session* session,
                        const Data_MultiInit_Message* message)
{
    LOG_DEBUG("process data multi init");
    const bool ret = SessionHandler::m_sessionInterface->initMultiblockBuffer(session,
                                                                              message->totalSize);
    if(ret)
    {
        send_Data_Multi_Init_Reply(session,
                                   message->commonHeader.messageId,
                                   Data_MultiInitReply_Message::OK);
    }
    else
    {
        send_Data_Multi_Init_Reply(session,
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
    LOG_DEBUG("process data multi init reply");

    if(message->status == Data_MultiInitReply_Message::OK)
    {
        uint64_t totalSize = SessionHandler::m_sessionInterface->getTotalBufferSize(session);
        uint32_t partCounter = 0;
        uint32_t totalPartNumber = static_cast<uint32_t>(totalSize / 500) + 1;
        uint8_t* dataPointer = SessionHandler::m_sessionInterface->getDataPointer(session);

        while(partCounter < totalPartNumber)
        {
            if(partCounter == totalPartNumber - 1)
            {
                send_Data_Multi_Static(session,
                                       totalPartNumber,
                                       partCounter,
                                       dataPointer + (500 * partCounter),
                                       totalSize % 500);
            }
            else
            {
                send_Data_Multi_Static(session,
                                       totalPartNumber,
                                       partCounter,
                                       dataPointer + (500 * partCounter),
                                       500);
            }

            partCounter++;
        }

        send_Data_Multi_Finish(session);
    }
    else
    {
        // TODO: error-call
    }
}

/**
 * @brief process_Data_Multi_Static
 */
inline void
process_Data_Multi_Static(Session* session,
                          const Data_MultiStatic_Message* message)
{
    LOG_DEBUG("process data multi static");
    SessionHandler::m_sessionInterface->writeDataIntoBuffer(session,
                                                            message->payload,
                                                            message->payloadSize);
}

/**
 * @brief process_Data_Multi_Finish
 */
inline void
process_Data_Multi_Finish(Session* session,
                          const Data_MultiFinish_Message*)
{
    LOG_DEBUG("process data multi finish");

    const uint64_t totalSize = SessionHandler::m_sessionInterface->getTotalBufferSize(session);
    const uint8_t* dataPointer = SessionHandler::m_sessionInterface->getDataPointer(session);
    SessionHandler::m_sessionInterface->receivedData(session,
                                                     false,
                                                     dataPointer,
                                                     totalSize);
}

/**
 * @brief process_Data_Type
 * @param session
 * @param header
 * @param recvBuffer
 * @param socket
 * @return
 */
inline uint64_t
process_Data_Type(Session* session,
                  const CommonMessageHeader* header,
                  MessageRingBuffer* recvBuffer)
{
    LOG_DEBUG("process data-type");
    switch(header->subType)
    {
        case DATA_SINGLE_STATIC_SUBTYPE:
            {
                const Data_SingleStatic_Message* message =
                        getObjectFromBuffer<Data_SingleStatic_Message>(recvBuffer);
                if(message == nullptr) {
                    break;
                }
                process_Data_Single_Static(session, message);
                return sizeof(*message);
            }
        case DATA_SINGLE_DYNAMIC_SUBTYPE:
            {
                const Data_SingleDynamic_Message* messageHeader =
                        getObjectFromBuffer<Data_SingleDynamic_Message>(recvBuffer);
                if(messageHeader == nullptr) {
                    break;
                }
                const uint64_t messageSize = process_Data_Single_Dynamic(session,
                                                                         messageHeader,
                                                                         recvBuffer);
                return messageSize;
            }
        case DATA_SINGLE_REPLY_SUBTYPE:
            {
                const Data_SingleReply_Message* message =
                        getObjectFromBuffer<Data_SingleReply_Message>(recvBuffer);
                if(message == nullptr) {
                    break;
                }
                process_Data_Single_Reply(session, message);
                return sizeof(*message);
            }
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
        default:
            break;
    }

    return 0;
}

} // namespace Common
} // namespace Project
} // namespace Kitsune

#endif // DATA_PROCESSING_H
