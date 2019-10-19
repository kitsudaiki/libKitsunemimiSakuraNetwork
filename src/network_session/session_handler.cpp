/**
 *  @file       session_handler.cpp
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

#include <libKitsuneProjectCommon/network_session/session_handler.h>
#include <network_session/timer_thread.h>
#include <network_session/ressource_handler.h>

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

Kitsune::Project::Common::TimerThread* SessionHandler::m_timerThread = nullptr;
Kitsune::Project::Common::SessionHandler* SessionHandler::m_sessionHandler = nullptr;
Kitsune::Project::Common::RessourceHandler* SessionHandler::m_ressourceHandler = nullptr;

/**
 * @brief Session::Session
 */
SessionHandler::SessionHandler(void* sessionTarget,
                               void (*processSession)(void*,
                                                      Session*),
                               void* dataTarget,
                               void (*processData)(void*,
                                                   Session*,
                                                   void*,
                                                   const uint32_t),
                               void* errorTarget,
                               void (*processError)(void*,
                                                    Session*,
                                                    const uint8_t,
                                                    const std::string))
{
    m_sessionTarget = sessionTarget;
    m_processSession = processSession;

    if(m_timerThread == nullptr)
    {
        m_timerThread = new TimerThread;
        m_timerThread->start();
    }

    if(m_ressourceHandler == nullptr) {
        m_ressourceHandler = new RessourceHandler(dataTarget,
                                                  processData,
                                                  errorTarget,
                                                  processError);
    }

    if(m_sessionHandler == nullptr) {
        m_sessionHandler = this;
    }
}

/**
 * @brief Session::~Session
 */
SessionHandler::~SessionHandler()
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
SessionHandler::addUnixDomainServer(const std::string socketFile)
{
    Network::UnixDomainServer* server = new Network::UnixDomainServer(this,
                                                                      &processConnectionUnixDomain);
    server->initServer(socketFile);
    server->start();

    m_ressourceHandler->m_serverIdCounter++;
    m_ressourceHandler->m_servers.insert(std::pair<uint32_t, Network::AbstractServer*>(
                         m_ressourceHandler->m_serverIdCounter, server));

    return m_ressourceHandler->m_serverIdCounter;
}

/**
 * @brief SessionHandler::addTcpServer
 * @param port
 * @return
 */
uint32_t
SessionHandler::addTcpServer(const uint16_t port)
{
    Network::TcpServer* server = new Network::TcpServer(this,
                                                        &processConnectionTcp);
    server->initServer(port);
    server->start();

    m_ressourceHandler->m_serverIdCounter++;
    m_ressourceHandler->m_servers.insert(std::pair<uint32_t, Network::AbstractServer*>(
                         m_ressourceHandler->m_serverIdCounter, server));

    return m_ressourceHandler->m_serverIdCounter;
}

/**
 * @brief SessionHandler::addTlsTcpServer
 * @param port
 * @param certFile
 * @param keyFile
 * @return
 */
uint32_t
SessionHandler::addTlsTcpServer(const uint16_t port,
                                const std::string certFile,
                                const std::string keyFile)
{
    Network::TlsTcpServer* server = new Network::TlsTcpServer(this,
                                                              &processConnectionTlsTcp,
                                                              certFile,
                                                              keyFile);
    server->initServer(port);
    server->start();

    m_ressourceHandler->m_serverIdCounter++;
    m_ressourceHandler->m_servers.insert(std::pair<uint32_t, Network::AbstractServer*>(
                         m_ressourceHandler->m_serverIdCounter, server));

    return m_ressourceHandler->m_serverIdCounter;
}

/**
 * @brief SessionHandler::closeServer
 * @param id
 * @return
 */
bool
SessionHandler::closeServer(const uint32_t id)
{
    std::map<uint32_t, Network::AbstractServer*>::iterator it;
    it = m_ressourceHandler->m_servers.find(id);

    if(it != m_ressourceHandler->m_servers.end())
    {
        Network::AbstractServer* server = it->second;
        server->closeServer();
        delete server;
        m_ressourceHandler->m_servers.erase(it);
        return true;
    }

    return false;
}

/**
 * @brief SessionHandler::getServer
 * @param id
 * @return
 */
Network::AbstractServer*
SessionHandler::getServer(const uint32_t id)
{
    std::map<uint32_t, Network::AbstractServer*>::iterator it;
    it = m_ressourceHandler->m_servers.find(id);

    if(it != m_ressourceHandler->m_servers.end()) {
        return it->second;
    }

    return nullptr;
}

//==================================================================================================

/**
 * @brief SessionHandler::addUnixDomainSocket
 * @param socketFile
 */
void
SessionHandler::startUnixDomainSession(const std::string socketFile)
{
    Network::UnixDomainSocket* unixDomainSocket = new Network::UnixDomainSocket(socketFile);
    Session* newSession = new Session(unixDomainSocket);

    unixDomainSocket->setMessageCallback(newSession, &processMessageUnixDomain);
    newSession->sessionId = m_ressourceHandler->increaseSessionIdCounter();

    m_ressourceHandler->addSession(newSession->sessionId, newSession);
    newSession->connect(true);
}

/**
 * @brief SessionHandler::addTcpClient
 * @param address
 * @param port
 */
void
SessionHandler::startTcpSession(const std::string address,
                                const uint16_t port)
{
    Network::TcpSocket* tcpSocket = new Network::TcpSocket(address, port);
    Session* newSession = new Session(tcpSocket);

    tcpSocket->setMessageCallback(newSession, &processMessageTcp);
    newSession->sessionId = m_ressourceHandler->increaseSessionIdCounter();

    m_ressourceHandler->addSession(newSession->sessionId, newSession);
    newSession->connect(true);
}

/**
 * @brief SessionHandler::addTlsTcpClient
 * @param address
 * @param port
 * @param certFile
 * @param keyFile
 */
void
SessionHandler::startTlsTcpSession(const std::string address,
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
    newSession->sessionId = m_ressourceHandler->increaseSessionIdCounter();

    m_ressourceHandler->addSession(newSession->sessionId, newSession);
    newSession->connect(true);
}

/**
 * @brief SessionHandler::closeSocket
 * @param id
 * @return
 */
bool
SessionHandler::closeSession(const uint32_t id)
{
    std::map<uint32_t, Session*>::iterator it;
    it = m_ressourceHandler->m_sessions.find(id);

    if(it != m_ressourceHandler->m_sessions.end()) {
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
SessionHandler::getSession(const uint32_t id)
{
    std::map<uint32_t, Session*>::iterator it;
    it = m_ressourceHandler->m_sessions.find(id);

    if(it != m_ressourceHandler->m_sessions.end()) {
        return it->second;
    }

    return nullptr;
}

//==================================================================================================

/**
 * @brief SessionHandler::isIdUsed
 * @param id
 * @return
 */
bool
SessionHandler::isIdUsed(const uint32_t id)
{
    std::map<uint32_t, Session*>::iterator it;
    it = m_ressourceHandler->m_sessions.find(id);

    if(it != m_ressourceHandler->m_sessions.end()) {
        return true;
    }

    return false;
}

} // namespace Common
} // namespace Project
} // namespace Kitsune
