/**
 * @file        error_processing.h
 *
 * @brief       send and handle messages of error-type
 *
 * @author      Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 * @copyright   Apache License Version 2.0
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

#include <libKitsunemimiNetwork/abstract_socket.h>
#include <libKitsunemimiNetwork/message_ring_buffer.h>

#include <libKitsunemimiProjectCommon/network_session/session_controller.h>
#include <libKitsunemimiProjectCommon/network_session/session.h>

#include <libKitsunemimiPersistence/logger/logger.h>

using Kitsunemimi::Network::MessageRingBuffer;
using Kitsunemimi::Network::getObjectFromBuffer;

namespace Kitsunemimi
{
namespace Project
{
namespace Common
{

/**
 * @brief send error-message to the other side
 *
 * @param session pointer to the session
 * @param errorCode error-code enum to automatic identify the error-message by code
 * @param message human readable error-message for log-output
 */
inline void
send_ErrorMessage(Session* session,
                  const uint8_t errorCode,
                  const std::string &message)
{
    LOG_DEBUG("SEND error message");

    switch(errorCode)
    {
        //------------------------------------------------------------------------------------------
        case Session::errorCodes::FALSE_VERSION:
        {
            Error_FalseVersion_Message errorMessage(
                    session->sessionId(),
                    session->increaseMessageIdCounter(),
                    message);
            SessionHandler::m_sessionInterface->sendMessage(session,
                                                            errorMessage.commonHeader,
                                                            &errorMessage,
                                                            sizeof(errorMessage));
            break;
        }
        //------------------------------------------------------------------------------------------
        case Session::errorCodes::UNKNOWN_SESSION:
        {
            Error_UnknownSession_Message errorMessage(
                    session->sessionId(),
                    session->increaseMessageIdCounter(),
                    message);
            SessionHandler::m_sessionInterface->sendMessage(session,
                                                            errorMessage.commonHeader,
                                                            &errorMessage,
                                                            sizeof(errorMessage));
            break;
        }
        //------------------------------------------------------------------------------------------
        case Session::errorCodes::INVALID_MESSAGE_SIZE:
        {
            Error_InvalidMessage_Message errorMessage(
                    session->sessionId(),
                    session->increaseMessageIdCounter(),
                    message);
            SessionHandler::m_sessionInterface->sendMessage(session,
                                                            errorMessage.commonHeader,
                                                            &errorMessage,
                                                            sizeof(errorMessage));
            break;
        }
        //------------------------------------------------------------------------------------------
        default:
            break;
    }
}

/**
 * @brief process messages of error-type
 *
 * @param session pointer to the session
 * @param header pointer to the common header of the message within the message-ring-buffer
 * @param recvBuffer pointer to the message-ring-buffer
 *
 * @return number of processed bytes
 */
inline uint64_t
process_Error_Type(Session* session,
                   const CommonMessageHeader* header,
                   MessageRingBuffer* recvBuffer)
{
    if(DEBUG_MODE) {
        LOG_DEBUG("process error message");
    }

    switch(header->subType)
    {
        //------------------------------------------------------------------------------------------
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
                            std::string(message->message, message->messageSize));
                return sizeof(*message);
            }
        //------------------------------------------------------------------------------------------
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
                            std::string(message->message, message->messageSize));
                return sizeof(*message);
            }
        //------------------------------------------------------------------------------------------
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
                            std::string(message->message, message->messageSize));
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

#endif // ERROR_PROCESSING_H
