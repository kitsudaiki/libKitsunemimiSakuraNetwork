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
    this->socket = socket;
    initStatemachine();
}

Session::~Session()
{
    // TODO
}

/**
 * @brief Session::connect
 * @return
 */
bool
Session::connect(const bool init)
{
    LOG_DEBUG("CALL session connect: " + std::to_string(sessionId));

    // check if already connected
    if(m_statemachine.isInState(CONNECTED)) {
        return true;
    }

    // connect socket
    const bool connected = socket->initClientSide();
    if(connected == false) {
        return false;
    }

    // git into connected state
    const bool ret = m_statemachine.goToNextState(CONNECT);
    LOG_DEBUG("state of state machine: " + m_statemachine.getCurrentState());

    if(ret == false) {
        return false;
    }

    // start socket-thread to listen for incoming messages
    socket->start();

    // init session
    if(init) {
        send_Session_Init_Start(sessionId, socket);
    }

    return true;
}

/**
 * @brief Session::startSession
 *
 * @return
 */
bool
Session::startSession()
{
    LOG_DEBUG("CALL session start: " + std::to_string(sessionId));

    const bool ret = m_statemachine.goToNextState(START_SESSION);
    LOG_DEBUG("state of state machine: " + m_statemachine.getCurrentState());
    return ret;
}

/**
 * @brief Session::disconnect
 * @return
 */
bool
Session::disconnect()
{
    LOG_DEBUG("CALL session disconnect: " + std::to_string(sessionId));

    if(m_statemachine.isInState(CONNECTED))
    {
        const bool ret = m_statemachine.goToNextState(DISCONNECT);
        LOG_DEBUG("state of state machine: " + m_statemachine.getCurrentState());
        if(ret == false) {
            return false;
        }

        socket->closeSocket();
        return true;
    }
    return false;
}

/**
 * @brief Session::closeSession
 *
 * @param init
 * @param replyExpected
 *
 * @return
 */
bool
Session::closeSession(const bool init,
                      const bool replyExpected)
{
    LOG_DEBUG("CALL session close: " + std::to_string(sessionId));

    if(m_statemachine.isInState(SESSION_READY))
    {
        const bool ret = m_statemachine.goToNextState(STOP_SESSION);
        LOG_DEBUG("state of state machine: " + m_statemachine.getCurrentState());
        if(ret == false) {
            return false;
        }

        if(init) {
            send_Session_Close_Start(sessionId, replyExpected, socket);
        }

        return true;
    }

    return false;
}

/**
 * @brief Session::sendHeartbeat
 * @return
 */
bool
Session::sendHeartbeat()
{
    LOG_DEBUG("CALL send hearbeat: " + std::to_string(sessionId));

    if(m_statemachine.isInState(SESSION_READY))
    {
        send_Heartbeat_Start(sessionId, socket);
        return true;
    }

    return false;
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
