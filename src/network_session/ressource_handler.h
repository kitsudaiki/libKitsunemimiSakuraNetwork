#ifndef RESSOURCE_HANDLER_H
#define RESSOURCE_HANDLER_H

#include <iostream>
#include <vector>
#include <map>
#include <atomic>

namespace Kitsune
{
namespace Network {
class AbstractServer;
}
namespace Project
{
namespace Common
{
class Session;

class RessourceHandler
{
public:
    RessourceHandler(void* dataTarget,
                     void (*processData)(void*, Session*, void*, const uint32_t),
                     void* errorTarget,
                     void (*processError)(void*, Session*, const uint8_t, const std::string));

    // (for internal usage)
    void addSession(const uint32_t id, Session* session);
    void confirmSession(const uint32_t id);
    Session* removeSession(const uint32_t id);

    bool isIdUsed(const uint32_t id);

    uint32_t increaseMessageIdCounter();
    uint16_t increaseSessionIdCounter();

    void* m_dataTarget = nullptr;
    void (*m_processData)(void*, Session*, void*, const uint32_t);
    void* m_errorTarget = nullptr;
    void (*m_processError)(void*, Session*, const uint8_t, const std::string);

    // object-holder
    std::map<uint32_t, Session*> m_sessions;
    std::map<uint32_t, Network::AbstractServer*> m_servers;

    uint32_t m_serverIdCounter = 0;

private:
    // counter
    std::atomic_flag m_messageIdCounter_lock = ATOMIC_FLAG_INIT;
    uint32_t m_messageIdCounter = 0;
    std::atomic_flag m_sessionIdCounter_lock = ATOMIC_FLAG_INIT;
    uint16_t m_sessionIdCounter = 0;
};

} // namespace Common
} // namespace Project
} // namespace Kitsune

#endif // RESSOURCE_HANDLER_H
