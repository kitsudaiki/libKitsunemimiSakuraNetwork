#ifndef TEST_SESSION_H
#define TEST_SESSION_H

#include <iostream>
#include <stdint.h>
#include <unistd.h>
#include <chrono>

namespace Kitsunemimi {
namespace Project {
class SessionController;
class Session;
}
}

typedef std::chrono::microseconds  chronoMicroSec;
typedef std::chrono::nanoseconds  chronoNanoSec;
typedef std::chrono::seconds  chronoSec;
typedef std::chrono::high_resolution_clock::time_point chronoTimePoint;
typedef std::chrono::high_resolution_clock chronoClock;

class TestSession
{
public:
    TestSession(const std::string &address,
                const uint16_t port,
                const std::string &type);
    void sendLoop();

    bool m_isClient = false;
    bool m_isTcp = false;

    uint64_t m_size = 0;
    uint64_t m_sizeCounter = 0;
    uint8_t* m_dataBuffer = nullptr;

    Kitsunemimi::Project::SessionController* m_controller = nullptr;
    Kitsunemimi::Project::Session* m_clientSession = nullptr;
    Kitsunemimi::Project::Session* m_serverSession = nullptr;

    std::chrono::high_resolution_clock::time_point m_start;
    std::chrono::high_resolution_clock::time_point m_end;
};

#endif // TEST_SESSION_H
