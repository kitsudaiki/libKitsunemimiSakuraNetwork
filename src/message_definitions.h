/**
 * @file       message_definitions.h
 *
 * @author     Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 * @copyright  Apache License Version 2.0
 *
 *      Copyright 2019 Tobias Anker
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *          http://www.apache.org/licenses/LICENSE-2.0
 *
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 */

#ifndef MESSAGE_DEFINITIONS_H
#define MESSAGE_DEFINITIONS_H

#define DEBUG_MODE false

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include <reply_handler.h>
#include <answer_handler.h>

namespace Kitsunemimi
{
namespace Project
{

enum types
{
    UNDEFINED_TYPE = 0,
    SESSION_TYPE = 1,
    HEARTBEAT_TYPE = 2,
    ERROR_TYPE = 3,
    SINGLEBLOCK_DATA_TYPE = 4,
    MULTIBLOCK_DATA_TYPE = 5,
};

enum session_subTypes
{
    SESSION_INIT_START_SUBTYPE = 1,
    SESSION_INIT_REPLY_SUBTYPE = 2,

    SESSION_CLOSE_START_SUBTYPE = 3,
    SESSION_CLOSE_REPLY_SUBTYPE = 4,
};

enum heartbeat_subTypes
{
    HEARTBEAT_START_SUBTYPE = 1,
    HEARTBEAT_REPLY_SUBTYPE = 2,
};

enum error_subTypes
{
    ERROR_FALSE_VERSION_SUBTYPE = 1,
    ERROR_UNKNOWN_SESSION_SUBTYPE = 2,
    ERROR_INVALID_MESSAGE_SUBTYPE = 3,
};

enum singleblock_data_subTypes
{
    DATA_SINGLE_STATIC_SUBTYPE = 1,
    DATA_SINGLE_DYNAMIC_SUBTYPE = 2,
    DATA_SINGLE_REPLY_SUBTYPE = 3,
};

enum multiblock_data_subTypes
{
    DATA_MULTI_INIT_SUBTYPE = 4,
    DATA_MULTI_INIT_REPLY_SUBTYPE = 5,
    DATA_MULTI_STATIC_SUBTYPE = 6,
    DATA_MULTI_FINISH_SUBTYPE = 7,
    DATA_MULTI_ABORT_INIT_SUBTYPE = 8,
    DATA_MULTI_ABORT_REPLY_SUBTYPE = 9,
};

//==================================================================================================

/**
 * @brief CommonMessageHeader
 */
struct CommonMessageHeader
{
    uint8_t version = 0x1;
    uint8_t type = 0;
    uint8_t subType = 0;
    uint8_t flags = 0;   // 0x1 = reply required; 0x2 = is reply; // 0x4 = answer required; 0x8 = is answer
    uint32_t messageId = 0;
    uint32_t sessionId = 0;
    uint32_t size = 0;
} __attribute__((packed));

struct CommonMessageEnd
{
    const uint32_t end = 1314472257;
} __attribute__((packed));

//==================================================================================================

/**
 * @brief Session_Init_Start_Message
 */
struct Session_Init_Start_Message
{
    CommonMessageHeader commonHeader;
    uint32_t clientSessionId = 0;
    uint64_t sessionIdentifier = 0;
    CommonMessageEnd commonEnd;

    Session_Init_Start_Message(const uint32_t sessionId,
                               const uint32_t messageId,
                               const uint64_t sessionIdentifier)
    {
        commonHeader.type = SESSION_TYPE;
        commonHeader.subType = SESSION_INIT_START_SUBTYPE;
        commonHeader.sessionId = sessionId;
        commonHeader.messageId = messageId;
        commonHeader.flags = 0x1;
        commonHeader.size = sizeof(*this);
        this->sessionIdentifier = sessionIdentifier;
    }
} __attribute__((packed));

/**
 * @brief Session_Init_Reply_Message
 */
struct Session_Init_Reply_Message
{
    CommonMessageHeader commonHeader;
    uint32_t clientSessionId = 0;
    uint32_t completeSessionId = 0;
    uint8_t padding[4];
    CommonMessageEnd commonEnd;

    Session_Init_Reply_Message(const uint32_t sessionId,
                               const uint32_t messageId)
    {
        commonHeader.type = SESSION_TYPE;
        commonHeader.subType = SESSION_INIT_REPLY_SUBTYPE;
        commonHeader.sessionId = sessionId;
        commonHeader.messageId = messageId;
        commonHeader.flags = 0x2;
        commonHeader.size = sizeof(*this);
    }
} __attribute__((packed));

//==================================================================================================

/**
 * @brief Session_Close_Start_Message
 */
struct Session_Close_Start_Message
{
    CommonMessageHeader commonHeader;
    uint32_t sessionId = 0;
    CommonMessageEnd commonEnd;

    Session_Close_Start_Message(const uint32_t sessionId,
                                const uint32_t messageId,
                                const bool replyExpected = false)
    {
        commonHeader.type = SESSION_TYPE;
        commonHeader.subType = SESSION_CLOSE_START_SUBTYPE;
        commonHeader.sessionId = sessionId;
        commonHeader.messageId = messageId;
        commonHeader.size = sizeof(*this);

        if(replyExpected) {
            commonHeader.flags = 0x1;
        }
    }
} __attribute__((packed));

/**
 * @brief Session_Close_Reply_Message
 */
struct Session_Close_Reply_Message
{
    CommonMessageHeader commonHeader;
    uint32_t sessionId = 0;
    CommonMessageEnd commonEnd;

    Session_Close_Reply_Message(const uint32_t sessionId,
                                const uint32_t messageId)
    {
        commonHeader.type = SESSION_TYPE;
        commonHeader.subType = SESSION_CLOSE_REPLY_SUBTYPE;
        commonHeader.sessionId = sessionId;
        commonHeader.messageId = messageId;
        commonHeader.flags = 0x2;
        commonHeader.size = sizeof(*this);
    }
} __attribute__((packed));

//==================================================================================================

/**
 * @brief Heartbeat_Start_Message
 */
struct Heartbeat_Start_Message
{
    CommonMessageHeader commonHeader;
    uint8_t padding[4];
    CommonMessageEnd commonEnd;

    Heartbeat_Start_Message(const uint32_t sessionId,
                            const uint32_t messageId)
    {
        commonHeader.type = HEARTBEAT_TYPE;
        commonHeader.subType = HEARTBEAT_START_SUBTYPE;
        commonHeader.sessionId = sessionId;
        commonHeader.messageId = messageId;
        commonHeader.flags = 0x1;
        commonHeader.size = sizeof(*this);
    }
} __attribute__((packed));

/**
 * @brief Heartbeat_Reply_Message
 */
struct Heartbeat_Reply_Message
{
    CommonMessageHeader commonHeader;
    uint8_t padding[4];
    CommonMessageEnd commonEnd;

    Heartbeat_Reply_Message(const uint32_t sessionId,
                            const uint32_t messageId)
    {
        commonHeader.type = HEARTBEAT_TYPE;
        commonHeader.subType = HEARTBEAT_REPLY_SUBTYPE;
        commonHeader.sessionId = sessionId;
        commonHeader.messageId = messageId;
        commonHeader.flags = 0x2;
        commonHeader.size = sizeof(*this);
    }
} __attribute__((packed));

//==================================================================================================

/**
 * @brief Error_FalseVersion_Message
 */
struct Error_FalseVersion_Message
{
    CommonMessageHeader commonHeader;
    uint64_t messageSize = 0;
    char message[1000];
    uint8_t padding[4];
    CommonMessageEnd commonEnd;

    Error_FalseVersion_Message(const uint32_t sessionId,
                               const uint32_t messageId,
                               const std::string &errorMessage)
    {
        commonHeader.type = ERROR_TYPE;
        commonHeader.subType = ERROR_FALSE_VERSION_SUBTYPE;
        commonHeader.sessionId = sessionId;
        commonHeader.messageId = messageId;
        commonHeader.size = sizeof(*this);

        messageSize = errorMessage.size();
        if(messageSize > 999) {
            messageSize = 999;
        }
        strncpy(message, errorMessage.c_str(), messageSize);
    }
} __attribute__((packed));

/**
 * @brief Error_UnknownSession_Message
 */
struct Error_UnknownSession_Message
{
    CommonMessageHeader commonHeader;
    uint64_t messageSize = 0;
    char message[1000];
    uint8_t padding[4];
    CommonMessageEnd commonEnd;

    Error_UnknownSession_Message(const uint32_t sessionId,
                                 const uint32_t messageId,
                                 const std::string &errorMessage)
    {
        commonHeader.type = ERROR_TYPE;
        commonHeader.subType = ERROR_UNKNOWN_SESSION_SUBTYPE;
        commonHeader.sessionId = sessionId;
        commonHeader.messageId = messageId;
        commonHeader.size = sizeof(*this);

        messageSize = errorMessage.size();
        if(messageSize > 999) {
            messageSize = 999;
        }
        strncpy(message, errorMessage.c_str(), messageSize);
    }
} __attribute__((packed));

/**
 * @brief Error_InvalidMessage_Message
 */
struct Error_InvalidMessage_Message
{
    CommonMessageHeader commonHeader;
    uint64_t messageSize = 0;
    char message[1000];
    uint8_t padding[4];
    CommonMessageEnd commonEnd;

    Error_InvalidMessage_Message(const uint32_t sessionId,
                                 const uint32_t messageId,
                                 const std::string &errorMessage)
    {
        commonHeader.type = ERROR_TYPE;
        commonHeader.subType = ERROR_INVALID_MESSAGE_SUBTYPE;
        commonHeader.sessionId = sessionId;
        commonHeader.messageId = messageId;
        commonHeader.size = sizeof(*this);

        messageSize = errorMessage.size();
        if(messageSize > 999) {
            messageSize = 999;
        }
        strncpy(message, errorMessage.c_str(), messageSize);
    }
} __attribute__((packed));

//==================================================================================================

/**
 * @brief Data_SingleStatic_Message
 */
struct Data_SingleStatic_Message
{
    CommonMessageHeader commonHeader;
    uint64_t payloadSize = 0;
    uint8_t payload[1000];
    uint8_t padding[4];
    CommonMessageEnd commonEnd;

    Data_SingleStatic_Message(const uint32_t sessionId,
                              const uint32_t messageId,
                              const bool replyExpected)
    {
        commonHeader.type = SINGLEBLOCK_DATA_TYPE;
        commonHeader.subType = DATA_SINGLE_STATIC_SUBTYPE;
        commonHeader.sessionId = sessionId;
        commonHeader.messageId = messageId;
        commonHeader.size = sizeof(*this);
        if(replyExpected) {
            commonHeader.flags |= 0x1;
        }
    }
} __attribute__((packed));

/**
 * @brief Data_SingleDynamic_Header
 */
struct Data_SingleDynamic_Header
{
    CommonMessageHeader commonHeader;
    uint64_t payloadSize = 0;

    Data_SingleDynamic_Header(const uint32_t sessionId,
                              const uint32_t messageId,
                              const bool replyExpected)
    {
        commonHeader.type = SINGLEBLOCK_DATA_TYPE;
        commonHeader.subType = DATA_SINGLE_DYNAMIC_SUBTYPE;
        commonHeader.sessionId = sessionId;
        commonHeader.messageId = messageId;
        commonHeader.size = sizeof(*this);
        if(replyExpected) {
            commonHeader.flags |= 0x1;
        }
    }
} __attribute__((packed));

/**
 * @brief Data_SingleReply_Message
 */
struct Data_SingleReply_Message
{
    CommonMessageHeader commonHeader;
    uint8_t padding[4];
    CommonMessageEnd commonEnd;

    Data_SingleReply_Message(const uint32_t sessionId,
                             const uint32_t messageId)
    {
        commonHeader.type = SINGLEBLOCK_DATA_TYPE;
        commonHeader.subType = DATA_SINGLE_REPLY_SUBTYPE;
        commonHeader.sessionId = sessionId;
        commonHeader.messageId = messageId;
        commonHeader.flags = 0x2;
        commonHeader.size = sizeof(*this);
    }
} __attribute__((packed));

//==================================================================================================

/**
 * @brief Data_MultiInit_Message
 */
struct Data_MultiInit_Message
{
    CommonMessageHeader commonHeader;
    uint64_t multiblockId = 0;
    uint64_t totalSize = 0;
    uint8_t padding[4];
    CommonMessageEnd commonEnd;

    Data_MultiInit_Message(const uint32_t sessionId,
                           const uint32_t messageId,
                           const uint64_t multiblockId,
                           const bool answerExpected,
                           const bool isAnswer)
    {
        commonHeader.type = MULTIBLOCK_DATA_TYPE;
        commonHeader.subType = DATA_MULTI_INIT_SUBTYPE;
        commonHeader.sessionId = sessionId;
        commonHeader.messageId = messageId;
        commonHeader.flags = 0x1;
        if(answerExpected) {
            commonHeader.flags |= 0x4;
        }
        if(isAnswer) {
            commonHeader.flags |= 0x8;
        }
        commonHeader.size = sizeof(*this);
        this->multiblockId = multiblockId;
    }
} __attribute__((packed));

/**
 * @brief Data_MultiInitReply_Message
 */
struct Data_MultiInitReply_Message
{
    enum stati {
        UNDEFINED = 0,
        OK = 1,
        FAIL = 2
    };

    CommonMessageHeader commonHeader;
    uint64_t multiblockId = 0;
    uint8_t status = UNDEFINED;
    uint8_t padding[3];
    CommonMessageEnd commonEnd;

    Data_MultiInitReply_Message(const uint32_t sessionId,
                                const uint32_t messageId,
                                const uint64_t multiblockId)
    {
        commonHeader.type = MULTIBLOCK_DATA_TYPE;
        commonHeader.subType = DATA_MULTI_INIT_REPLY_SUBTYPE;
        commonHeader.sessionId = sessionId;
        commonHeader.messageId = messageId;
        commonHeader.flags = 0x2;
        commonHeader.size = sizeof(*this);
        this->multiblockId = multiblockId;
    }
} __attribute__((packed));

/**
 * @brief Data_MultiStatic_Message
 */
struct Data_MultiStatic_Message
{
    CommonMessageHeader commonHeader;
    uint64_t multiblockId = 0;
    uint32_t totalPartNumber = 0;
    uint32_t partId = 0;
    uint64_t payloadSize = 0;
    uint8_t payload[1004];
    CommonMessageEnd commonEnd;

    Data_MultiStatic_Message(const uint32_t sessionId,
                             const uint32_t messageId,
                             const uint64_t multiblockId)
    {
        commonHeader.type = MULTIBLOCK_DATA_TYPE;
        commonHeader.subType = DATA_MULTI_STATIC_SUBTYPE;
        commonHeader.sessionId = sessionId;
        commonHeader.messageId = messageId;
        commonHeader.size = sizeof(*this);
        this->multiblockId = multiblockId;
    }
} __attribute__((packed));

/**
 * @brief Data_MultiFinish_Message
 */
struct Data_MultiFinish_Message
{
    CommonMessageHeader commonHeader;
    uint64_t multiblockId = 0;
    uint8_t padding[4];
    CommonMessageEnd commonEnd;

    Data_MultiFinish_Message(const uint32_t sessionId,
                             const uint32_t messageId,
                             const uint64_t multiblockId)
    {
        commonHeader.type = MULTIBLOCK_DATA_TYPE;
        commonHeader.subType = DATA_MULTI_FINISH_SUBTYPE;
        commonHeader.sessionId = sessionId;
        commonHeader.messageId = messageId;
        commonHeader.size = sizeof(*this);
        this->multiblockId = multiblockId;
    }
} __attribute__((packed));

/**
 * @brief Data_MultiAbortInit_Message
 */
struct Data_MultiAbortInit_Message
{
    CommonMessageHeader commonHeader;
    uint64_t multiblockId = 0;
    uint8_t padding[4];
    CommonMessageEnd commonEnd;

    Data_MultiAbortInit_Message(const uint32_t sessionId,
                                const uint32_t messageId,
                                const uint64_t multiblockId)
    {
        commonHeader.type = MULTIBLOCK_DATA_TYPE;
        commonHeader.subType = DATA_MULTI_ABORT_INIT_SUBTYPE;
        commonHeader.sessionId = sessionId;
        commonHeader.messageId = messageId;
        commonHeader.size = sizeof(*this);
        this->multiblockId = multiblockId;
    }
} __attribute__((packed));

/**
 * @brief Data_MultiAbortReply_Message
 */
struct Data_MultiAbortReply_Message
{
    CommonMessageHeader commonHeader;
    uint64_t multiblockId = 0;
    uint8_t padding[4];
    CommonMessageEnd commonEnd;

    Data_MultiAbortReply_Message(const uint32_t sessionId,
                                 const uint32_t messageId,
                                 const uint64_t multiblockId)
    {
        commonHeader.type = MULTIBLOCK_DATA_TYPE;
        commonHeader.subType = DATA_MULTI_ABORT_REPLY_SUBTYPE;
        commonHeader.sessionId = sessionId;
        commonHeader.messageId = messageId;
        commonHeader.size = sizeof(*this);
        this->multiblockId = multiblockId;
    }
} __attribute__((packed));

//==================================================================================================

} // namespace Project
} // namespace Kitsunemimi

#endif // MESSAGE_DEFINITIONS_H
