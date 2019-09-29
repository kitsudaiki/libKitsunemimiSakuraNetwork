/**
 *  @file       messages.h
 *
 *  @author     Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 *  @copyright  Apache License Version 2.0
 */

#ifndef MESSAGE_HEADER_H
#define MESSAGE_HEADER_H

#include <stdint.h>

namespace Kitsune
{
namespace Project
{
namespace Common
{

enum types
{
    UNDEFINED_TYPE = 0,
    SESSION_TYPE = 1,
    HEARTBEAT_TYPE = 2,
    ERROR_TYPE = 3,
};

enum session_subTypes
{
    SESSION_INIT_START_SUBTYPE = 1,
    SESSION_ID_CHANGE_SUBTYPE = 2,
    SESSION_ID_CONFIRM_SUBTYPE = 3,
    SESSION_INIT_REPLY_SUBTYPE = 4,

    SESSION_END_START_SUBTYPE = 5,
    SESSION_END_REPLY_SUBTYPE = 6,
};

enum heartbeat_subTypes
{
    HEARTBEAT_START_SUBTYPE = 1,
    HEARTBEAT_REPLY_SUBTYPE = 2,
};

enum error_subTypes
{
    ERROR_FALSE_VERSION_SUBTYPE = 1,
    ERROR_TIMEOUT_SUBTYPE = 2,
};

struct CommonMessageHeader
{
    uint8_t type = 0;
    uint8_t subType = 0;
    uint8_t version = 0;
    uint8_t flags = 0;   // 0x1 = Reply required
    uint32_t messageId = 0;
    uint32_t sessionId = 0;
    uint32_t size = 0;
} __attribute__((packed));

struct CommonMessageEnd
{
    const uint32_t end = 1314472257;
} __attribute__((packed));

} // namespace Common
} // namespace Project
} // namespace Kitsune

#endif // MESSAGE_HEADER_H
