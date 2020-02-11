#include "test_session.h"

#include <libKitsunemimiProjectNetwork/session.h>
#include <libKitsunemimiProjectNetwork/session_controller.h>

void dataCallback(void* target,
                  Kitsunemimi::Project::Session*,
                  const bool isStream,
                  const void* data,
                  const uint64_t dataSize)
{
    const char* message = static_cast<const char*>(data);
    const std::string stringMessage = std::string(message, dataSize);
    std::cout<<"message: "<<std::endl;
    std::cout<<stringMessage<<std::endl;
    std::cout<<""<<std::endl;
}

void errorCallback(void*,
                   Kitsunemimi::Project::Session*,
                   const uint8_t,
                   const std::string)
{
}

void sessionCallback(void* target,
                     bool isInit,
                     Kitsunemimi::Project::Session* session,
                     const uint64_t)
{
    TestSession* testClass = static_cast<TestSession*>(target);

    std::cout<<"session-callback for id: "<<session->m_sessionId<<"\n"<<std::endl;
    if(isInit)
    {
        std::cout<<"init session"<<std::endl;
        testClass->m_sessioin = session;
    }
    else
    {
        std::cout<<"end session"<<std::endl;
        testClass->m_sessioin = nullptr;
    }
}

TestSession::TestSession(const std::string &address,
                         const uint16_t port)
{
    m_controller = new Kitsunemimi::Project::SessionController(this, &sessionCallback,
                                                               this, &dataCallback,
                                                               this, &errorCallback);

    m_controller->addTcpServer(port);

    if(address != "") {
        m_controller->startTcpSession(address, port);
    }

}
