/**
 *  @file       error_processing.h
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

#ifndef ERROR_PROCESSING_H
#define ERROR_PROCESSING_H

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
using Kitsune::Network::AbstractSocket;

namespace Kitsune
{
namespace Project
{
namespace Common
{

/**
 * @brief send_ErrorMessage
 * @param session
 * @param errorCode
 * @param message
 */
inline void
send_ErrorMessage(Session* session,
                  const uint8_t errorCode,
                  const std::string message)
{
    LOG_DEBUG("SEND error message");

    switch(errorCode)
    {
        case Session::errorCodes::FALSE_VERSION:
        {
            Error_FalseVersion_Message errorMessage(
                    session->sessionId(),
                    SessionHandler::m_sessionHandler->increaseMessageIdCounter(),
                    message);
            SessionHandler::m_sessionInterface->sendMessage(session, &message, sizeof(message));
            break;
        }

        case Session::errorCodes::UNKNOWN_SESSION:
        {
            Error_UnknownSession_Message errorMessage(
                    session->sessionId(),
                    SessionHandler::m_sessionHandler->increaseMessageIdCounter(),
                    message);
            SessionHandler::m_sessionInterface->sendMessage(session, &message, sizeof(message));
            break;
        }

        case Session::errorCodes::INVALID_MESSAGE_SIZE:
        {
            Error_InvalidMessage_Message errorMessage(
                    session->sessionId(),
                    SessionHandler::m_sessionHandler->increaseMessageIdCounter(),
                    message);
            SessionHandler::m_sessionInterface->sendMessage(session, &message, sizeof(message));
            break;
        }

        default:
            break;
    }
}

/**
 * @brief process_Error_Type
 * @param session
 * @param header
 * @param recvBuffer
 * @param socket
 * @return
 */
inline uint64_t
process_Error_Type(Session* session,
                   const CommonMessageHeader* header,
                   MessageRingBuffer* recvBuffer,
                   AbstractSocket*)
{
    LOG_DEBUG("process error message");

    switch(header->subType)
    {
        case ERROR_FALSE_VERSION_SUBTYPE:
            {
                const Error_FalseVersion_Message* message =
                        getObjectFromBuffer<Error_FalseVersion_Message>(recvBuffer);
                if(message == nullptr) {
                    break;
                }

                SessionHandler::m_sessionInterface->receivedError(
                            session,
                            Session::errorCodes::FALSE_VERSION,
                            std::string(message->message));
                return sizeof(*message);
            }
        case ERROR_UNKNOWN_SESSION_SUBTYPE:
            {
                const Error_UnknownSession_Message* message =
                        getObjectFromBuffer<Error_UnknownSession_Message>(recvBuffer);
                if(message == nullptr) {
                    break;
                }

                SessionHandler::m_sessionInterface->receivedError(
                            session,
                            Session::errorCodes::UNKNOWN_SESSION,
                            std::string(message->message));
                return sizeof(*message);
            }

        case ERROR_INVALID_MESSAGE_SUBTYPE:
        {
            const Error_InvalidMessage_Message* message =
                    getObjectFromBuffer<Error_InvalidMessage_Message>(recvBuffer);
            if(message == nullptr) {
                break;
            }

            SessionHandler::m_sessionInterface->receivedError(
                        session,
                        Session::errorCodes::INVALID_MESSAGE_SIZE,
                        std::string(message->message));
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

#endif // ERROR_PROCESSING_H
