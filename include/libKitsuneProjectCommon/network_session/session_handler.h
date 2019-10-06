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
    SessionHandler();
    ~SessionHandler();

    static Kitsune::Project::Common::TimerThread* m_timerThread;

    // server
    uint32_t addUnixDomainServer(const std::string socketFile);
    uint32_t addTcpServer(const uint16_t port);
    uint32_t addTlsTcpServer(const uint16_t port,
                             const std::string certFile,
                             const std::string keyFile);
    bool closeServer(const uint32_t id);
    Network::AbstractServer* getServer(const uint32_t id);

    // session
    void addUnixDomainSession(const std::string socketFile);
    void addTcpSession(const std::string address,
                       const uint16_t port);
    void addTlsTcpSession(const std::string address,
                          const uint16_t port,
                          const std::string certFile,
                          const std::string keyFile);
    bool closeSession(const uint32_t id);
    Session getSession(const uint32_t id);
    bool isIdUsed(const uint32_t id);

    void addPendingSession(const uint32_t id, Session &session);
    Session removePendingSession(const uint32_t id);
    bool finishPendingSession(const uint32_t pendingId,
                              const uint32_t newId);

private:
    std::map<uint32_t, Session> m_pendingSessions;
    std::map<uint32_t, Session> m_sessions;
    std::map<uint32_t, Network::AbstractServer*> m_servers;

    uint32_t m_sessionIdCounter = 0;
    uint32_t m_serverIdCounter = 0;
};

} // namespace Common
} // namespace Project
} // namespace Kitsune

#endif // SESSION_HANDLER_H
