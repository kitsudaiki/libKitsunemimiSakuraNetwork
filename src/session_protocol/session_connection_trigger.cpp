/**
 *  @file       session_connection_trigger.cpp
 *
 *  @author     Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 *  @copyright  Apache License Version 2.0
 */

#include "session_connection_trigger.h"
#include <session_protocol/session_handler.h>

namespace Kitsune
{
namespace Project
{
namespace Common
{

/**
 * @brief SessionConnectionTrigger::SessionConnectionTrigger
 * @param sessionHandler
 */
SessionConnectionTrigger::SessionConnectionTrigger(SessionHandler* sessionHandler)
{
    m_sessionHandler = sessionHandler;
}

/**
 * @brief SessionConnectionTrigger::~SessionConnectionTrigger
 */
SessionConnectionTrigger::~SessionConnectionTrigger()
{

}

/**
 * @brief SessionConnectionTrigger::handleConnection
 * @param connection
 */
void
SessionConnectionTrigger::handleConnection(Network::AbstractSocket* connection)
{

}

} // namespace Common
} // namespace Project
} // namespace Kitsune
