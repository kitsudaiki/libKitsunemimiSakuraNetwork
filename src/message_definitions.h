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

#include <handler/reply_handler.h>
#include <handler/message_blocker_handler.h>

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
    STREAM_DATA_TYPE = 4,
    SINGLEBLOCK_DATA_TYPE = 5,
    MULTIBLOCK_DATA_TYPE = 6,
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

enum stream_data_subTypes
{
    DATA_STREAM_STATIC_SUBTYPE = 1,
    DATA_STREAM_DYNAMIC_SUBTYPE = 2,
    DATA_STREAM_REPLY_SUBTYPE = 3,
};

enum singleblock_data_subTypes
{
    DATA_SINGLE_DATA_SUBTYPE = 1,
    DATA_SINGLE_REPLY_SUBTYPE = 2,
};

enum multiblock_data_subTypes
{
    DATA_MULTI_INIT_SUBTYPE = 1,
    DATA_MULTI_INIT_REPLY_SUBTYPE = 2,
    DATA_MULTI_STATIC_SUBTYPE = 3,
    DATA_MULTI_FINISH_SUBTYPE = 4,
    DATA_MULTI_ABORT_INIT_SUBTYPE = 5,
    DATA_MULTI_ABORT_REPLY_SUBTYPE = 6,
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
} __attribute__((packed));

inline void
create_Session_Init_Start_Message(Session_Init_Start_Message &target,
                                  const uint32_t sessionId,
                                  const uint32_t messageId,
                                  const uint64_t sessionIdentifier)
{
    target.commonHeader.type = SESSION_TYPE;
    target.commonHeader.subType = SESSION_INIT_START_SUBTYPE;
    target.commonHeader.sessionId = sessionId;
    target.commonHeader.messageId = messageId;
    target.commonHeader.flags = 0x1;
    target.commonHeader.size = sizeof(target);
    target.sessionIdentifier = sessionIdentifier;
}

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
} __attribute__((packed));

inline void
create_Session_Init_Reply_Message(Session_Init_Reply_Message &target,
                                  const uint32_t sessionId,
                                  const uint32_t messageId)
{
    target.commonHeader.type = SESSION_TYPE;
    target.commonHeader.subType = SESSION_INIT_REPLY_SUBTYPE;
    target.commonHeader.sessionId = sessionId;
    target.commonHeader.messageId = messageId;
    target.commonHeader.flags = 0x2;
    target.commonHeader.size = sizeof(target);
}

//==================================================================================================

/**
 * @brief Session_Close_Start_Message
 */
struct Session_Close_Start_Message
{
    CommonMessageHeader commonHeader;
    uint32_t sessionId = 0;
    CommonMessageEnd commonEnd;
} __attribute__((packed));

inline void
create_Session_Close_Start_Message(Session_Close_Start_Message &target,
                                   const uint32_t sessionId,
                                   const uint32_t messageId,
                                   const bool replyExpected = false)
{
    target.commonHeader.type = SESSION_TYPE;
    target.commonHeader.subType = SESSION_CLOSE_START_SUBTYPE;
    target.commonHeader.sessionId = sessionId;
    target.commonHeader.messageId = messageId;
    target.commonHeader.size = sizeof(target);

    if(replyExpected) {
        target.commonHeader.flags = 0x1;
    }
}

/**
 * @brief Session_Close_Reply_Message
 */
struct Session_Close_Reply_Message
{
    CommonMessageHeader commonHeader;
    uint32_t sessionId = 0;
    CommonMessageEnd commonEnd;
} __attribute__((packed));

inline void
create_Session_Close_Reply_Message(Session_Close_Reply_Message &target,
                                   const uint32_t sessionId,
                                   const uint32_t messageId)
{
    target.commonHeader.type = SESSION_TYPE;
    target.commonHeader.subType = SESSION_CLOSE_REPLY_SUBTYPE;
    target.commonHeader.sessionId = sessionId;
    target.commonHeader.messageId = messageId;
    target.commonHeader.flags = 0x2;
    target.commonHeader.size = sizeof(target);
}

//==================================================================================================

/**
 * @brief Heartbeat_Start_Message
 */
struct Heartbeat_Start_Message
{
    CommonMessageHeader commonHeader;
    uint8_t padding[4];
    CommonMessageEnd commonEnd;
} __attribute__((packed));

inline void
create_Heartbeat_Start_Message(Heartbeat_Start_Message &target,
                               const uint32_t sessionId,
                               const uint32_t messageId)
{
    target.commonHeader.type = HEARTBEAT_TYPE;
    target.commonHeader.subType = HEARTBEAT_START_SUBTYPE;
    target.commonHeader.sessionId = sessionId;
    target.commonHeader.messageId = messageId;
    target.commonHeader.flags = 0x1;
    target.commonHeader.size = sizeof(target);
}

/**
 * @brief Heartbeat_Reply_Message
 */
struct Heartbeat_Reply_Message
{
    CommonMessageHeader commonHeader;
    uint8_t padding[4];
    CommonMessageEnd commonEnd;
} __attribute__((packed));

inline void
create_Heartbeat_Reply_Message(Heartbeat_Reply_Message &target,
                               const uint32_t sessionId,
                               const uint32_t messageId)
{
    target.commonHeader.type = HEARTBEAT_TYPE;
    target.commonHeader.subType = HEARTBEAT_REPLY_SUBTYPE;
    target.commonHeader.sessionId = sessionId;
    target.commonHeader.messageId = messageId;
    target.commonHeader.flags = 0x2;
    target.commonHeader.size = sizeof(target);
}

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
} __attribute__((packed));

inline void
create_Error_FalseVersion_Message(Error_FalseVersion_Message &target,
                                  const uint32_t sessionId,
                                  const uint32_t messageId,
                                  const std::string &errorMessage)
{
    target.commonHeader.type = ERROR_TYPE;
    target.commonHeader.subType = ERROR_FALSE_VERSION_SUBTYPE;
    target.commonHeader.sessionId = sessionId;
    target.commonHeader.messageId = messageId;
    target.commonHeader.size = sizeof(target);

    target.messageSize = errorMessage.size();
    if(target.messageSize > 999) {
        target.messageSize = 999;
    }
    strncpy(target.message, errorMessage.c_str(), target.messageSize);
}

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
} __attribute__((packed));

inline void
create_Error_UnknownSession_Message(Error_UnknownSession_Message &target,
                                    const uint32_t sessionId,
                                    const uint32_t messageId,
                                    const std::string &errorMessage)
{
    target.commonHeader.type = ERROR_TYPE;
    target.commonHeader.subType = ERROR_UNKNOWN_SESSION_SUBTYPE;
    target.commonHeader.sessionId = sessionId;
    target.commonHeader.messageId = messageId;
    target.commonHeader.size = sizeof(target);

    target.messageSize = errorMessage.size();
    if(target.messageSize > 999) {
        target.messageSize = 999;
    }
    strncpy(target.message, errorMessage.c_str(), target.messageSize);
}

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
} __attribute__((packed));

inline void
create_Error_InvalidMessage_Message(Error_InvalidMessage_Message &target,
                                    const uint32_t sessionId,
                                    const uint32_t messageId,
                                    const std::string &errorMessage)
{
    target.commonHeader.type = ERROR_TYPE;
    target.commonHeader.subType = ERROR_INVALID_MESSAGE_SUBTYPE;
    target.commonHeader.sessionId = sessionId;
    target.commonHeader.messageId = messageId;
    target.commonHeader.size = sizeof(target);

    target.messageSize = errorMessage.size();
    if(target.messageSize > 999) {
        target.messageSize = 999;
    }
    strncpy(target.message, errorMessage.c_str(), target.messageSize);
}

//==================================================================================================

/**
 * @brief Data_StreamStatic_Message
 */
struct Data_StreamStatic_Message
{
    CommonMessageHeader commonHeader;
    uint64_t payloadSize = 0;
    uint8_t payload[1000];
    uint8_t padding[4];
    CommonMessageEnd commonEnd;
} __attribute__((packed));

inline void
create_Data_StreamStatic_Message(Data_StreamStatic_Message &target,
                                 const uint32_t sessionId,
                                 const uint32_t messageId,
                                 const bool replyExpected)
{
    target.commonHeader.type = STREAM_DATA_TYPE;
    target.commonHeader.subType = DATA_STREAM_STATIC_SUBTYPE;
    target.commonHeader.sessionId = sessionId;
    target.commonHeader.messageId = messageId;
    target.commonHeader.size = sizeof(target);
    if(replyExpected) {
        target.commonHeader.flags |= 0x1;
    }
}

/**
 * @brief Data_StreamDynamic_Header
 */
struct Data_StreamDynamic_Header
{
    CommonMessageHeader commonHeader;
    uint64_t payloadSize = 0;
} __attribute__((packed));

inline void
create_Data_StreamDynamic_Header(Data_StreamDynamic_Header &target,
                                 const uint32_t sessionId,
                                 const uint32_t messageId,
                                 const bool replyExpected)
{
    target.commonHeader.type = STREAM_DATA_TYPE;
    target.commonHeader.subType = DATA_STREAM_DYNAMIC_SUBTYPE;
    target.commonHeader.sessionId = sessionId;
    target.commonHeader.messageId = messageId;
    target.commonHeader.size = sizeof(target);
    if(replyExpected) {
        target.commonHeader.flags |= 0x1;
    }
}

/**
 * @brief Data_StreamReply_Message
 */
struct Data_StreamReply_Message
{
    CommonMessageHeader commonHeader;
    uint8_t padding[4];
    CommonMessageEnd commonEnd;
} __attribute__((packed));

inline void
create_Data_StreamReply_Message(Data_StreamReply_Message &target,
                                const uint32_t sessionId,
                                const uint32_t messageId)
{
    target.commonHeader.type = STREAM_DATA_TYPE;
    target.commonHeader.subType = DATA_STREAM_REPLY_SUBTYPE;
    target.commonHeader.sessionId = sessionId;
    target.commonHeader.messageId = messageId;
    target.commonHeader.flags = 0x2;
    target.commonHeader.size = sizeof(target);
}

//==================================================================================================

/**
 * @brief Data_SingleStatic_Message
 */
struct Data_SingleBlock_Message
{
    CommonMessageHeader commonHeader;
    uint64_t multiblockId = 0;
    uint64_t payloadSize = 0;
    uint8_t payload[1000];
    uint8_t padding[4];
    CommonMessageEnd commonEnd;
} __attribute__((packed));

inline void
create_Data_SingleBlock_Message(Data_SingleBlock_Message &target,
                                const uint32_t sessionId,
                                const uint32_t messageId,
                                const uint64_t multiblockId)
{
    target.commonHeader.type = SINGLEBLOCK_DATA_TYPE;
    target.commonHeader.subType = DATA_SINGLE_DATA_SUBTYPE;
    target.commonHeader.sessionId = sessionId;
    target.commonHeader.messageId = messageId;
    target.commonHeader.size = sizeof(target);
    target.commonHeader.flags = 0x1;
    target.multiblockId = multiblockId;
}

/**
 * @brief Data_SingleReply_Message
 */
struct Data_SingleBlockReply_Message
{
    CommonMessageHeader commonHeader;
    uint8_t padding[4];
    CommonMessageEnd commonEnd;
} __attribute__((packed));

inline void
create_Data_SingleBlockReply_Message(Data_SingleBlockReply_Message &target,
                                     const uint32_t sessionId,
                                     const uint32_t messageId)
{
    target.commonHeader.type = SINGLEBLOCK_DATA_TYPE;
    target.commonHeader.subType = DATA_SINGLE_REPLY_SUBTYPE;
    target.commonHeader.sessionId = sessionId;
    target.commonHeader.messageId = messageId;
    target.commonHeader.flags = 0x2;
    target.commonHeader.size = sizeof(target);
}

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
} __attribute__((packed));

inline void
create_Data_MultiInit_Message(Data_MultiInit_Message &target,
                              const uint32_t sessionId,
                              const uint32_t messageId,
                              const uint64_t multiblockId,
                              const bool answerExpected)
{
    target.commonHeader.type = MULTIBLOCK_DATA_TYPE;
    target.commonHeader.subType = DATA_MULTI_INIT_SUBTYPE;
    target.commonHeader.sessionId = sessionId;
    target.commonHeader.messageId = messageId;
    target.commonHeader.flags = 0x1;
    target.commonHeader.size = sizeof(target);
    if(answerExpected) {
        target.commonHeader.flags |= 0x4;
    }

    target.multiblockId = multiblockId;
}

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
} __attribute__((packed));

inline void
create_Data_MultiInitReply_Message(Data_MultiInitReply_Message &target,
                                   const uint32_t sessionId,
                                   const uint32_t messageId,
                                   const uint64_t multiblockId)
{
    target.commonHeader.type = MULTIBLOCK_DATA_TYPE;
    target.commonHeader.subType = DATA_MULTI_INIT_REPLY_SUBTYPE;
    target.commonHeader.sessionId = sessionId;
    target.commonHeader.messageId = messageId;
    target.commonHeader.flags = 0x2;
    target.commonHeader.size = sizeof(target);
    target.multiblockId = multiblockId;
}

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
} __attribute__((packed));

inline void
create_Data_MultiStatic_Message(Data_MultiStatic_Message &target,
                                const uint32_t sessionId,
                                const uint32_t messageId,
                                const uint64_t multiblockId)
{
    target.commonHeader.type = MULTIBLOCK_DATA_TYPE;
    target.commonHeader.subType = DATA_MULTI_STATIC_SUBTYPE;
    target.commonHeader.sessionId = sessionId;
    target.commonHeader.messageId = messageId;
    target.commonHeader.size = sizeof(target);
    target.multiblockId = multiblockId;
}

/**
 * @brief Data_MultiFinish_Message
 */
struct Data_MultiFinish_Message
{
    CommonMessageHeader commonHeader;
    uint64_t multiblockId = 0;
    uint64_t blockerId = 0;
    uint8_t padding[4];
    CommonMessageEnd commonEnd;
} __attribute__((packed));

inline void
create_Data_MultiFinish_Message(Data_MultiFinish_Message &target,
                                const uint32_t sessionId,
                                const uint32_t messageId,
                                const uint64_t multiblockId,
                                const uint64_t blockerId)
{
    target.commonHeader.type = MULTIBLOCK_DATA_TYPE;
    target.commonHeader.subType = DATA_MULTI_FINISH_SUBTYPE;
    target.commonHeader.sessionId = sessionId;
    target.commonHeader.messageId = messageId;
    target.commonHeader.size = sizeof(target);

    target.multiblockId = multiblockId;
    target.blockerId = blockerId;

    if(blockerId != 0) {
        target.commonHeader.flags |= 0x8;
    }
}

/**
 * @brief Data_MultiAbortInit_Message
 */
struct Data_MultiAbortInit_Message
{
    CommonMessageHeader commonHeader;
    uint64_t multiblockId = 0;
    uint8_t padding[4];
    CommonMessageEnd commonEnd;
} __attribute__((packed));

inline void
create_Data_MultiAbortInit_Message(Data_MultiAbortInit_Message &target,
                                   const uint32_t sessionId,
                                   const uint32_t messageId,
                                   const uint64_t multiblockId)
{
    target.commonHeader.type = MULTIBLOCK_DATA_TYPE;
    target.commonHeader.subType = DATA_MULTI_ABORT_INIT_SUBTYPE;
    target.commonHeader.sessionId = sessionId;
    target.commonHeader.messageId = messageId;
    target.commonHeader.size = sizeof(target);
    target.multiblockId = multiblockId;
}

/**
 * @brief Data_MultiAbortReply_Message
 */
struct Data_MultiAbortReply_Message
{
    CommonMessageHeader commonHeader;
    uint64_t multiblockId = 0;
    uint8_t padding[4];
    CommonMessageEnd commonEnd;
} __attribute__((packed));

inline void
create_Data_MultiAbortReply_Message(Data_MultiAbortReply_Message &target,
                                    const uint32_t sessionId,
                                    const uint32_t messageId,
                                    const uint64_t multiblockId)
{
    target.commonHeader.type = MULTIBLOCK_DATA_TYPE;
    target.commonHeader.subType = DATA_MULTI_ABORT_REPLY_SUBTYPE;
    target.commonHeader.sessionId = sessionId;
    target.commonHeader.messageId = messageId;
    target.commonHeader.size = sizeof(target);
    target.multiblockId = multiblockId;
}
//==================================================================================================

} // namespace Project
} // namespace Kitsunemimi

#endif // MESSAGE_DEFINITIONS_H
