/**
 *  @file       session_connection_trigger.h
 *
 *  @author     Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 *  @copyright  Apache License Version 2.0
 */

#ifndef SESSION_CONNECTION_TRIGGER_H
#define SESSION_CONNECTION_TRIGGER_H

#include <connection_trigger.h>

namespace Kitsune
{
namespace Project
{
namespace Common
{
class SessionHandler;

class SessionConnectionTrigger : public Network::ConnectionTrigger
{
public:
    SessionConnectionTrigger(SessionHandler* sessionHandler);
    ~SessionConnectionTrigger();

    void handleConnection(Network::AbstractSocket* connection);

private:
    SessionHandler* m_sessionHandler = nullptr;

};

} // namespace Common
} // namespace Project
} // namespace Kitsune

#endif // SESSION_CONNECTION_TRIGGER_H
