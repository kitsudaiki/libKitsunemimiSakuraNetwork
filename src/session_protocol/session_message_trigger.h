/**
 *  @file       session_message_trigger.h
 *
 *  @author     Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 *  @copyright  Apache License Version 2.0
 */

#ifndef SESSION_MESSAGE_TRIGGER_H
#define SESSION_MESSAGE_TRIGGER_H

#include <message_trigger.h>

namespace Kitsune
{
namespace Project
{
namespace Common
{

class SessionMessageTrigger : public Network::MessageTrigger
{
public:
    SessionMessageTrigger();
    ~SessionMessageTrigger();

    uint64_t runTask(Network::MessageRingBuffer& recvBuffer,
                     Network::AbstractSocket* socket);
};

} // namespace Common
} // namespace Project
} // namespace Kitsune

#endif // SESSION_MESSAGE_TRIGGER_H
