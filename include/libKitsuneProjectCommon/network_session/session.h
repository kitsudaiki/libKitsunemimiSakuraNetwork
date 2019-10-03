/**
 *  @file       session.h
 *
 *  @author     Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 *  @copyright  Apache License Version 2.0
 */

#ifndef SESSION_H
#define SESSION_H

#include <iostream>
#include <abstract_socket.h>

namespace Kitsune
{
namespace Project
{
namespace Common
{

struct Session
{
    uint32_t sessionId = 0;
    Network::AbstractSocket* socket = nullptr;

    Session() {}
    ~Session() {}
};

} // namespace Common
} // namespace Project
} // namespace Kitsune

#endif // SESSION_H
