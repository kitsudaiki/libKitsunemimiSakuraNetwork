/**
 *  @file       session_message_trigger.h
 *
 *  @author     Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 *  @copyright  Apache License Version 2.0
 */

#include "session_message_trigger.h"

namespace Kitsune
{
namespace Project
{
namespace Common
{

SessionMessageTrigger::SessionMessageTrigger()
{

}

SessionMessageTrigger::~SessionMessageTrigger()
{

}

uint64_t
SessionMessageTrigger::runTask(Network::MessageRingBuffer &recvBuffer,
                               Network::AbstractSocket* socket)
{
    return 0;
}

} // namespace Common
} // namespace Project
} // namespace Kitsune
