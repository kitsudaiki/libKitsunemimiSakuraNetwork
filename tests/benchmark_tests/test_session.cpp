#include "test_session.h"

#include <libKitsunemimiCommon/buffer/stack_buffer.h>

#include <libKitsunemimiSakuraNetwork/session.h>
#include <libKitsunemimiSakuraNetwork/session_controller.h>

#include <libKitsunemimiCommon/common_methods/string_methods.h>
#include <libKitsunemimiCommon/buffer/data_buffer.h>
#include <libKitsunemimiCommon/common_items/table_item.h>

namespace Kitsunemimi
{
namespace Sakura
{

Kitsunemimi::Sakura::TestSession* TestSession::m_instance = nullptr;

/**
 * @brief streamDataCallback
 */
void streamDataCallback(void* target,
                        Kitsunemimi::Sakura::Session* session,
                        const void*,
                        const uint64_t dataSize)
{
    TestSession* instance = static_cast<TestSession*>(target);

    if(session->isClientSide() == false)
    {
        instance->m_sizeCounter += dataSize;
        //std::cout<<"testClass->m_sizeCounter: "<<testClass->m_sizeCounter<<std::endl;
        if(instance->m_sizeCounter == instance->m_totalSize)
        {
            uint8_t data[10];
            instance->m_serverSession->sendStreamData(data, 10);
            instance->m_sizeCounter = 0;
        }
    }
    else
    {
        instance->m_sizeCounter = 0;
        instance->m_cv.notify_all();
    }
}

/**
 * @brief standaloneDataCallback
 */
void standaloneDataCallback(void* target,
                            Kitsunemimi::Sakura::Session* session,
                            const uint64_t blockerId,
                            Kitsunemimi::DataBuffer* data)
{
    TestSession* instance = static_cast<TestSession*>(target);

    // handling for request transfer-type
    if(instance->m_transferType == "request")
    {
        if(session->isClientSide() == false)
        {
            instance->m_sizeCounter += data->usedBufferSize;
            delete data;
            uint8_t data[10];
            instance->m_serverSession->sendResponse(data, 10, blockerId);
        }
    }

    // handling for standalone transfer-type
    if(instance->m_transferType == "standalone")
    {
        if(session->isClientSide() == false)
        {
            instance->m_sizeCounter += data->usedBufferSize;
            delete data;
            uint8_t data[10];
            instance->m_serverSession->sendStandaloneData(data, 10);
        }
        else
        {
            instance->m_sizeCounter = 0;
            instance->m_cv.notify_all();
        }
    }
}

/**
 * @brief errorCallback
 */
void errorCallback(Kitsunemimi::Sakura::Session*,
                   const uint8_t,
                   const std::string message)
{
    std::cout<<"ERROR: "<<message<<std::endl;
}

/**
 * @brief sessionCreateCallback
 * @param session
 */
void sessionCreateCallback(Kitsunemimi::Sakura::Session* session,
                           const std::string)
{
    session->setStreamMessageCallback(TestSession::m_instance, &streamDataCallback);
    session->setStandaloneMessageCallback(TestSession::m_instance, &standaloneDataCallback);

    std::cout<<"session-callback for id: "<<session->m_sessionId<<"\n"<<std::endl;

    if(session->isClientSide())
    {
        std::cout<<"init session"<<std::endl;
        TestSession::m_instance->m_clientSession = session;
    }
    else
    {
        TestSession::m_instance->m_serverSession = session;
    }
}

/**
 * @brief sessionCloseCallback
 */
void sessionCloseCallback(Kitsunemimi::Sakura::Session*,
                          const std::string)
{
    std::cout<<"end session"<<std::endl;
    TestSession::m_instance->m_clientSession = nullptr;
    TestSession::m_instance->m_serverSession = nullptr;
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
    TestSession::m_instance = this;

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
    m_controller = new Kitsunemimi::Sakura::SessionController(&sessionCreateCallback,
                                                              &sessionCloseCallback,
                                                              &errorCallback);

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
            if(address != "server")
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
    else
    {
        std::unique_lock<std::mutex> lock(m_cvMutex);
        m_cv.wait(lock);
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

}
}
