/**
 *  @file       session_controller.cpp
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

#include <libKitsuneProjectCommon/network_session/session_controller.h>
#include <network_session/timer_thread.h>
#include <network_session/session_handler.h>

#include <libKitsuneNetwork/tcp/tcp_server.h>
#include <libKitsuneNetwork/tcp/tcp_socket.h>
#include <libKitsuneNetwork/unix/unix_domain_server.h>
#include <libKitsuneNetwork/unix/unix_domain_socket.h>
#include <libKitsuneNetwork/tls_tcp/tls_tcp_server.h>
#include <libKitsuneNetwork/tls_tcp/tls_tcp_socket.h>
#include <libKitsuneNetwork/abstract_server.h>

#include <network_session/callbacks.h>

#include <libKitsunePersistence/logger/logger.h>

namespace Kitsune
{
namespace Project
{
namespace Common
{

SessionController* SessionController::m_sessionController = nullptr;

/**
 * @brief Session::Session
 */
SessionController::SessionController(void* sessionTarget,
                                     void (*processSession)(void*,
                                                      Session*),
                                     void* dataTarget,
                                     void (*processData)(void*,
                                                         Session*,
                                                         void*,
                                                         const uint64_t),
                                     void* errorTarget,
                                     void (*processError)(void*,
                                                          Session*,
                                                          const uint8_t,
                                                          const std::string))
{
    m_sessionTarget = sessionTarget;
    m_processSession = processSession;

    if(m_sessionController == nullptr) {
        m_sessionController = this;
    }

    if(SessionHandler::m_sessionHandler == nullptr)
    {
        SessionHandler::m_sessionHandler = new SessionHandler(dataTarget,
                                                              processData,
                                                              errorTarget,
                                                              processError);
    }
}

/**
 * @brief Session::~Session
 */
SessionController::~SessionController()
{
    // TODO: send finish-message
    // TODO: delete all from timer-thread
    // TODO: close socket
}

//==================================================================================================

/**
 * @brief SessionHandler::addUnixDomainServer
 * @param socketFile
 * @return
 */
uint32_t
SessionController::addUnixDomainServer(const std::string socketFile)
{
    Network::UnixDomainServer* server = new Network::UnixDomainServer(this,
                                                                      &processConnectionUnixDomain);
    server->initServer(socketFile);
    server->start();

    SessionHandler* sessionHandler = SessionHandler::m_sessionHandler;
    sessionHandler->m_serverIdCounter++;
    sessionHandler->m_servers.insert(std::pair<uint32_t, Network::AbstractServer*>(
                                     sessionHandler->m_serverIdCounter, server));

    return sessionHandler->m_serverIdCounter;
}

/**
 * @brief SessionHandler::addTcpServer
 * @param port
 * @return
 */
uint32_t
SessionController::addTcpServer(const uint16_t port)
{
    Network::TcpServer* server = new Network::TcpServer(this,
                                                        &processConnectionTcp);
    server->initServer(port);
    server->start();

    SessionHandler* sessionHandler = SessionHandler::m_sessionHandler;
    sessionHandler->m_serverIdCounter++;
    sessionHandler->m_servers.insert(std::pair<uint32_t, Network::AbstractServer*>(
                                     sessionHandler->m_serverIdCounter, server));

    return sessionHandler->m_serverIdCounter;
}

/**
 * @brief SessionHandler::addTlsTcpServer
 * @param port
 * @param certFile
 * @param keyFile
 * @return
 */
uint32_t
SessionController::addTlsTcpServer(const uint16_t port,
                                   const std::string certFile,
                                   const std::string keyFile)
{
    Network::TlsTcpServer* server = new Network::TlsTcpServer(this,
                                                              &processConnectionTlsTcp,
                                                              certFile,
                                                              keyFile);
    server->initServer(port);
    server->start();

    SessionHandler* sessionHandler = SessionHandler::m_sessionHandler;
    sessionHandler->m_serverIdCounter++;
    sessionHandler->m_servers.insert(std::pair<uint32_t, Network::AbstractServer*>(
                                     sessionHandler->m_serverIdCounter, server));

    return sessionHandler->m_serverIdCounter;
}

/**
 * @brief SessionHandler::closeServer
 * @param id
 * @return
 */
bool
SessionController::closeServer(const uint32_t id)
{
    std::map<uint32_t, Network::AbstractServer*>::iterator it;
    it = SessionHandler::m_sessionHandler->m_servers.find(id);

    if(it != SessionHandler::m_sessionHandler->m_servers.end())
    {
        Network::AbstractServer* server = it->second;
        server->closeServer();
        delete server;
        SessionHandler::m_sessionHandler->m_servers.erase(it);
        return true;
    }

    return false;
}

//==================================================================================================

/**
 * @brief SessionHandler::addUnixDomainSocket
 * @param socketFile
 */
void
SessionController::startUnixDomainSession(const std::string socketFile)
{
    Network::UnixDomainSocket* unixDomainSocket = new Network::UnixDomainSocket(socketFile);
    Session* newSession = new Session(unixDomainSocket);

    unixDomainSocket->setMessageCallback(newSession, &processMessageUnixDomain);
    newSession->m_sessionId = SessionHandler::m_sessionHandler->increaseSessionIdCounter();

    SessionHandler::m_sessionHandler->addSession(newSession->m_sessionId, newSession);
    newSession->connectiSession(newSession->m_sessionId, true);
}

/**
 * @brief SessionHandler::addTcpClient
 * @param address
 * @param port
 */
void
SessionController::startTcpSession(const std::string address,
                                   const uint16_t port)
{
    Network::TcpSocket* tcpSocket = new Network::TcpSocket(address, port);
    Session* newSession = new Session(tcpSocket);

    tcpSocket->setMessageCallback(newSession, &processMessageTcp);
    newSession->m_sessionId = SessionHandler::m_sessionHandler->increaseSessionIdCounter();

    SessionHandler::m_sessionHandler->addSession(newSession->m_sessionId, newSession);
    newSession->connectiSession(newSession->m_sessionId, true);
}

/**
 * @brief SessionHandler::addTlsTcpClient
 * @param address
 * @param port
 * @param certFile
 * @param keyFile
 */
void
SessionController::startTlsTcpSession(const std::string address,
                                      const uint16_t port,
                                      const std::string certFile,
                                      const std::string keyFile)
{
    Network::TlsTcpSocket* tlsTcpSocket = new Network::TlsTcpSocket(address,
                                                                    port,
                                                                    certFile,
                                                                    keyFile);
    Session* newSession = new Session(tlsTcpSocket);

    tlsTcpSocket->setMessageCallback(newSession, &processMessageTlsTcp);
    newSession->m_sessionId = SessionHandler::m_sessionHandler->increaseSessionIdCounter();

    SessionHandler::m_sessionHandler->addSession(newSession->m_sessionId, newSession);
    newSession->connectiSession(newSession->m_sessionId, true);
}

/**
 * @brief SessionHandler::closeSocket
 * @param id
 * @return
 */
bool
SessionController::closeSession(const uint32_t id)
{
    std::map<uint32_t, Session*>::iterator it;
    it = SessionHandler::m_sessionHandler->m_sessions.find(id);

    if(it != SessionHandler::m_sessionHandler->m_sessions.end()) {
        return it->second->closeSession(true);
    }

    return false;
}

/**
 * @brief SessionHandler::getSocket
 * @param id
 * @return
 */
Session*
SessionController::getSession(const uint32_t id)
{
    std::map<uint32_t, Session*>::iterator it;
    it = SessionHandler::m_sessionHandler->m_sessions.find(id);

    if(it != SessionHandler::m_sessionHandler->m_sessions.end()) {
        return it->second;
    }

    return nullptr;
}

//==================================================================================================

} // namespace Common
} // namespace Project
} // namespace Kitsune
