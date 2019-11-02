/**
 * @file        internal_session_interface.cpp
 *
 * @brief       Internal used class for access session-objects
 *
 * @detail      The session class is public accessible, but it has many variables and methods,
 *              where the most shouldn't ever used from outside this library. So these things had
 *              to be declared as private. Problem was, that the methods for handling of incoming
 *              messages are not within a class, but should have access to these stuff. So this
 *              internal class was added as friend class for the session-class. It would have been
 *              also possible to make all in the session-class public, but that would had make the
 *              class more complicate for new users of this library.
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

#include "internal_session_interface.h"

#include <libKitsunemimiProjectCommon/network_session/session.h>

#include <network_session/message_definitions.h>
#include <network_session/session_handler.h>

#include <libKitsunemimiNetwork/abstract_socket.h>
#include <libKitsunemimiCommon/data_buffer.h>
#include <libKitsunemimiPersistence/logger/logger.h>

namespace Kitsunemimi
{
namespace Project
{
namespace Common
{

/**
 * @brief constructor
 */
InternalSessionInterface::InternalSessionInterface(void* sessionTarget,
                                                   void (*processSession)(void*,
                                                                          Session*,
                                                                          const uint64_t),
                                                   void* dataTarget,
                                                   void (*processData)(void*,
                                                                       Session*,
                                                                       const bool,
                                                                       const void*,
                                                                       const uint64_t),
                                                   void* errorTarget,
                                                   void (*processError)(void*,
                                                                        Session*,
                                                                        const uint8_t,
                                                                        const std::string))
{
    m_sessionTarget = sessionTarget;
    m_processSession = processSession;
    m_dataTarget = dataTarget;
    m_processData = processData;
    m_errorTarget = errorTarget;
    m_processError = processError;
}

/**
 * @brief destructor
 */
InternalSessionInterface::~InternalSessionInterface() {}

/**
 * @brief createNewSession
 */
Session*
InternalSessionInterface::createNewSession(Network::AbstractSocket* socket)
{
    return new Session(socket);
}

/**
 * @brief receivedData
 */
void
InternalSessionInterface::receivedData(Session* session,
                                       const bool isStream,
                                       const void* data,
                                       const uint64_t dataSize)
{
    session->m_processData(session->m_dataTarget,
                           session,
                           isStream,
                           data,
                           dataSize);
}

/**
 * @brief receivedError
 */
void
InternalSessionInterface::receivedError(Session* session,
                                        const uint8_t errorCode,
                                        const std::string &message)
{
    LOG_ERROR("ERROR in networking: " + message);

    session->m_processError(session->m_errorTarget,
                            session,
                            errorCode,
                            message);
}

/**
 * @brief sendMessage
 */
void
InternalSessionInterface::sendMessage(Session* session,
                                      const CommonMessageHeader &header,
                                      const void* data,
                                      const uint64_t size)
{
    if(header.flags == 0x1)
    {
        SessionHandler::m_timerThread->addMessage(header.type,
                                                  header.sessionId,
                                                  header.messageId,
                                                  session);
    }

    session->m_socket->sendMessage(data, size);
}

/**
 * @brief initMultiblockBuffer
 */
bool
InternalSessionInterface::initMultiblockBuffer(Session* session,
                                               const uint64_t size)
{
    return session->startMultiblockDataTransfer(size);
}

/**
 * @brief writeDataIntoBuffer
 */
bool
InternalSessionInterface::writeDataIntoBuffer(Session* session,
                                              const void* data,
                                              const uint64_t size)
{
    return session->writeDataIntoBuffer(data, size);
}

/**
 * @brief finishMultiblockBuffer
 */
bool
InternalSessionInterface::finishMultiblockBuffer(Session* session)
{
    return session->finishMultiblockDataTransfer();
}

/**
 * @brief isInMultiblock
 */
bool
InternalSessionInterface::isInMultiblock(Session* session)
{
    return session->m_inMultiMessage;
}

/**
 * @brief getUsedBufferSize
 */
uint64_t
InternalSessionInterface::getUsedBufferSize(Session* session)
{
    return session->m_multiBlockBuffer->bufferPosition;
}

/**
 * @brief getDataPointer
 */
uint8_t*
InternalSessionInterface::getDataPointer(Session* session)
{
    return session->m_multiBlockBuffer->getBlock(0);
}

/**
 * @brief sendHeartbeat
 */
void
InternalSessionInterface::sendHeartbeat(Session* session)
{
    session->sendHeartbeat();
}

/**
 * @brief connectiSession
 */
bool
InternalSessionInterface::connectiSession(Session* session,
                                          const uint32_t sessionId,
                                          const uint64_t sessionIdentifier,
                                          const bool init)
{
    session->m_sessionTarget = m_sessionTarget;
    session->m_processSession = m_processSession;
    session->m_dataTarget = m_dataTarget;
    session->m_processData = m_processData;
    session->m_errorTarget = m_errorTarget;
    session->m_processError = m_processError;

    return session->connectiSession(sessionId, sessionIdentifier, init);
}

/**
 * @brief makeSessionReady
 */
bool
InternalSessionInterface::makeSessionReady(Session* session,
                                           const uint32_t sessionId,
                                           const uint64_t sessionIdentifier)
{
    return session->makeSessionReady(sessionId, sessionIdentifier);
}

/**
 * @brief endSession
 */
bool
InternalSessionInterface::endSession(Session* session,
                                     const bool init)
{
    return session->endSession(init);
}

/**
 * @brief disconnectSession
 */
bool
InternalSessionInterface::disconnectSession(Session* session)
{
    return session->disconnectSession();
}

} // namespace Common
} // namespace Project
} // namespace Kitsunemimi
