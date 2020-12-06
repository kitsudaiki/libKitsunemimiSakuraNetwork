#include "test_session.h"

#include <libKitsunemimiSakuraNetwork/session.h>
#include <libKitsunemimiSakuraNetwork/session_controller.h>
#include <libKitsunemimiCommon/common_methods/string_methods.h>
#include <libKitsunemimiCommon/buffer/data_buffer.h>

namespace Kitsunemimi
{
namespace Sakura
{

Kitsunemimi::Sakura::TestSession* TestSession::m_instance = nullptr;

/**
 * @brief standaloneDataCallback
 * @param target
 * @param data
 * @param dataSize
 */
void streamDataCallback(Kitsunemimi::Sakura::Session* session,
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
void standaloneDataCallback(Kitsunemimi::Sakura::Session* session,
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
void errorCallback(Kitsunemimi::Sakura::Session*,
                   const uint8_t,
                   const std::string message)
{
    std::cout<<"ERROR: "<<message<<std::endl;
}

/**
 * @brief sessionCallback
 * @param session
 */
void sessionCreateCallback(Kitsunemimi::Sakura::Session* session,
                           const std::string)
{
    session->setStreamMessageCallback(&streamDataCallback);
    session->setStandaloneMessageCallback(&standaloneDataCallback);

    std::cout<<"session-callback for id: "<<session->m_sessionId<<"\n"<<std::endl;
    std::cout<<"init session"<<std::endl;
    TestSession::m_instance->m_session = session;
}

/**
 * @brief sessionCloseCallback
 */
void sessionCloseCallback(Kitsunemimi::Sakura::Session*,
                          const std::string)
{
    std::cout<<"end session"<<std::endl;
    TestSession::m_instance->m_session = nullptr;
}

/**
 * @brief TestSession::TestSession
 * @param address
 * @param port
 */
TestSession::TestSession(const std::string &address,
                         const uint16_t port)
{
    TestSession::m_instance = this;
    m_controller = new Kitsunemimi::Sakura::SessionController(&sessionCreateCallback,
                                                              &sessionCloseCallback,
                                                              &errorCallback);

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

}
}
