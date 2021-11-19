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

#include <libKitsunemimiSakuraNetwork/session_controller.h>

#include <handler/reply_handler.h>
#include <handler/message_blocker_handler.h>
#include <handler/session_handler.h>
#include <callbacks.h>
#include <messages_processing/session_processing.h>

#include <libKitsunemimiNetwork/tcp/tcp_server.h>
#include <libKitsunemimiNetwork/tcp/tcp_socket.h>
#include <libKitsunemimiNetwork/unix/unix_domain_server.h>
#include <libKitsunemimiNetwork/unix/unix_domain_socket.h>
#include <libKitsunemimiNetwork/tls_tcp/tls_tcp_server.h>
#include <libKitsunemimiNetwork/tls_tcp/tls_tcp_socket.h>
#include <libKitsunemimiNetwork/abstract_server.h>
#include <libKitsunemimiNetwork/abstract_socket.h>

#include <libKitsunemimiCommon/logger.h>

namespace Kitsunemimi
{
namespace Sakura
{

SessionController* SessionController::m_sessionController = nullptr;

/**
 * @brief constructor
 */
SessionController::SessionController(void (*processCreateSession)(Session*, const std::string),
                                     void (*processCloseSession)(Session*, const std::string),
                                     void (*processError)(Session*,
                                                          const uint8_t,
                                                          const std::string))
{
    m_sessionController = this;

    if(SessionHandler::m_sessionHandler == nullptr)
    {
        SessionHandler::m_sessionHandler = new SessionHandler(processCreateSession,
                                                              processCloseSession,
                                                              processError);
    }
}

/**
 * @brief destructor
 */
SessionController::~SessionController()
{
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
 * @param threadName base-name for server and client threads
 *
 * @return id of the new server if sussessful, else return 0
 */
uint32_t
SessionController::addUnixDomainServer(const std::string &socketFile,
                                       ErrorContainer &error,
                                       const std::string &threadName)
{
    Network::UnixDomainServer* server = new Network::UnixDomainServer(this,
                                                                      &processConnection_Callback,
                                                                      threadName);
    if(server->initServer(socketFile, error) == false) {
        return 0;
    }
    server->startThread();

    SessionHandler* sessionHandler = SessionHandler::m_sessionHandler;
    m_serverIdCounter++;
    sessionHandler->lockServerMap();
    sessionHandler->m_servers.insert(std::make_pair(m_serverIdCounter, server));
    sessionHandler->unlockServerMap();

    return m_serverIdCounter;
}

/**
 * @brief add new tcp-server
 *
 * @param port port where the server should listen
 * @param threadName base-name for server and client threads
 *
 * @return id of the new server if sussessful, else return 0
 */
uint32_t
SessionController::addTcpServer(const uint16_t port,
                                ErrorContainer &error,
                                const std::string &threadName)
{
    Network::TcpServer* server = new Network::TcpServer(this,
                                                        &processConnection_Callback,
                                                        threadName);
    if(server->initServer(port, error) == false) {
        return 0;
    }
    server->startThread();

    SessionHandler* sessionHandler = SessionHandler::m_sessionHandler;
    m_serverIdCounter++;
    sessionHandler->lockServerMap();
    sessionHandler->m_servers.insert(std::make_pair(m_serverIdCounter, server));
    sessionHandler->unlockServerMap();

    return m_serverIdCounter;
}

/**
 * @brief add new tls-encrypted tcp-server
 *
 * @param port port where the server should listen
 * @param certFile certificate-file for tls-encryption
 * @param keyFile key-file for tls-encryption
 * @param threadName base-name for server and client threads
 *
 * @return id of the new server if sussessful, else return 0
 */
uint32_t
SessionController::addTlsTcpServer(const uint16_t port,
                                   const std::string &certFile,
                                   const std::string &keyFile,
                                   ErrorContainer &error,
                                   const std::string &threadName)
{
    Network::TlsTcpServer* server = new Network::TlsTcpServer(this,
                                                              &processConnection_Callback,
                                                              threadName,
                                                              certFile,
                                                              keyFile);
    if(server->initServer(port, error) == false) {
        return 0;
    }
    server->startThread();

    SessionHandler* sessionHandler = SessionHandler::m_sessionHandler;
    m_serverIdCounter++;
    sessionHandler->lockServerMap();
    sessionHandler->m_servers.insert(std::pair<uint32_t, Network::AbstractServer*>(
                                     m_serverIdCounter, server));
    sessionHandler->unlockServerMap();

    return m_serverIdCounter;
}

/**
 * @brief close server
 *
 * @param id id of the server
 *
 * @return false, if id not exist or server can not be closed, else true
 */
bool
SessionController::closeServer(const uint32_t id)
{
    SessionHandler* sessionHandler = SessionHandler::m_sessionHandler;
    sessionHandler->lockServerMap();

    std::map<uint32_t, Network::AbstractServer*>::iterator it;
    it = sessionHandler->m_servers.find(id);

    if(it != sessionHandler->m_servers.end())
    {
        Network::AbstractServer* server = it->second;
        const bool ret = server->closeServer();
        if(ret == false) {
            return false;
        }

        server->scheduleThreadForDeletion();
        sessionHandler->m_servers.erase(it);
        sessionHandler->unlockServerMap();

        return true;
    }

    sessionHandler->unlockServerMap();

    return false;
}

/**
 * @brief SessionController::cloesAllServers
 */
void
SessionController::cloesAllServers()
{
    SessionHandler* sessionHandler = SessionHandler::m_sessionHandler;
    sessionHandler->lockServerMap();

    std::map<uint32_t, Network::AbstractServer*>::iterator it;
    for(it = sessionHandler->m_servers.begin();
        it != sessionHandler->m_servers.end();
        it++)
    {
        it->second->closeServer();
    }

    sessionHandler->unlockServerMap();
}

//==================================================================================================

/**
 * @brief start new unix-domain-socket
 *
 * @param socketFile socket-file-path, where the unix-domain-socket server is listening
 * @param sessionIdentifier additional identifier as help for an upper processing-layer
 *
 * @return true, if session was successfully created and connected, else false
 */
Session*
SessionController::startUnixDomainSession(const std::string &socketFile,
                                          const std::string &sessionIdentifier,
                                          const std::string &threadName,
                                          ErrorContainer &error)
{
    Network::UnixDomainSocket* unixDomainSocket = new Network::UnixDomainSocket(socketFile,
                                                                                threadName);
    return startSession(unixDomainSocket, sessionIdentifier, error);
}

/**
 * @brief start new tcp-session
 *
 * @param address ip-address of the server
 * @param port port where the server is listening
 * @param sessionIdentifier additional identifier as help for an upper processing-layer
 *
 * @return true, if session was successfully created and connected, else false
 */
Session*
SessionController::startTcpSession(const std::string &address,
                                   const uint16_t port,
                                   const std::string &sessionIdentifier,
                                   const std::string &threadName,
                                   ErrorContainer &error)
{
    Network::TcpSocket* tcpSocket = new Network::TcpSocket(address, port, threadName);
    return startSession(tcpSocket, sessionIdentifier, error);
}

/**
 * @brief start new tls-tcp-session
 *
 * @param address ip-address of the server
 * @param port port where the server is listening
 * @param certFile path to the certificate-file
 * @param keyFile path to the key-file
 * @param sessionIdentifier additional identifier as help for an upper processing-layer
 *
 * @return true, if session was successfully created and connected, else false
 */
Session*
SessionController::startTlsTcpSession(const std::string &address,
                                      const uint16_t port,
                                      const std::string &certFile,
                                      const std::string &keyFile,
                                      const std::string &sessionIdentifier,
                                      const std::string &threadName,
                                      ErrorContainer &error)
{
    Network::TlsTcpSocket* tlsTcpSocket = new Network::TlsTcpSocket(address,
                                                                    port,
                                                                    threadName,
                                                                    certFile,
                                                                    keyFile);
    return startSession(tlsTcpSocket, sessionIdentifier, error);
}

/**
 * @brief start a new session
 *
 * @param socket socket of the new session
 * @param sessionIdentifier additional identifier as help for an upper processing-layer
 *
 * @return true, if session was successfully created and connected, else false
 */
Session*
SessionController::startSession(Network::AbstractSocket* socket,
                                const std::string &sessionIdentifier,
                                ErrorContainer &error)
{
    // precheck
    if(sessionIdentifier.size() > 64)
    {
        delete socket;
        return nullptr;
    }

    // create new session
    Session* newSession = new Session(socket);
    const uint32_t newId = SessionHandler::m_sessionHandler->increaseSessionIdCounter();
    socket->setMessageCallback(newSession, &processMessage_callback);

    // connect session
    if(newSession->connectiSession(newId, error))
    {
        SessionHandler::m_sessionHandler->addSession(newId, newSession);
        send_Session_Init_Start(newSession, sessionIdentifier, error);

        std::unique_lock<std::mutex> lock(newSession->m_cvMutex);
        newSession->m_cv.wait(lock);

        return newSession;
    }

    newSession->closeSession(error);
    sleep(1);
    delete newSession;

    return nullptr;
}

//==================================================================================================

} // namespace Sakura
} // namespace Kitsunemimi
