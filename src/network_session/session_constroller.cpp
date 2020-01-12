/**
 * @file       session_controller.cpp
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

#include <libKitsunemimiProjectNetwork/network_session/session_controller.h>

#include <network_session/timer_thread.h>
#include <network_session/session_handler.h>
#include <network_session/callbacks.h>

#include <libKitsunemimiNetwork/tcp/tcp_server.h>
#include <libKitsunemimiNetwork/tcp/tcp_socket.h>
#include <libKitsunemimiNetwork/unix/unix_domain_server.h>
#include <libKitsunemimiNetwork/unix/unix_domain_socket.h>
#include <libKitsunemimiNetwork/tls_tcp/tls_tcp_server.h>
#include <libKitsunemimiNetwork/tls_tcp/tls_tcp_socket.h>
#include <libKitsunemimiNetwork/abstract_server.h>
#include <libKitsunemimiNetwork/abstract_socket.h>

#include <libKitsunemimiPersistence/logger/logger.h>

namespace Kitsunemimi
{
namespace Project
{
namespace Common
{

SessionController* SessionController::m_sessionController = nullptr;

/**
 * @brief constructor
 */
SessionController::SessionController(void* sessionTarget,
                                     void (*processSession)(void*,
                                                            bool,
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
    m_sessionController = this;

    if(SessionHandler::m_sessionHandler == nullptr)
    {
        SessionHandler::m_sessionHandler = new SessionHandler(sessionTarget,
                                                              processSession,
                                                              dataTarget,
                                                              processData,
                                                              errorTarget,
                                                              processError);
    }
}

/**
 * @brief destructor
 */
SessionController::~SessionController()
{
    closeAllSession();
    cloesAllServers();

    if(SessionHandler::m_sessionHandler != nullptr)
    {
        delete SessionHandler::m_sessionHandler;
        SessionHandler::m_sessionHandler = nullptr;
    }
}

//==================================================================================================

/**
 * @brief add new unix-domain-server
 *
 * @param socketFile file-path for the server
 *
 * @return id of the new server
 */
uint32_t
SessionController::addUnixDomainServer(const std::string socketFile)
{
    Network::UnixDomainServer* server = new Network::UnixDomainServer(this,
                                                                      &processConnection_Callback);
    server->initServer(socketFile);
    server->startThread();

    SessionHandler* sessionHandler = SessionHandler::m_sessionHandler;
    m_serverIdCounter++;
    sessionHandler->m_servers.insert(std::pair<uint32_t, Network::AbstractServer*>(
                                     m_serverIdCounter, server));

    return m_serverIdCounter;
}

/**
 * @brief add new tcp-server
 *
 * @param port port where the server should listen
 *
 * @return id of the new server
 */
uint32_t
SessionController::addTcpServer(const uint16_t port)
{
    Network::TcpServer* server = new Network::TcpServer(this,
                                                        &processConnection_Callback);
    server->initServer(port);
    server->startThread();

    SessionHandler* sessionHandler = SessionHandler::m_sessionHandler;
    m_serverIdCounter++;
    sessionHandler->m_servers.insert(std::pair<uint32_t, Network::AbstractServer*>(
                                     m_serverIdCounter, server));

    return m_serverIdCounter;
}

/**
 * @brief add new tls-encrypted tcp-server
 *
 * @param port port where the server should listen
 * @param certFile certificate-file for tls-encryption
 * @param keyFile key-file for tls-encryption
 *
 * @return id of the new server
 */
uint32_t
SessionController::addTlsTcpServer(const uint16_t port,
                                   const std::string certFile,
                                   const std::string keyFile)
{
    Network::TlsTcpServer* server = new Network::TlsTcpServer(this,
                                                              &processConnection_Callback,
                                                              certFile,
                                                              keyFile);
    server->initServer(port);
    server->startThread();

    SessionHandler* sessionHandler = SessionHandler::m_sessionHandler;
    m_serverIdCounter++;
    sessionHandler->m_servers.insert(std::pair<uint32_t, Network::AbstractServer*>(
                                     m_serverIdCounter, server));

    return m_serverIdCounter;
}

/**
 * @brief close server
 *
 * @param id id of the server
 *
 * @return false, if id not exist, else true
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

/**
 * @brief SessionController::cloesAllServers
 */
void
SessionController::cloesAllServers()
{
    std::map<uint32_t, Network::AbstractServer*>::iterator it;
    for(it = SessionHandler::m_sessionHandler->m_servers.begin();
        it != SessionHandler::m_sessionHandler->m_servers.end();
        it++)
    {
        it->second->closeServer();
    }
}

//==================================================================================================

/**
 * @brief SessionHandler::addUnixDomainSocket
 * @param socketFile
 */
bool
SessionController::startUnixDomainSession(const std::string socketFile,
                                          const uint64_t sessionIdentifier)
{
    Network::UnixDomainSocket* unixDomainSocket = new Network::UnixDomainSocket(socketFile);
    return startSession(unixDomainSocket, sessionIdentifier);
}

/**
 * @brief SessionHandler::addTcpClient
 * @param address
 * @param port
 */
bool
SessionController::startTcpSession(const std::string address,
                                   const uint16_t port,
                                   const uint64_t sessionIdentifier)
{
    Network::TcpSocket* tcpSocket = new Network::TcpSocket(address, port);
    return startSession(tcpSocket, sessionIdentifier);
}

/**
 * @brief SessionHandler::addTlsTcpClient
 * @param address
 * @param port
 * @param certFile
 * @param keyFile
 */
bool
SessionController::startTlsTcpSession(const std::string address,
                                      const uint16_t port,
                                      const std::string certFile,
                                      const std::string keyFile,
                                      const uint64_t sessionIdentifier)
{
    Network::TlsTcpSocket* tlsTcpSocket = new Network::TlsTcpSocket(address,
                                                                    port,
                                                                    certFile,
                                                                    keyFile);
    return startSession(tlsTcpSocket, sessionIdentifier);
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

/**
 * @brief SessionController::closeAllSession
 */
void
SessionController::closeAllSession()
{
    std::map<uint32_t, Session*>::iterator it;
    for(it = SessionHandler::m_sessionHandler->m_sessions.begin();
        it != SessionHandler::m_sessionHandler->m_sessions.end();
        it++)
    {
        it->second->closeSession();
    }
}

/**
 * @brief SessionController::startSession
 * @param socket
 * @param sessionIdentifier
 * @return
 */
bool
SessionController::startSession(Network::AbstractSocket *socket,
                                const uint64_t sessionIdentifier)
{
    Session* newSession = SessionHandler::m_sessionInterface->createNewSession(socket);
    socket->setMessageCallback(newSession, &processMessage_callback);

    const uint32_t newId = SessionHandler::m_sessionHandler->increaseSessionIdCounter();
    SessionHandler::m_sessionHandler->addSession(newId, newSession);
    return SessionHandler::m_sessionInterface->connectiSession(newSession, newId, sessionIdentifier, true);
}

//==================================================================================================

} // namespace Common
} // namespace Project
} // namespace Kitsunemimi
