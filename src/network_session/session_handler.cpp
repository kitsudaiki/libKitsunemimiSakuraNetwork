/**
 *  @file       session_handler.cpp
 *
 *  @author     Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 *  @copyright  Apache License Version 2.0
 */

#include <network_session/session_handler.h>
#include <network_session/timer_thread.h>

#include <tcp/tcp_server.h>
#include <tcp/tcp_socket.h>
#include <unix/unix_domain_server.h>
#include <unix/unix_domain_socket.h>
#include <tls_tcp/tls_tcp_server.h>
#include <tls_tcp/tls_tcp_socket.h>
#include <abstract_server.h>

#include <network_session/callbacks.h>

#include <logger/logger.h>

namespace Kitsune
{
namespace Project
{
namespace Common
{

Kitsune::Project::Common::TimerThread* SessionHandler::m_timerThread = nullptr;
Kitsune::Project::Common::SessionHandler* SessionHandler::m_sessionHandler = nullptr;

/**
 * @brief Session::Session
 */
SessionHandler::SessionHandler(void* target,
                               void (*processSession)(void*, Session))
{
    m_target = target;
    m_processSession = processSession;

    if(m_timerThread == nullptr)
    {
        m_timerThread = new TimerThread;
        m_timerThread->start();
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

    m_serverIdCounter++;
    m_servers.insert(std::pair<uint32_t, Network::AbstractServer*>(m_serverIdCounter, server));

    return m_serverIdCounter;
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

    m_serverIdCounter++;
    m_servers.insert(std::pair<uint32_t, Network::AbstractServer*>(m_serverIdCounter, server));

    return m_serverIdCounter;
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

    m_serverIdCounter++;
    m_servers.insert(std::pair<uint32_t, Network::AbstractServer*>(m_serverIdCounter, server));

    return m_serverIdCounter;
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
    it = m_servers.find(id);

    if(it != m_servers.end())
    {
        Network::AbstractServer* server = it->second;
        server->closeServer();
        delete server;
        m_servers.erase(it);
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
    it = m_servers.find(id);

    if(it != m_servers.end()) {
        return it->second;
    }

    return nullptr;
}

/**
 * @brief SessionHandler::addUnixDomainSocket
 * @param socketFile
 */
void
SessionHandler::startUnixDomainSession(const std::string socketFile)
{
    Network::UnixDomainSocket* unixDomainSocket = new Network::UnixDomainSocket(socketFile);
    unixDomainSocket->initClientSide();
    unixDomainSocket->setMessageCallback(this, &processMessageUnixDomain);
    unixDomainSocket->start();

    Session newSession;
    newSession.socket = unixDomainSocket;
    newSession.sessionId = increaseSessionIdCounter();

    addPendingSession(m_sessionIdCounter, newSession);

    sendSessionInitStart(newSession.sessionId, newSession.socket);
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
    tcpSocket->initClientSide();
    tcpSocket->setMessageCallback(this, &processMessageTcp);
    tcpSocket->start();

    Session newSession;
    newSession.socket = tcpSocket;
    newSession.sessionId = increaseSessionIdCounter();

    addPendingSession(m_sessionIdCounter, newSession);

    sendSessionInitStart(newSession.sessionId, newSession.socket);
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
    tlsTcpSocket->initClientSide();
    tlsTcpSocket->setMessageCallback(this, &processMessageTlsTcp);
    tlsTcpSocket->start();

    Session newSession;
    newSession.socket = tlsTcpSocket;
    newSession.sessionId = increaseSessionIdCounter();

    addPendingSession(m_sessionIdCounter, newSession);

    sendSessionInitStart(newSession.sessionId, newSession.socket);
}

/**
 * @brief SessionHandler::closeSocket
 * @param id
 * @return
 */
bool
SessionHandler::closeSession(const uint32_t id)
{
    std::map<uint32_t, Session>::iterator it;
    it = m_sessions.find(id);

    if(it != m_sessions.end())
    {
        AbstractSocket* socket = it->second.socket;
        socket->stop();
        socket->closeSocket();
        delete socket;
        m_sessions.erase(it);
        return true;
    }

    return false;
}

/**
 * @brief SessionHandler::getSocket
 * @param id
 * @return
 */
Session
SessionHandler::getSession(const uint32_t id)
{
    std::map<uint32_t, Session>::iterator it;
    it = m_sessions.find(id);

    if(it != m_sessions.end()) {
        return it->second;
    }

    return Session();
}

/**
 * @brief SessionHandler::isIdUsed
 * @param id
 * @return
 */
bool
SessionHandler::isIdUsed(const uint32_t id)
{
    // TODO: avoid race-condition
    std::map<uint32_t, Session>::iterator it;
    it = m_sessions.find(id);

    if(it != m_sessions.end())
    {
        return true;
    }

    return false;
}

/**
 * @brief SessionHandler::addSession
 * @param id
 * @param session
 */
void
SessionHandler::addSession(const uint32_t id, Session &session)
{
    m_sessions.insert(std::pair<uint32_t, Session>(id, session));
    m_processSession(m_target, session);
}

/**
 * @brief SessionHandler::addPendingSession
 * @param id
 * @param session
 */
void
SessionHandler::addPendingSession(const uint32_t id, Session &session)
{
    m_pendingSessions.insert(std::pair<uint32_t, Session>(id, session));
}

/**
 * @brief SessionHandler::removePendingSession
 * @param id
 * @return
 */
Session
SessionHandler::removePendingSession(const uint32_t id)
{
    std::map<uint32_t, Session>::iterator it;
    it = m_pendingSessions.find(id);

    if(it != m_pendingSessions.end())
    {
        Session tempSession = it->second;
        m_pendingSessions.erase(it);
        return tempSession;
    }

    return Session();
}

/**
 * @brief SessionHandler::increaseMessageIdCounter
 * @return
 */
uint32_t
SessionHandler::increaseMessageIdCounter()
{
    uint32_t tempId = 0;
    while (m_messageIdCounter_lock.test_and_set(std::memory_order_acquire))  // acquire lock
                 ; // spin
    m_messageIdCounter++;
    tempId = m_messageIdCounter;
    m_messageIdCounter_lock.clear(std::memory_order_release);
    return tempId;
}

/**
 * @brief SessionHandler::increaseSessionIdCounter
 * @return
 */
uint32_t
SessionHandler::increaseSessionIdCounter()
{
    uint32_t tempId = 0;
    while (m_sessionIdCounter_lock.test_and_set(std::memory_order_acquire))  // acquire lock
                 ; // spin
    m_sessionIdCounter++;
    tempId = m_sessionIdCounter;
    m_sessionIdCounter_lock.clear(std::memory_order_release);
    return tempId;
}

} // namespace Common
} // namespace Project
} // namespace Kitsune
