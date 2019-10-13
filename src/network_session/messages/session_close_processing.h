/**
 *  @file       session_close_processing.h
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

#ifndef SESSION_CLOSE_PROCESSING_H
#define SESSION_CLOSE_PROCESSING_H

#include <libKitsuneProjectCommon/network_session/session_handler.h>
#include <network_session/messages/message_definitions.h>
#include <network_session/messages/message_creation.h>
#include <libKitsuneNetwork/abstract_socket.h>

#include <libKitsunePersistence/logger/logger.h>

using Kitsune::Network::AbstractSocket;


namespace Kitsune
{
namespace Project
{
namespace Common
{

inline void
process_Session_Close_Start(const Session_Close_Start_Message* message,
                            AbstractSocket* socket)
{

}

inline void
process_Session_Close_Reply(const Session_Close_Reply_Message* message,
                            AbstractSocket* socket)
{

}

} // namespace Common
} // namespace Project
} // namespace Kitsune

#endif // SESSION_CLOSE_PROCESSING_H
