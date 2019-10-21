/**
 *  @file       session.h
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

#include <libKitsuneProjectCommon/network_session/session.h>
#include <libKitsuneNetwork/abstract_socket.h>

#include <network_session/messages_processing/session_processing.h>
#include <network_session/messages_processing/heartbeat_processing.h>
#include <network_session/messages_processing/data_processing.h>

#include <libKitsunePersistence/logger/logger.h>

#define NOT_CONNECTED "not connected"
#define CONNECTED "connected"
#define SESSION_NOT_READY "session not ready"
#define SESSION_READY "session init"
#define NORMAL "normal"
#define IN_DATATRANSFER "in datatransfer"

#define CONNECT "connect"
#define DISCONNECT "disconnect"
#define START_SESSION "start session"
#define STOP_SESSION "stop session"
#define START_DATATRANSFER "start datatransfer"
#define STOP_DATATRANSFER "stop datatransfer"

namespace Kitsune
{
namespace Project
{
namespace Common
{

Session::Session(Network::AbstractSocket* socket)
{
    m_socket = socket;
    initStatemachine();
}

Session::~Session()
{
    closeSession(false);
}

/**
 * @brief Session::sendData
 * @param data
 * @param size
 * @return
 */
bool
Session::sendData(const void* data,
                  const uint64_t size,
                  const bool dynamic,
                  const bool replyExpected)
{
    if(m_sessionReady == false) {
        return false;
    }

    send_Data_Single_Static(m_sessionId,
                            data,
                            size,
                            m_socket);
    return true;
}

/**
 * @brief Session::closeSession
 * @return
 */
bool
Session::closeSession(const bool replyExpected)
{
    return endSession(true, replyExpected);
}

/**
 * @brief Session::sessionId
 * @return
 */
uint32_t
Session::sessionId() const
{
    return m_sessionId;
}

/**
 * @brief Session::connect
 * @return
 */
bool
Session::connectiSession(const uint32_t sessionId,
                         bool init)
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
    m_socket->start();

    // init session
    if(init) {
        send_Session_Init_Start(m_sessionId, m_socket);
    }

    return true;
}

/**
 * @brief Session::startSession
 *
 * @return
 */
bool
Session::makeSessionReady()
{
    LOG_DEBUG("CALL session start: " + std::to_string(m_sessionId));

    // check statemachine
    if(m_statemachine.isInState(SESSION_NOT_READY) == false) {
        return false;
    }

    if(m_statemachine.goToNextState(START_SESSION) == false) {
        return false;
    }

    m_sessionReady = true;
    m_processSession(m_sessionTarget, this);

    return true;
}

/**
 * @brief Session::endSession
 *
 * @param init
 * @param replyExpected
 *
 * @return
 */
bool
Session::endSession(const bool init,
                    const bool replyExpected)
{
    LOG_DEBUG("CALL session close: " + std::to_string(m_sessionId));

    // check statemachine
    if(m_statemachine.isInState(SESSION_READY) == false) {
        return false;
    }

    if(m_statemachine.goToNextState(STOP_SESSION) == false) {
        return false;
    }
    m_sessionReady = true;;

    if(init) {
        send_Session_Close_Start(m_sessionId, replyExpected, m_socket);
    }

    return true;
}

/**
 * @brief Session::disconnect
 * @return
 */
bool
Session::disconnectSession()
{
    LOG_DEBUG("CALL session disconnect: " + std::to_string(m_sessionId));

    // check statemachine
    if(m_statemachine.isInState(CONNECTED) == false) {
        return false;
    }

    if(m_statemachine.goToNextState(DISCONNECT) == false) {
        return false;
    }
    m_socket->closeSocket();

    return true;
}

/**
 * @brief Session::sendHeartbeat
 * @return
 */
bool
Session::sendHeartbeat()
{
    LOG_DEBUG("CALL send hearbeat: " + std::to_string(m_sessionId));

    if(m_statemachine.isInState(SESSION_READY) == false) {
        return false;
    }

    send_Heartbeat_Start(m_sessionId, m_socket);

    return true;
}

/**
 * @brief Session::initStatemachine
 */
void
Session::initStatemachine()
{
    // init states
    assert(m_statemachine.createNewState(NOT_CONNECTED));
    assert(m_statemachine.createNewState(CONNECTED));
    assert(m_statemachine.createNewState(SESSION_NOT_READY));
    assert(m_statemachine.createNewState(SESSION_READY));
    assert(m_statemachine.createNewState(NORMAL));
    assert(m_statemachine.createNewState(IN_DATATRANSFER));

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

} // namespace Common
} // namespace Project
} // namespace Kitsune
