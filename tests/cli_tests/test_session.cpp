#include "test_session.h"

#include <libKitsunemimiProjectNetwork/session.h>
#include <libKitsunemimiProjectNetwork/session_controller.h>
#include <libKitsunemimiCommon/common_methods/string_methods.h>
#include <libKitsunemimiCommon/buffer/data_buffer.h>

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
    std::string receivedMessage(static_cast<const char*>(data->data), data->bufferPosition);

    if(session->isClientSide() == false)
    {
        std::cout<<"+++++++++++++++++++++++++++++++++++++++++++++++++"<<std::endl;
        std::cout<<"blockerId: "<<blockerId<<std::endl;
        std::cout<<"message: "<<std::endl;
        std::cout<<receivedMessage<<std::endl;
        std::cout<<""<<std::endl;
        std::cout<<"-------------------------------------------------"<<std::endl;
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
    while(true)
    {
        while(m_session == nullptr) {
            usleep(10000);
        }

        std::cout<<"ready for input:"<<std::endl;
        std::string message = "";
        std::cin >> message;

        if(m_session != nullptr)
        {
            if(m_isClient)
            {
                Kitsunemimi::DataBuffer* data = m_session->sendRequest(message.c_str(),
                                                                       message.size(),
                                                                       10);

                const std::string stringMessage = std::string((char*)data->data,
                                                              data->bufferPosition);

                std::cout<<"#################################################"<<std::endl;
                std::cout<<"message: "<<std::endl;
                std::cout<<stringMessage<<std::endl;
                std::cout<<""<<std::endl;
                std::cout<<"================================================="<<std::endl;
            }
            else
            {
                std::vector<std::string> splitted;
                Kitsunemimi::splitStringByDelimiter(splitted, message, '-');
                long id = strtol(splitted.at(1).c_str(), NULL, 10);
                m_session->sendResponse(splitted.at(0).c_str(),
                                        splitted.at(0).size(),
                                        id);
            }

        }
    }
}
