/**
 *  @file       session_handler.h
 *
 *  @author     Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 *  @copyright  Apache License Version 2.0
 */

#ifndef SESSION_HANDLER_H
#define SESSION_HANDLER_H

#include <iostream>
#include <vector>
#include <map>
#include <atomic>

#include <network_session/session.h>

namespace Kitsune
{
namespace Network {
class AbstractServer;
}
namespace Project
{
namespace Common
{
class TimerThread;

class SessionHandler
{
public:
    SessionHandler(void* target,
                   void (*processSession)(void*, Session));
    ~SessionHandler();

    static Kitsune::Project::Common::TimerThread* m_timerThread;
    static Kitsune::Project::Common::SessionHandler* m_sessionHandler;

    // server
    uint32_t addUnixDomainServer(const std::string socketFile);
    uint32_t addTcpServer(const uint16_t port);
    uint32_t addTlsTcpServer(const uint16_t port,
                             const std::string certFile,
                             const std::string keyFile);
    bool closeServer(const uint32_t id);
    Network::AbstractServer* getServer(const uint32_t id);

    // session
    void startUnixDomainSession(const std::string socketFile);
    void startTcpSession(const std::string address,
                         const uint16_t port);
    void startTlsTcpSession(const std::string address,
                            const uint16_t port,
                            const std::string certFile,
                            const std::string keyFile);
    bool closeSession(const uint32_t id);
    Session getSession(const uint32_t id);
    bool isIdUsed(const uint32_t id);

    void addSession(const uint32_t id, Session &session);
    void addPendingSession(const uint32_t id, Session &session);
    Session removePendingSession(const uint32_t id);

    uint32_t increaseMessageIdCounter();
    uint32_t increaseSessionIdCounter();

private:
    std::map<uint32_t, Session> m_pendingSessions;
    std::map<uint32_t, Session> m_sessions;
    std::map<uint32_t, Network::AbstractServer*> m_servers;

    // callback-parameter
    void* m_target = nullptr;
    void (*m_processSession)(void*, Session);

    std::atomic_flag m_messageIdCounter_lock = ATOMIC_FLAG_INIT;
    uint32_t m_messageIdCounter = 0;

    std::atomic_flag m_sessionIdCounter_lock = ATOMIC_FLAG_INIT;
    uint32_t m_sessionIdCounter = 0;

    uint32_t m_serverIdCounter = 0;
};

} // namespace Common
} // namespace Project
} // namespace Kitsune

#endif // SESSION_HANDLER_H
