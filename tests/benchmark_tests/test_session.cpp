#include "test_session.h"

#include <libKitsunemimiProjectNetwork/session.h>
#include <libKitsunemimiProjectNetwork/session_controller.h>

#include <libKitsunemimiCommon/common_methods/string_methods.h>
#include <libKitsunemimiCommon/data_buffer.h>
#include <libKitsunemimiCommon/common_items/table_item.h>

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
    TestSession* testClass = static_cast<TestSession*>(target);
    if(session->isClientSide() == false)
    {
        testClass->m_sizeCounter += dataSize;

        if(testClass->m_sizeCounter == testClass->m_size*10)
        {
            uint8_t data[10];
            testClass->m_serverSession->sendStreamData(data, 10);
        }
    }
    else
    {
        testClass->m_end = std::chrono::system_clock::now();
        float duration = std::chrono::duration_cast<chronoMicroSec>(testClass->m_end - testClass->m_start).count();
        duration /= 1000000.0f;
        const float speed = ((static_cast<float>(testClass->m_size*10)
                             / (1024.0f*1024.0f*1024.0f))
                             / duration) * 8;

        Kitsunemimi::TableItem result;

        result.addColumn("key");
        result.addColumn("value");

        result.addRow(std::vector<std::string>{"duration", std::to_string(duration) + " seconds"});
        result.addRow(std::vector<std::string>{"speed", std::to_string(speed) + " Gbits/sec"});

        std::cout<<result.toString()<<std::endl;

        exit(0);
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
        testClass->m_serverSession->sendStandaloneData(data, 10);
    }
    else
    {
        testClass->m_sizeCounter += data->bufferPosition;

        if(testClass->m_sizeCounter == testClass->m_size)
        {
            testClass->m_end = std::chrono::system_clock::now();
            float duration = std::chrono::duration_cast<chronoMicroSec>(testClass->m_end - testClass->m_start).count();
            duration /= 1000000.0f;

            std::cout<<"duration: "<<duration<<" seconds"<<std::endl;
            const float speed = (static_cast<float>(testClass->m_size) / (1024.0f*1024.0f)) / duration;
            std::cout<<"speed: "<<speed<<" MiB/s"<<std::endl;
        }

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
        if(session->isClientSide())
        {
            std::cout<<"init session"<<std::endl;
            testClass->m_clientSession = session;
        }
        else
        {
            testClass->m_serverSession = session;
        }
    }
    else
    {
        std::cout<<"end session"<<std::endl;
        testClass->m_clientSession = nullptr;
        testClass->m_serverSession = nullptr;
    }
}

/**
 * @brief TestSession::TestSession
 * @param address
 * @param port
 */
TestSession::TestSession(const std::string &address,
                         const uint16_t port,
                         const std::string &type)
{
    m_size = 1024*1024*1024;
    m_dataBuffer = new uint8_t[1024*1024];

    if(type == "tcp") {
        m_isTcp = true;
    } else {
        m_isTcp = false;
    }

    m_controller = new Kitsunemimi::Project::SessionController(this, &sessionCallback,
                                                               this, &streamDataCallback,
                                                               this, &standaloneDataCallback,
                                                               this, &errorCallback);

    if(port == 0)
    {
        if(m_isTcp)
        {
            m_isClient = true;
            m_controller->addTcpServer(4321);
            usleep(10000);
            m_controller->startTcpSession("127.0.0.1", 4321);
        }
        else
        {
            m_isClient = true;
            m_controller->addUnixDomainServer("/tmp/sock.uds");
            usleep(10000);
            m_controller->startUnixDomainSession("/tmp/sock.uds");
        }
    }
    else
    {
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
}

/**
 * @brief TestSession::sendLoop
 */
void
TestSession::sendLoop()
{
    if(m_isClient)
    {
        while(m_clientSession == nullptr) {
            usleep(10000);
        }

        m_start = std::chrono::system_clock::now();

        for(int i = 0; i < 10*8*1024; i++)
        {
            assert(m_clientSession->sendStreamData(m_dataBuffer, 128*1024));
        }
    }

    while(true)
    {
        usleep(10000);
    }
}
