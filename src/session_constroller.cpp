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

#include <libKitsunemimiPersistence/logger/logger.h>

namespace Kitsunemimi
{
namespace Sakura
{

SessionController* SessionController::m_sessionController = nullptr;

/**
 * @brief constructor
 */
SessionController::SessionController(void (*processSession)(bool, Session*, const std::string),
                                     void (*processError)(Session*,
                                                          const uint8_t,
                                                          const std::string))
{
    m_sessionController = this;

    if(SessionHandler::m_sessionHandler == nullptr) {
        SessionHandler::m_sessionHandler = new SessionHandler(processSession, processError);
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
 * @return id of the new server if sussessful, else return 0
 */
uint32_t
SessionController::addUnixDomainServer(const std::string &socketFile)
{
    Network::UnixDomainServer* server = new Network::UnixDomainServer(this,
                                                                      &processConnection_Callback);
    if(server->initServer(socketFile) == false) {
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
 *
 * @return id of the new server if sussessful, else return 0
 */
uint32_t
SessionController::addTcpServer(const uint16_t port)
{
    Network::TcpServer* server = new Network::TcpServer(this,
                                                        &processConnection_Callback);
    if(server->initServer(port) == false) {
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
 *
 * @return id of the new server if sussessful, else return 0
 */
uint32_t
SessionController::addTlsTcpServer(const uint16_t port,
                                   const std::string &certFile,
                                   const std::string &keyFile)
{
    Network::TlsTcpServer* server = new Network::TlsTcpServer(this,
                                                              &processConnection_Callback,
                                                              certFile,
                                                              keyFile);
    if(server->initServer(port) == false) {
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
                                          const std::string &sessionIdentifier)
{
    Network::UnixDomainSocket* unixDomainSocket = new Network::UnixDomainSocket(socketFile);
    return startSession(unixDomainSocket, sessionIdentifier);
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
                                   const std::string &sessionIdentifier)
{
    Network::TcpSocket* tcpSocket = new Network::TcpSocket(address, port);
    return startSession(tcpSocket, sessionIdentifier);
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
                                      const std::string &sessionIdentifier)
{
    Network::TlsTcpSocket* tlsTcpSocket = new Network::TlsTcpSocket(address,
                                                                    port,
                                                                    certFile,
                                                                    keyFile);
    return startSession(tlsTcpSocket, sessionIdentifier);
}

/**
 * @brief get a session by its id
 *
 * @param id id of the requested session
 *
 * @return pointer to the session, if found, else nullptr
 */
Session*
SessionController::getSession(const uint32_t id)
{
    SessionHandler* sessionHandler = SessionHandler::m_sessionHandler;
    sessionHandler->lockSessionMap();

    std::map<uint32_t, Session*>::iterator it;
    it = sessionHandler->m_sessions.find(id);

    if(it != sessionHandler->m_sessions.end())
    {
        Session* session = it->second;
        sessionHandler->unlockSessionMap();
        return session;
    }

    sessionHandler->unlockSessionMap();

    return nullptr;
}

/**
 * @brief close a specific session
 *
 * @param id id of the session, which should be closed
 *
 * @return true, if id was found and session-close was successful, else false
 */
bool
SessionController::closeSession(const uint32_t id)
{
    SessionHandler* sessionHandler = SessionHandler::m_sessionHandler;
    sessionHandler->lockSessionMap();

    std::map<uint32_t, Session*>::iterator it;
    it = SessionHandler::m_sessionHandler->m_sessions.find(id);

    if(it != SessionHandler::m_sessionHandler->m_sessions.end())
    {
        sessionHandler->unlockSessionMap();
        return it->second->closeSession(true);
    }

    sessionHandler->unlockSessionMap();

    return false;
}

/**
 * @brief close and remove all sessions
 */
void
SessionController::closeAllSession()
{
    SessionHandler* sessionHandler = SessionHandler::m_sessionHandler;

    // copy to avoid problems, when the close-process reduce the list, while there is a iteration
    // over the same list
    sessionHandler->lockSessionMap();
    std::map<uint32_t, Session*> copy = SessionHandler::m_sessionHandler->m_sessions;
    sessionHandler->unlockSessionMap();

    std::map<uint32_t, Session*>::iterator it;
    for(it = copy.begin();
        it != copy.end();
        it++)
    {
        it->second->closeSession();
    }

    SessionHandler::m_sessionHandler->m_sessions.clear();
}

/**
 * @brief link two sessions with each other
 *
 * @param session1 first session to link
 * @param session2 second session to link
 *
 * @return false, if a least one of the sessions is already linked, else true
 */
bool
SessionController::linkSessions(Session* session1, Session* session2)
{
    bool result = false;

    // lock both spin-locks
    while(session1->m_linkSession_lock.test_and_set(std::memory_order_acquire))  {
        asm("");
    }
    while(session2->m_linkSession_lock.test_and_set(std::memory_order_acquire))  {
        asm("");
    }

    // check and link sessions
    if(session1->m_linkedSession != nullptr
            || session2->m_linkedSession != nullptr)
    {
        session1->m_linkedSession = session2->m_linkedSession;
        session2->m_linkedSession = session1->m_linkedSession;
    }

    // releads spin-locks
    session2->m_linkSession_lock.clear(std::memory_order_release);
    session1->m_linkSession_lock.clear(std::memory_order_release);

    return result;
}

/**
 * @brief remove the link between two sessions
 *
 * @param session one of the sessions of the already existing link
 *
 * @return false, if session is not linked, else true
 */
bool
SessionController::unlinkSession(Session* session)
{
    Session* session2 = nullptr;

    while(session->m_linkSession_lock.test_and_set(std::memory_order_acquire))  {
        asm("");
    }

    if(session->m_linkedSession == nullptr)
    {
        session->m_linkSession_lock.clear(std::memory_order_release);
        return false;
    }

    session2 = session->m_linkedSession;

    while(session2->m_linkSession_lock.test_and_set(std::memory_order_acquire))  {
        asm("");
    }

    // unlink both sessions
    session->m_linkedSession = nullptr;
    session2->m_linkedSession = nullptr;

    // releads spin-locks
    session2->m_linkSession_lock.clear(std::memory_order_release);
    session->m_linkSession_lock.clear(std::memory_order_release);

    return true;
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
SessionController::startSession(Network::AbstractSocket *socket,
                                const std::string &sessionIdentifier)
{
    // precheck
    if(sessionIdentifier.size() > 64) {
        return nullptr;
    }

    // create new session
    Session* newSession = new Session(socket);
    const uint32_t newId = SessionHandler::m_sessionHandler->increaseSessionIdCounter();
    socket->setMessageCallback(newSession, &processMessage_callback);

    // connect session
    if(newSession->connectiSession(newId))
    {
        SessionHandler::m_sessionHandler->addSession(newId, newSession);
        send_Session_Init_Start(newSession, sessionIdentifier);

        std::unique_lock<std::mutex> lock(newSession->m_cvMutex);
        newSession->m_cv.wait(lock);

        return newSession;
    }
    else
    {
        newSession->closeSession();
        delete newSession;
    }

    return nullptr;
}

//==================================================================================================

} // namespace Sakura
} // namespace Kitsunemimi
