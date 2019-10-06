#ifndef CALLBACKS_H
#define CALLBACKS_H

#include <abstract_socket.h>
#include <message_ring_buffer.h>
#include <buffering/data_buffer.h>
#include <network_session/session_handler.h>
#include <network_session/messages/message_processing.h>

using Kitsune::Network::MessageRingBuffer;
using Kitsune::Network::AbstractSocket;
using Kitsune::Common::DataBuffer;

namespace Kitsune
{
namespace Project
{
namespace Common
{

/**
 * processMessageTcp-callback
 */
uint64_t processMessageTcp(void* target,
                           MessageRingBuffer* recvBuffer,
                           AbstractSocket* socket)
{
    return processMessage(target, recvBuffer, socket);
}

/**
 * processConnectionTcp-callback
 */
void processConnectionTcp(void* target,
                          AbstractSocket* socket)
{
    socket->setMessageCallback(target, &processMessageTcp);
    socket->start();
}

/**
 * processMessageTlsTcp-callback
 */
uint64_t processMessageTlsTcp(void* target,
                              MessageRingBuffer* recvBuffer,
                              AbstractSocket* socket)
{
    return processMessage(target, recvBuffer, socket);
}

/**
 * processConnectionTlsTcp-callback
 */
void processConnectionTlsTcp(void* target,
                             AbstractSocket* socket)
{
    socket->setMessageCallback(target, &processMessageTlsTcp);
    socket->start();
}

/**
 * processMessageUnixDomain-callback
 */
uint64_t processMessageUnixDomain(void* target,
                                  MessageRingBuffer* recvBuffer,
                                  AbstractSocket* socket)
{
    return processMessage(target, recvBuffer, socket);
}

/**
 * processConnectionUnixDomain-callback
 */
void processConnectionUnixDomain(void* target,
                                 AbstractSocket* socket)
{
    socket->setMessageCallback(target, &processMessageUnixDomain);
    socket->start();
}


} // namespace Common
} // namespace Project
} // namespace Kitsune

#endif // CALLBACKS_H
