#include "ressource_handler.h"

#include <libKitsuneProjectCommon/network_session/session.h>
#include <libKitsuneProjectCommon/network_session/session_handler.h>

#include <libKitsuneNetwork/tcp/tcp_server.h>
#include <libKitsuneNetwork/tcp/tcp_socket.h>
#include <libKitsuneNetwork/unix/unix_domain_server.h>
#include <libKitsuneNetwork/unix/unix_domain_socket.h>
#include <libKitsuneNetwork/tls_tcp/tls_tcp_server.h>
#include <libKitsuneNetwork/tls_tcp/tls_tcp_socket.h>
#include <libKitsuneNetwork/abstract_server.h>

#include <libKitsunePersistence/logger/logger.h>

namespace Kitsune
{
namespace Project
{
namespace Common
{

RessourceHandler::RessourceHandler(void* dataTarget,
                                   void (*processData)(void*, Session*,
                                                       void*, const uint32_t),
                                   void* errorTarget,
                                   void (*processError)(void*, Session*,
                                                        const uint8_t, const std::string))
{
    m_dataTarget = dataTarget;
    m_processData = processData;
    m_errorTarget = errorTarget;
    m_processError = processError;
}


/**
 * @brief SessionHandler::addSession
 * @param id
 * @param session
 */
void
RessourceHandler::addSession(const uint32_t id, Session* session)
{
    LOG_DEBUG("add session with id: " + std::to_string(id));
    m_sessions.insert(std::pair<uint32_t, Session*>(id, session));
}

/**
 * @brief SessionHandler::confirmSession
 * @param id
 */
void
RessourceHandler::confirmSession(const uint32_t id)
{
    LOG_DEBUG("confirm session with id: " + std::to_string(id));

    SessionHandler* sHandler = SessionHandler::m_sessionHandler;
    Session* session = sHandler->getSession(id);
    sHandler->m_processSession(sHandler->m_sessionTarget, session);
}

/**
 * @brief SessionHandler::removeSession
 * @param id
 */
Session*
RessourceHandler::removeSession(const uint32_t id)
{
    LOG_DEBUG("remove session with id: " + std::to_string(id));
    std::map<uint32_t, Session*>::iterator it;
    it = m_sessions.find(id);

    if(it != m_sessions.end())
    {
        Session* tempSession = it->second;
        m_sessions.erase(it);
        return tempSession;
    }

    return nullptr;
}

/**
 * @brief SessionHandler::increaseMessageIdCounter
 * @return
 */
uint32_t
RessourceHandler::increaseMessageIdCounter()
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
uint16_t
RessourceHandler::increaseSessionIdCounter()
{
    uint16_t tempId = 0;
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
