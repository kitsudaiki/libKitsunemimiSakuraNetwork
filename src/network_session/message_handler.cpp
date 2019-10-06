#include "message_handler.h"

namespace Kitsune
{
namespace Project
{
namespace Common
{

uint64_t handleMessage(SessionHandler* sessionHandler,
                       Network::MessageRingBuffer *recvBuffer,
                       Network::AbstractSocket *socket)
{
    const uint8_t* dataPointer = getDataPointer(*recvBuffer, recvBuffer->readWriteDiff);

    if(dataPointer == nullptr) {
        return 0;
    }

    return recvBuffer->readWriteDiff;
}


} // namespace Common
} // namespace Project
} // namespace Kitsune
