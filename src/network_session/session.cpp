/**
 * @file       session.h
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

#include <libKitsunemimiProjectCommon/network_session/session.h>
#include <libKitsunemimiNetwork/abstract_socket.h>

#include <network_session/messages_processing/session_processing.h>
#include <network_session/messages_processing/heartbeat_processing.h>
#include <network_session/messages_processing/singleblock_data_processing.h>
#include <network_session/messages_processing/multiblock_data_processing.h>

#include <libKitsunemimiPersistence/logger/logger.h>

enum statemachineItems {
    NOT_CONNECTED = 1,
    CONNECTED = 2,
    SESSION_NOT_READY = 3,
    SESSION_READY = 4,
    NORMAL = 5,
    IN_DATATRANSFER = 6,

    CONNECT = 7,
    DISCONNECT = 8,
    START_SESSION = 9,
    STOP_SESSION = 10,
    START_DATATRANSFER = 11,
    STOP_DATATRANSFER = 12,

};


namespace Kitsunemimi
{
namespace Project
{
namespace Common
{

/**
 * @brief constructor
 *
 * @param socket pointer to socket
 */
Session::Session(Network::AbstractSocket* socket)
{
    m_socket = socket;

    initStatemachine();
}

/**
 * @brief destructor
 */
Session::~Session()
{
    closeSession(false);
}

/**
 * @brief send data as stream
 *
 * @param data data-pointer
 * @param size number of bytes
 * @param dynamic if true, packets are not bigger as necessary, but its slower
 * @param replyExpected if true, the other side sends a reply-message to check timeouts
 *
 * @return false if session is NOT ready to send, else true
 */
bool
Session::sendStreamData(const void* data,
                        const uint64_t size,
                        const bool dynamic,
                        const bool replyExpected)
{
    if(m_sessionReady == false) {
        return false;
    }

    if(dynamic) {
        send_Data_Single_Dynamic(this, data, size, replyExpected);
    } else {
        send_Data_Single_Static(this, data, size, replyExpected);
    }

    return true;
}

/**
 * @brief send data a multi-block-message
 *
 * @param data data-pointer
 * @param size number of bytes
 *
 * @return false if session is NOT ready to send, else true
 */
bool
Session::sendStandaloneData(const void* data,
                            const uint64_t size)
{
    if(m_sessionReady == false) {
        return false;
    }

    if(startMultiblockDataTransfer(size))
    {
        SessionHandler::m_sessionInterface->writeDataIntoBuffer(this, data, size);
        send_Data_Multi_Init(this, size);
        return true;
    }

    return false;
}

/**
 * @brief close the session inclusive multiblock-messages, statemachine, message to the other side
 *        and close the socket
 *
 * @param replyExpected true, to expect a reply-message
 *
 * @return true, if all was successful, else false
 */
bool
Session::closeSession(const bool replyExpected)
{
    if(m_statemachine.isInState(SESSION_NOT_READY)) {
        return false;
    }

    finishMultiblockDataTransfer(true);
    if(replyExpected) {
        send_Session_Close_Start(this, true);
    }
    else {
        return endSession(true);
    }

    return true;
}

/**
 * @brief getter for the id of the session
 *
 * @return session-id
 */
uint32_t
Session::sessionId() const
{
    return m_sessionId;
}

/**
 * @brief check if session is client- or server-side
 *
 * @return true, if session is on client-side, else false
 */
bool
Session::isClientSide() const
{
    return m_socket->isClientSide();
}

/**
 * @brief create the network connection of the session
 *
 * @param sessionId id for the session
 * @param sessionIdentifier session-identifier value to identify the session on server-side
 *                          before the first data-message was send
 * @param init true to start the initial message-transfer
 *
 * @return false if session is already init or socker can not be connected, else true
 */
bool
Session::connectiSession(const uint32_t sessionId,
                         const uint64_t sessionIdentifier,
                         const bool init)
{
    LOG_DEBUG("CALL session connect: " + std::to_string(m_sessionId));

    // check if already connected
    if(m_statemachine.isInState(NOT_CONNECTED) == false) {
        return false;
    }

    // connect socket
    if(m_socket->initClientSide() == false) {
        return false;
    }

    // git into connected state
    if(m_statemachine.goToNextState(CONNECT) == false) {
        return false;
    }
    m_sessionId = sessionId;
    m_socket->startThread();

    // init session
    if(init) {
        send_Session_Init_Start(this, sessionIdentifier);
    }

    return true;
}

/**
 * @brief bring the session into ready-state after a successful initial message-transfer
 *
 * @param sessionId final id for the session
 * @param sessionIdentifier session-identifier value to identify the session on server-side
 *                          before the first data-message was send
 *
 * @return false, if session is already in ready-state, else true
 */
bool
Session::makeSessionReady(const uint32_t sessionId,
                          const uint64_t sessionIdentifier)
{
    LOG_DEBUG("CALL make session ready: " + std::to_string(m_sessionId));

    if(m_statemachine.goToNextState(START_SESSION, SESSION_NOT_READY))
    {
        m_sessionReady = true;
        m_sessionId = sessionId;
        m_sessionIdentifier = sessionIdentifier;

        m_processSession(m_sessionTarget, this, m_sessionIdentifier);

        return true;
    }

    return false;
}

/**
 * @brief initialize multiblock-message by data-buffer for a new multiblock and bring statemachine
 *        into required state
 *
 * @param size total size of the payload of the message (no header)
 *
 * @return false, if session is already in send/receive of a multiblock-message
 */
bool
Session::startMultiblockDataTransfer(const uint64_t size)
{
    LOG_DEBUG("CALL start multiblock data-transfer: " + std::to_string(m_sessionId));

    if(m_statemachine.goToNextState(START_DATATRANSFER, NORMAL))
    {
        m_inMultiMessage = true;

        const uint32_t numberOfBlocks = static_cast<uint32_t>(size / 4096) + 1;
        m_multiBlockBuffer = new Kitsunemimi::Common::DataBuffer(numberOfBlocks);

        return true;
    }

    return false;
}

/**
 * @brief append data to the data-buffer for the multiblock-message
 *
 * @param data pointer to the data
 * @param size number of bytes
 *
 * @return false, if session is not in the multiblock-transfer-state
 */
bool
Session::writeDataIntoBuffer(const void* data,
                             const uint64_t size)
{
    if(DEBUG_MODE) {
        LOG_DEBUG("CALL write data into buffer: " + std::to_string(m_sessionId));
    }

    if(m_inMultiMessage == false) {
        return false;
    }

    return Kitsunemimi::Common::addDataToBuffer(m_multiBlockBuffer,
                                            data,
                                            size);
}

/**
 * @brief last step of a mutliblock data-transfer by cleaning the buffer. Can also initialize the
 *        abort-process for a multiblock-datatransfer
 *
 * @param initAbort true to initialize an abort-process

 * @return true, if statechange was successful, else false
 */
bool
Session::finishMultiblockDataTransfer(const bool initAbort)
{
    LOG_DEBUG("CALL finish multiblock data-transfer: " + std::to_string(m_sessionId));

    // abort multi-block data-transfer, if one is in progress
    if(m_statemachine.goToNextState(STOP_DATATRANSFER))
    {
        m_inMultiMessage = false;

        if(initAbort) {
            send_Data_Multi_Abort(this);
        }

        if(m_multiBlockBuffer != nullptr)
        {
            delete m_multiBlockBuffer;
            m_multiBlockBuffer = nullptr;
        }

        return true;
    }

    return false;
}

/**
 * @brief stop the session to prevent it from all data-transfers. Delete the session from the
 *        session-handler and close the socket.
 *
 * @param init true, if the caller of the methods initialize the closing process for the session
 *
 * @return true, if statechange and socket-disconnect were successful, else false
 */
bool
Session::endSession(const bool init)
{
    LOG_DEBUG("CALL session close: " + std::to_string(m_sessionId));

    // try to stop the session
    if(m_statemachine.goToNextState(STOP_SESSION))
    {
        m_sessionReady = false;
        if(init) {
            send_Session_Close_Start(this, false);
        }

        SessionHandler::m_sessionHandler->removeSession(m_sessionId);
        return disconnectSession();
    }

    return false;
}

/**
 * @brief disconnect the socket within the sesson
 *
 * @return true, is state-change of the statemachine and closing sthe socket were successful,
 *         else false
 */
bool
Session::disconnectSession()
{
    LOG_DEBUG("CALL session disconnect: " + std::to_string(m_sessionId));

    if(m_statemachine.goToNextState(DISCONNECT))
    {
        return m_socket->closeSocket();
    }

    return false;
}

/**
 * @brief send a heartbeat-message
 *
 * @return true, if session is ready, else false
 */
bool
Session::sendHeartbeat()
{
    LOG_DEBUG("CALL send hearbeat: " + std::to_string(m_sessionId));

    if(m_statemachine.isInState(SESSION_READY))
    {
        send_Heartbeat_Start(this);
        return true;
    }

    return false;
}

/**
 * @brief init the statemachine
 */
void
Session::initStatemachine()
{
    // init states
    assert(m_statemachine.createNewState(NOT_CONNECTED, "not connected"));
    assert(m_statemachine.createNewState(CONNECTED, "connected"));
    assert(m_statemachine.createNewState(SESSION_NOT_READY, "session not ready"));
    assert(m_statemachine.createNewState(SESSION_READY, "session ready"));
    assert(m_statemachine.createNewState(NORMAL, "normal"));
    assert(m_statemachine.createNewState(IN_DATATRANSFER, "in datatransfer"));

    // set child state
    assert(m_statemachine.addChildState(CONNECTED,     SESSION_NOT_READY));
    assert(m_statemachine.addChildState(CONNECTED,     SESSION_READY));
    assert(m_statemachine.addChildState(SESSION_READY, NORMAL));
    assert(m_statemachine.addChildState(SESSION_READY, IN_DATATRANSFER));

    // set initial states
    assert(m_statemachine.setInitialChildState(CONNECTED,     SESSION_NOT_READY));
    assert(m_statemachine.setInitialChildState(SESSION_READY, NORMAL));

    // init transitions
    assert(m_statemachine.addTransition(NOT_CONNECTED,     CONNECT,            CONNECTED));
    assert(m_statemachine.addTransition(CONNECTED,         DISCONNECT,         NOT_CONNECTED));
    assert(m_statemachine.addTransition(SESSION_NOT_READY, START_SESSION,      SESSION_READY));
    assert(m_statemachine.addTransition(SESSION_READY,     STOP_SESSION,       SESSION_NOT_READY));
    assert(m_statemachine.addTransition(NORMAL,            START_DATATRANSFER, IN_DATATRANSFER));
    assert(m_statemachine.addTransition(IN_DATATRANSFER,   STOP_DATATRANSFER,  NORMAL));
}

/**
 * @brief increase the message-id-counter and return the new id
 *
 * @return new message id
 */
uint32_t
Session::increaseMessageIdCounter()
{
    uint32_t tempId = 0;
    while (m_messageIdCounter_lock.test_and_set(std::memory_order_acquire)) 
                 ; // spin
    m_messageIdCounter++;
    tempId = m_messageIdCounter;
    m_messageIdCounter_lock.clear(std::memory_order_release);
    return tempId;
}

} // namespace Common
} // namespace Project
} // namespace Kitsunemimi
