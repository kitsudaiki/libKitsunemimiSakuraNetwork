#include "test_session.h"

#include <libKitsunemimiProjectNetwork/session.h>
#include <libKitsunemimiProjectNetwork/session_controller.h>
#include <libKitsunemimiCommon/common_methods/string_methods.h>
#include <libKitsunemimiCommon/data_buffer.h>

/**
 * @brief standaloneDataCallback
 * @param target
 * @param data
 * @param dataSize
 */
void streamDataCallback(void* target,
                        Kitsunemimi::Project::Session* session,
                        const void* data,
                        const uint64_t dataSize)
{
    const char* message = static_cast<const char*>(data);
    const std::string stringMessage = std::string(message, dataSize);
    if(session->isClientSide() == false)
    {
        std::cout<<"+++++++++++++++++++++++++++++++++++++++++++++++++"<<std::endl;
        std::cout<<"message: "<<std::endl;
        std::cout<<stringMessage<<std::endl;
        std::cout<<""<<std::endl;
        std::cout<<"-------------------------------------------------"<<std::endl;
    }
}

/**
 * @brief standaloneDataCallback
 * @param target
 * @param isStream
 * @param data
 * @param dataSize
 */
void standaloneDataCallback(void* target,
                            Kitsunemimi::Project::Session* session,
                            const uint64_t blockerId,
                            Kitsunemimi::DataBuffer* data)
{
    //delete data;
    TestSession* testClass = static_cast<TestSession*>(target);
    if(session->isClientSide() == false)
    {
        uint8_t data[10];
        testClass->m_session->sendStandaloneData(data, 10);
    }
    else
    {
        testClass->m_end = std::chrono::system_clock::now();
        float duration = std::chrono::duration_cast<chronoMicroSec>(testClass->m_end - testClass->m_start).count();
        duration /= 1000000.0f;

        std::cout<<"duration: "<<duration<<" seconds"<<std::endl;
        const float speed = (static_cast<float>(testClass->m_size) / (1024.0f*1024.0f)) / duration;
        std::cout<<"speed: "<<speed<<" MiB/s"<<std::endl;
    }
}

/**
 * @brief errorCallback
 */
void errorCallback(void*,
                   Kitsunemimi::Project::Session*,
                   const uint8_t,
                   const std::string message)
{
    std::cout<<"ERROR: "<<message<<std::endl;
}

/**
 * @brief sessionCallback
 * @param target
 * @param isInit
 * @param session
 */
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
        testClass->m_session = session;
    }
    else
    {
        std::cout<<"end session"<<std::endl;
        testClass->m_session = nullptr;
    }
}

/**
 * @brief TestSession::TestSession
 * @param address
 * @param port
 */
TestSession::TestSession(const std::string &address,
                         const uint16_t port)
{
    m_size = 1024*1024*1024;
    m_dataBuffer = new uint8_t[m_size];

    m_controller = new Kitsunemimi::Project::SessionController(this, &sessionCallback,
                                                               this, &streamDataCallback,
                                                               this, &standaloneDataCallback,
                                                               this, &errorCallback);

    if(address != "")
    {
        m_isClient = true;
        m_controller->startTcpSession(address, port);
    }
    else
    {
        m_controller->addTcpServer(port);
    }
}

/**
 * @brief TestSession::sendLoop
 */
void
TestSession::sendLoop()
{
    if(m_isClient)
    {
        while(m_session == nullptr) {
            usleep(10000);
        }

        m_start = std::chrono::system_clock::now();

        m_session->sendStandaloneData(m_dataBuffer, m_size);
    }

    while(true)
    {
        usleep(10000);
    }
}
