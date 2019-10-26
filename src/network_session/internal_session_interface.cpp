/**
 *  @file       internal_session_interface.cpp
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

#include "internal_session_interface.h"

#include <libKitsuneProjectCommon/network_session/session.h>

#include <network_session/message_definitions.h>
#include <network_session/session_handler.h>

#include <libKitsuneNetwork/abstract_socket.h>
#include <libKitsuneCommon/data_buffer.h>
#include <libKitsunePersistence/logger/logger.h>

namespace Kitsune
{
namespace Project
{
namespace Common
{

/**
 * @brief InternalSessionInterface::InternalSessionInterface
 * @param sessionTarget
 * @param dataTarget
 * @param errorTarget
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
 * @brief InternalSessionInterface::~InternalSessionInterface
 */
InternalSessionInterface::~InternalSessionInterface()
{

}

/**
 * @brief InternalSessionInterface::createNewSession
 * @param socket
 * @return
 */
Session*
InternalSessionInterface::createNewSession(Network::AbstractSocket* socket)
{
    return new Session(socket);
}

/**
 * @brief InternalSessionInterface::receivedData
 * @param session
 * @param isStream
 * @param data
 * @param dataSize
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
 * @brief RessourceHandler::receivedError
 * @param session
 * @param errorCode
 * @param message
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
 * @brief RessourceHandler::sendMessage
 * @param session
 * @param data
 * @param size
 * @return
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
 * @brief InternalSessionInterface::initMultiblockBuffer
 * @param session
 * @param size
 * @return
 */
bool
InternalSessionInterface::initMultiblockBuffer(Session* session,
                                               const uint64_t size)
{
    const uint32_t numberOfBlocks = static_cast<uint32_t>(size / 4096) + 1;
    session->m_multiBlockBuffer = new Kitsune::Common::DataBuffer(numberOfBlocks);

    // TODO: check if allocation was successful
    return true;
}

/**
 * @brief InternalSessionInterface::writeDataIntoBuffer
 * @param session
 * @param data
 * @param size
 * @return
 */
bool
InternalSessionInterface::writeDataIntoBuffer(Session* session,
                                              const void* data,
                                              const uint64_t size)
{
    return Kitsune::Common::addDataToBuffer(session->m_multiBlockBuffer,
                                            data,
                                            size);
}

/**
 * @brief InternalSessionInterface::getTotalBufferSize
 * @param session
 * @return
 */
uint64_t
InternalSessionInterface::getTotalBufferSize(Session* session)
{
    return session->m_multiBlockBuffer->bufferPosition;
}

/**
 * @brief InternalSessionInterface::getDataPointer
 * @param session
 * @return
 */
uint8_t*
InternalSessionInterface::getDataPointer(Session *session)
{
    return session->m_multiBlockBuffer->getBlock(0);
}

/**
 * @brief InternalSessionInterface::deleteBuffer
 * @param session
 * @return
 */
bool
InternalSessionInterface::deleteBuffer(Session* session)
{
    if(session->m_multiBlockBuffer != nullptr)
    {
        delete session->m_multiBlockBuffer;
        session->m_multiBlockBuffer = nullptr;
        return true;
    }

    return false;
}

/**
 * @brief InternalSessionInterface::sendHeartbeat
 * @param session
 */
void
InternalSessionInterface::sendHeartbeat(Session *session)
{
    session->sendHeartbeat();
}

/**
 * @brief RessourceHandler::connectiSession
 * @param session
 * @param sessionId
 * @param init
 * @return
 */
bool
InternalSessionInterface::connectiSession(Session* session,
                                          const uint32_t sessionId,
                                          const uint64_t customValue,
                                          const bool init)
{
    session->m_sessionTarget = m_sessionTarget;
    session->m_processSession = m_processSession;
    session->m_dataTarget = m_dataTarget;
    session->m_processData = m_processData;
    session->m_errorTarget = m_errorTarget;
    session->m_processError = m_processError;

    return session->connectiSession(sessionId, customValue, init);
}

/**
 * @brief RessourceHandler::makeSessionReady
 * @param session
 * @param sessionId
 * @return
 */
bool
InternalSessionInterface::makeSessionReady(Session* session,
                                           const uint32_t sessionId,
                                           const uint64_t customValue)
{
    session->m_sessionId = sessionId;
    session->m_customValue = customValue;
    return session->makeSessionReady();
}

/**
 * @brief RessourceHandler::endSession
 * @param session
 * @param init
 * @param replyExpected
 * @return
 */
bool
InternalSessionInterface::endSession(Session* session,
                                     const bool init,
                                     const bool replyExpected)
{
    return session->endSession(init, replyExpected);
}

/**
 * @brief RessourceHandler::disconnectSession
 * @param session
 * @return
 */
bool
InternalSessionInterface::disconnectSession(Session* session)
{
    return session->disconnectSession();
}

} // namespace Common
} // namespace Project
} // namespace Kitsune
