#ifndef TEST_SESSION_H
#define TEST_SESSION_H

#include <iostream>
#include <stdint.h>
#include <unistd.h>

namespace Kitsunemimi {
namespace Project {
class SessionController;
class Session;
}
}

class TestSession
{
public:
    TestSession(const std::string &address,
                const uint16_t port);
    void sendLoop();

    bool m_isClient = false;

    Kitsunemimi::Project::SessionController* m_controller = nullptr;
    Kitsunemimi::Project::Session* m_session = nullptr;
};

#endif // TEST_SESSION_H
