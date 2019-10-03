/**
 *  @file       session_handler.cpp
 *
 *  @author     Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 *  @copyright  Apache License Version 2.0
 */

#include <network_session/session_handler.h>
#include <network_session/timer_thread.h>

namespace Kitsune
{
namespace Project
{
namespace Common
{

Kitsune::Project::Common::TimerThread* SessionHandler::m_timerThread = nullptr;

/**
 * @brief Session::Session
 */
SessionHandler::SessionHandler()
{
    if(m_timerThread == nullptr)
    {
        m_timerThread = new TimerThread;
        m_timerThread->start();
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

}

/**
 * @brief SessionHandler::addTcpServer
 * @param port
 * @return
 */
uint32_t
SessionHandler::addTcpServer(const uint16_t port)
{

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

}

/**
 * @brief SessionHandler::closeServer
 * @param id
 * @return
 */
bool
SessionHandler::closeServer(const uint32_t id)
{

}

/**
 * @brief SessionHandler::getServer
 * @param id
 * @return
 */
Network::AbstractServer*
SessionHandler::getServer(const uint32_t id)
{

}

/**
 * @brief SessionHandler::addUnixDomainSocket
 * @param socketFile
 * @return
 */
uint32_t
SessionHandler::addUnixDomainSession(const std::string socketFile)
{

}

/**
 * @brief SessionHandler::addTcpClient
 * @param address
 * @param port
 * @return
 */
uint32_t
SessionHandler::addTcpSession(const std::string address,
                              const uint16_t port)
{

}

/**
 * @brief SessionHandler::addTlsTcpClient
 * @param address
 * @param port
 * @param certFile
 * @param keyFile
 * @return
 */
uint32_t
SessionHandler::addTlsTcpSession(const std::string address,
                                 const uint16_t port,
                                 const std::string certFile,
                                 const std::string keyFile)
{

}

/**
 * @brief SessionHandler::closeSocket
 * @param id
 * @return
 */
bool
SessionHandler::closeSession(const uint32_t id)
{

}

/**
 * @brief SessionHandler::getSocket
 * @param id
 * @return
 */
Session SessionHandler::getSession(const uint32_t id)
{

}

/**
 * @brief SessionHandler::isIdUsed
 * @param id
 * @return
 */
bool
SessionHandler::isIdUsed(const uint32_t id)
{

}

/**
 * @brief SessionHandler::addPendingSession
 * @param session
 */
void SessionHandler::addPendingSession(Session &session)
{

}

/**
 * @brief SessionHandler::removePendingSession
 * @param id
 * @return
 */
bool
SessionHandler::removePendingSession(const uint32_t id)
{

}

/**
 * @brief SessionHandler::finishPendingSession
 * @param pendingId
 * @param newId
 * @return
 */
bool
SessionHandler::finishPendingSession(const uint32_t pendingId,
                                     const uint32_t newId)
{

}

} // namespace Common
} // namespace Project
} // namespace Kitsune
