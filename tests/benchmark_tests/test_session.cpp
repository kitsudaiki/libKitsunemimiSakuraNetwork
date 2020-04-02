#include "test_session.h"

#include <libKitsunemimiCommon/buffer/stack_buffer.h>

#include <libKitsunemimiProjectNetwork/session.h>
#include <libKitsunemimiProjectNetwork/session_controller.h>

#include <libKitsunemimiCommon/common_methods/string_methods.h>
#include <libKitsunemimiCommon/buffer/data_buffer.h>
#include <libKitsunemimiCommon/common_items/table_item.h>

/**
 * @brief streamDataCallback
 */
void streamDataCallback(void* target,
                        Kitsunemimi::Project::Session* session,
                        const void*,
                        const uint64_t dataSize)
{
    TestSession* testClass = static_cast<TestSession*>(target);
    if(session->isClientSide() == false)
    {
        testClass->m_sizeCounter += dataSize;
        //std::cout<<"testClass->m_sizeCounter: "<<testClass->m_sizeCounter<<std::endl;
        if(testClass->m_sizeCounter == testClass->m_totalSize)
        {
            uint8_t data[10];
            testClass->m_serverSession->sendStreamData(data, 10);
        }
    }
    else
    {
        testClass->m_sizeCounter = 0;
        testClass->m_cv.notify_all();
    }
}

/**
 * @brief standaloneDataCallback
 */
void standaloneDataCallback(void* target,
                            Kitsunemimi::Project::Session* session,
                            const uint64_t blockerId,
                            Kitsunemimi::DataBuffer* data)
{
    TestSession* testClass = static_cast<TestSession*>(target);

    // handling for request transfer-type
    if(testClass->m_transferType == "request")
    {
        if(session->isClientSide() == false)
        {
            testClass->m_sizeCounter += data->bufferPosition;
            delete data;
            uint8_t data[10];
            testClass->m_serverSession->sendResponse(data, 10, blockerId);
        }
    }

    // handling for standalone transfer-type
    if(testClass->m_transferType == "standalone")
    {
        if(session->isClientSide() == false)
        {
            testClass->m_sizeCounter += data->bufferPosition;
            delete data;
            uint8_t data[10];
            testClass->m_serverSession->sendStandaloneData(data, 10);
        }
        else
        {
            testClass->m_sizeCounter = 0;
            testClass->m_cv.notify_all();
        }
    }
}

/**
 * @brief errorCallback
 */
void errorCallback(void*,
                   Kitsunemimi::Project::Session*,
                   const uint8_t errorType,
                   const std::string message)
{
    std::cout<<"ERROR: "<<message<<std::endl;
}

/**
 * @brief sessionCallback
 */
void sessionCallback(void* target,
                     bool isInit,
                     Kitsunemimi::Project::Session* session,
                     const std::string)
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
 * @brief iniitialize test-class
 *
 * @param address ip address
 * @param port port-number
 * @param socket socket-type (tcp or uds)
 * @param transferType transfer-type (stream, standalone or request)
 */
TestSession::TestSession(const std::string &address,
                         const uint16_t port,
                         const std::string &socket,
                         const std::string &transferType)
{
    // init global values
    m_totalSize = 1024l*1024l*1024l*10l;
    m_dataBuffer = new uint8_t[128*1024*1024];

    m_transferType = transferType;
    if(socket == "tcp") {
        m_isTcp = true;
    } else {
        m_isTcp = false;
    }

    m_timeSlot.unitName = "Gbits/s";

    // create controller and connect callbacks
    m_controller = new Kitsunemimi::Project::SessionController(this, &sessionCallback,
                                                               this, &streamDataCallback,
                                                               this, &standaloneDataCallback,
                                                               this, &errorCallback);

    if(m_isTcp)
    {
        if(address == "127.0.0.1")
        {
            m_isClient = true;
            m_controller->addTcpServer(port);
            usleep(10000);
            m_controller->startTcpSession(address, port);
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
    else
    {
        m_isClient = true;
        m_controller->addUnixDomainServer("/tmp/sock.uds");
        usleep(10000);
        m_controller->startUnixDomainSession("/tmp/sock.uds");
    }
}

/**
 * @brief run test
 */
void
TestSession::runTest(const long packageSize)
{

    if(m_isClient)
    {
        std::unique_lock<std::mutex> lock(m_cvMutex);

        // send stack_stream-messages
        if(m_transferType == "stack_stream")
        {
            m_stackBuffer = new Kitsunemimi::StackBuffer(20, 4);
            for(uint32_t i = 0; i < 8*1024; i++)
            {
                Kitsunemimi::DataBuffer* temp = new Kitsunemimi::DataBuffer((256*1024) / 4096);
                temp->bufferPosition = 128*1024+24;
                m_stackBuffer->blocks.push_back(temp);
            }

            m_timeSlot.name = "stack_stream-speed";
            for(int j = 0; j < 10; j++)
            {
                std::cout<<"stack_stream"<<std::endl;
                m_timeSlot.startTimer();
                for(int i = 0; i < 10; i++)
                {
                    assert(m_clientSession->sendStreamData(*m_stackBuffer));
                }
                m_cv.wait(lock);

                m_timeSlot.stopTimer();
                m_timeSlot.values.push_back(calculateSpeed(m_timeSlot.getDuration(MICRO_SECONDS)));
            }
        }

        // send stream-messages
        if(m_transferType == "stream")
        {
            m_timeSlot.name = "stream-speed";
            for(int j = 0; j < 10; j++)
            {
                std::cout<<"stream"<<std::endl;
                m_timeSlot.startTimer();
                for(long i = 0; i < (10l*1024l*1024l*1024l) / packageSize; i++)
                {
                    assert(m_clientSession->sendStreamData(m_dataBuffer,
                                                           static_cast<uint64_t>(packageSize)));
                }
                m_cv.wait(lock);

                m_timeSlot.stopTimer();
                m_timeSlot.values.push_back(calculateSpeed(m_timeSlot.getDuration(MICRO_SECONDS)));
            }
        }

        // send standalone-messages
        if(m_transferType == "standalone")
        {
            m_timeSlot.name = "standalone-speed";
            for(int j = 0; j < 10; j++)
            {
                std::cout<<"standalone"<<std::endl;
                m_timeSlot.startTimer();
                for(int i = 0; i < (10l*1024l*1024l*1024l) / packageSize; i++)
                {
                    m_clientSession->sendStandaloneData(m_dataBuffer,
                                                        static_cast<uint64_t>(packageSize));
                    m_cv.wait(lock);
                }
                m_timeSlot.stopTimer();
                m_timeSlot.values.push_back(calculateSpeed(m_timeSlot.getDuration(MICRO_SECONDS)));
            }
        }

        // send request-messages
        if(m_transferType == "request")
        {
            m_timeSlot.name = "request-speed";
            for(int j = 0; j < 10; j++)
            {
                std::cout<<"request"<<std::endl;
                m_timeSlot.startTimer();
                for(int i = 0; i < (10l*1024l*1024l*1024l) / packageSize; i++)
                {
                    m_clientSession->sendRequest(m_dataBuffer,
                                                 static_cast<uint64_t>(packageSize),
                                                 10000);
                }
                m_sizeCounter = 0;
                m_timeSlot.stopTimer();
                m_timeSlot.values.push_back(calculateSpeed(m_timeSlot.getDuration(MICRO_SECONDS)));
            }
        }

        // create output of the test-result
        addToResult(m_timeSlot);
        printResult();
    }
}

double
TestSession::calculateSpeed(double duration)
{
    duration /= 1000000.0;

    const double speed = ((static_cast<double>(m_totalSize)
                          / (1024.0*1024.0*1024.0))
                          / duration) * 8.0;
    return speed;
}
