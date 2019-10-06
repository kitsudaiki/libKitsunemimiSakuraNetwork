/**
 *  @file       message_definitions.h
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
    DATA_TYPE = 4,
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

enum data_subTypes
{
    DATA_PLAIN_SUBTYPE = 1,
    DATA_MULTI_SUBTYPE = 2,
    DATA_REPLY_SUBTYPE = 3,
};

//==================================================================================================

struct CommonMessageHeader
{
    uint8_t version = 0x1;
    uint8_t type = 0;
    uint8_t subType = 0;
    uint8_t flags = 0;   // 0x1 = Reply required
    uint32_t messageId = 0;
    uint32_t sessionId = 0;
    uint32_t size = 0;
} __attribute__((packed));

struct CommonMessageEnd
{
    const uint32_t end = 1314472257;
} __attribute__((packed));

//==================================================================================================

struct Session_InitStart_Message
{
    CommonMessageHeader commonHeader;
    uint32_t offeredSessionId = 0;
    CommonMessageEnd commonEnd;

    Session_InitStart_Message() {
        commonHeader.type = SESSION_TYPE;
        commonHeader.subType = SESSION_INIT_START_SUBTYPE;
        commonHeader.flags = 0x1;
    }
} __attribute__((packed));

struct Session_IdChange_Message
{
    CommonMessageHeader commonHeader;
    uint32_t oldOfferedSessionId = 0;
    uint32_t newOfferedSessionId = 0;
    CommonMessageEnd commonEnd;

    Session_IdChange_Message() {
        commonHeader.type = SESSION_TYPE;
        commonHeader.subType = SESSION_ID_CHANGE_SUBTYPE;
        commonHeader.flags = 0x1;
    }
} __attribute__((packed));

struct Session_IdConfirm_Message
{
    CommonMessageHeader commonHeader;
    uint32_t confirmedSessionId = 0;
    CommonMessageEnd commonEnd;

    Session_IdConfirm_Message() {
        commonHeader.type = SESSION_TYPE;
        commonHeader.subType = SESSION_ID_CONFIRM_SUBTYPE;
        commonHeader.flags = 0x1;
    }
} __attribute__((packed));

struct Session_InitReply_Message
{
    CommonMessageHeader commonHeader;
    uint32_t sessionId = 0;
    CommonMessageEnd commonEnd;

    Session_InitReply_Message() {
        commonHeader.type = SESSION_TYPE;
        commonHeader.subType = SESSION_INIT_REPLY_SUBTYPE;
    }
} __attribute__((packed));

//==================================================================================================

struct Session_EndStart_Message
{
    CommonMessageHeader commonHeader;
    uint32_t sessionId = 0;
    CommonMessageEnd commonEnd;

    Session_EndStart_Message(bool replyExpected = false) {
        commonHeader.type = SESSION_TYPE;
        commonHeader.subType = SESSION_END_START_SUBTYPE;
        if(replyExpected) {
            commonHeader.flags = 0x1;
        }
    }
} __attribute__((packed));

struct Session_EndReply_Message
{
    CommonMessageHeader commonHeader;
    uint32_t sessionId = 0;
    CommonMessageEnd commonEnd;

    Session_EndReply_Message() {
        commonHeader.type = SESSION_TYPE;
        commonHeader.subType = SESSION_END_REPLY_SUBTYPE;
    }
} __attribute__((packed));

//==================================================================================================

} // namespace Common
} // namespace Project
} // namespace Kitsune

#endif // MESSAGE_HEADER_H
