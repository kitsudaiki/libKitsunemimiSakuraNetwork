#ifndef MESSAGE_HANDLER_H
#define MESSAGE_HANDLER_H

#include <abstract_socket.h>
#include <message_ring_buffer.h>
#include <network_session/session_handler.h>

using Kitsune::Network::MessageRingBuffer;
using Kitsune::Network::AbstractSocket;
using Kitsune::Common::DataBuffer;

namespace Kitsune
{
namespace Project
{
namespace Common
{

uint64_t handleMessage(SessionHandler* sessionHandler,
                       MessageRingBuffer* recvBuffer,
                       AbstractSocket* socket);

} // namespace Common
} // namespace Project
} // namespace Kitsune

#endif // MESSAGE_HANDLER_H
