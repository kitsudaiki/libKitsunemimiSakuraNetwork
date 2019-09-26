/**
 *  @file       timer_thread.h
 *
 *  @author     Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 *  @copyright  Apache License Version 2.0
 */

#ifndef TIMER_THREAD_H
#define TIMER_THREAD_H

#include <vector>
#include <iostream>

#include <threading/thread.h>

namespace Kitsune
{
namespace Project
{
namespace Common
{
class Session;

class TimerThread : public Kitsune::Common::Thread
{
public:
    TimerThread();

    void addMessage(const uint64_t messageId,
                    Session* session);
    bool removeMessage(const uint64_t messageId);
    void removeAllOfSession(const uint32_t sessionId);

protected:
    void run();

private:
    struct MessageTime
    {
        uint64_t messageId = 0;
        float timer = 0;
        Session* session = nullptr;
    };

    float m_timeoutValue = 0.0f;
    std::vector<MessageTime> m_messageList;

    bool removeMessageFromList(const uint64_t messageId);
};

} // namespace Common
} // namespace Project
} // namespace Kitsune

#endif // TIMER_THREAD_H
