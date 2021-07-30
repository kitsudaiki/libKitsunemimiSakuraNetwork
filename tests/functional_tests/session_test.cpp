/**
 * @file       session_test.cpp
 *
 * @author     Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 * @copyright  Apache License Version 2.0
 *
 *      Copyright 2019 Tobias Anker
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *          http://www.apache.org/licenses/LICENSE-2.0
 *
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 */

#include "session_test.h"

namespace Kitsunemimi
{
namespace Sakura
{

Kitsunemimi::Sakura::Session_Test* Session_Test::m_instance = nullptr;

/**
 * @brief streamDataCallback
 * @param target
 * @param data
 * @param dataSize
 */
void streamDataCallback(Session*,
                        const void* data,
                        const uint64_t dataSize)
{
    LOG_DEBUG("TEST: streamDataCallback");

    std::string receivedMessage(static_cast<const char*>(data), dataSize);

    bool ret = false;

    if(dataSize == Session_Test::m_instance->m_staticMessage.size())
    {
        ret = true;
        Session_Test::m_instance->compare(receivedMessage, Session_Test::m_instance->m_staticMessage);
    }

    if(dataSize == Session_Test::m_instance->m_dynamicMessage.size())
    {
        ret = true;
        Session_Test::m_instance->compare(receivedMessage, Session_Test::m_instance->m_dynamicMessage);
    }

    Session_Test::m_instance->compare(ret,  true);
}

/**
 * @brief standaloneDataCallback
 * @param target
 * @param data
 * @param dataSize
 */
void standaloneDataCallback(Session*,
                            const uint64_t,
                            DataBuffer* data)
{
    LOG_DEBUG("TEST: standaloneDataCallback");

    std::string receivedMessage(static_cast<const char*>(data->data), data->bufferPosition);

    if(data->bufferPosition <= 1024)
    {
        Session_Test::m_instance->compare(data->bufferPosition,
                                          Session_Test::m_instance->m_singleBlockMessage.size());
        Session_Test::m_instance->compare(receivedMessage,
                                          Session_Test::m_instance->m_singleBlockMessage);
    }
    else
    {
        Session_Test::m_instance->compare(data->bufferPosition,
                                          Session_Test::m_instance->m_multiBlockMessage.size());
        Session_Test::m_instance->compare(receivedMessage,
                                          Session_Test::m_instance->m_multiBlockMessage);
    }

    delete data;
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
 * @param sessionIdentifier
 */
void sessionCreateCallback(Kitsunemimi::Sakura::Session* session,
                           const std::string sessionIdentifier)
{
    session->setStreamMessageCallback(&streamDataCallback);
    session->setStandaloneMessageCallback(&standaloneDataCallback);

    Session_Test::m_instance->compare(session->sessionId(), (uint32_t)131073);
    Session_Test::m_instance->m_numberOfInitSessions++;
    Session_Test::m_instance->compare(sessionIdentifier, std::string("test"));
    Session_Test::m_instance->m_testSession = session;
}

void sessionCloseCallback(Kitsunemimi::Sakura::Session*,
                          const std::string)
{
    Session_Test::m_instance->m_numberOfEndSessions++;
}

/**
 * @brief Session_Test::Session_Test
 */
Session_Test::Session_Test() :
    Kitsunemimi::CompareTestHelper("Session_Test")
{
    Session_Test::m_instance = this;

    initTestCase();
    runTest();
}

/**
 * @brief initTestCase
 */
void
Session_Test::initTestCase()
{
    m_staticMessage = "hello!!! (static)";
    m_dynamicMessage = "hello!!! (dynamic)";
    m_singleBlockMessage ="------------------------------------------------------------------------"
                          "-------------------------------------#----------------------------------"
                          "------------------------------------------------------------------------"
                          "---#--------------------------------------------------------------------"
                          "-----------------------------------------#------------------------------"
                          "------------------------------------------------------------------------"
                          "-------#----------------------------------------------------------------"
                          "---------------------------------------------#--------------------------"
                          "------------------------------------------------------------------------"
                          "-----------#------------------------------------------------------------"
                          "-------------------------------------------------#----------------------"
                          "-----#";
    m_multiBlockMessage = "------------------------------------------------------------------------"
                          "-------------------------------------#----------------------------------"
                          "------------------------------------------------------------------------"
                          "---#--------------------------------------------------------------------"
                          "-----------------------------------------#------------------------------"
                          "------------------------------------------------------------------------"
                          "-------#----------------------------------------------------------------"
                          "---------------------------------------------#--------------------------"
                          "------------------------------------------------------------------------"
                          "-----------#------------------------------------------------------------"
                          "-------------------------------------------------#----------------------"
                          "------------------------------------------------------------------------"
                          "-------------------------------------#----------------------------------"
                          "------------------------------------------------------------------------"
                          "---#--------------------------------------------------------------------"
                          "-----------------------------------------#------------------------------"
                          "------------------------------------------------------------------------"
                          "-------#----------------------------------------------------------------"
                          "---------------------------------------------#--------------------------"
                          "------------------------------------------------------------------------"
                          "-----------#------------------------------------------------------------"
                          "-------------------------------------------------#----------------------"
                          "------------------------------------------------------------------------"
                          "-------------------------------------#----------------------------------"
                          "------------------------------------------------------------------------"
                          "---#--------------------------------------------------------------------"
                          "-----------------------------------------#------------------------------"
                          "------------------------------------------------------------------------"
                          "-------#----------------------------------------------------------------"
                          "---------------------------------------------#--------------------------"
                          "------------------------------------------------------------------------"
                          "-----------#------------------------------------------------------------"
                          "-------------------------------------------------#----------------------"
                          "------------------------------------------------------------------------"
                          "-------------------------------------#----------------------------------"
                          "------------------------------------------------------------------------"
                          "---#--------------------------------------------------------------------"
                          "-----------------------------------------#------------------------------"
                          "------------------------------------------------------------------------"
                          "-------#----------------------------------------------------------------"
                          "---------------------------------------------#--------------------------"
                          "------------------------------------------------------------------------"
                          "-----------#------------------------------------------------------------"
                          "-------------------------------------------------#----------------------"
                          "-----#";
}

/**
 * @brief Session_Test::sendTestMessages
 * @param session
 */
void
Session_Test::sendTestMessages(Session* session)
{
    bool ret = false;

    // stream-message
    const std::string staticTestString = Session_Test::m_instance->m_staticMessage;
    ret = session->sendStreamData(staticTestString.c_str(),
                                  staticTestString.size(),
                                  true);
    Session_Test::m_instance->compare(ret,  true);

    // singleblock-message
    const std::string singleblockTestString = Session_Test::m_instance->m_singleBlockMessage;
    ret = session->sendStandaloneData(singleblockTestString.c_str(),
                                      singleblockTestString.size());
    Session_Test::m_instance->compare(ret,  true);

    // multiblock-message
    const std::string multiblockTestString = Session_Test::m_instance->m_multiBlockMessage;
    ret = session->sendStandaloneData(multiblockTestString.c_str(),
                                      multiblockTestString.size());
    Session_Test::m_instance->compare(ret,  true);
}

/**
 * @brief runTest
 */
void
Session_Test::runTest()
{
    SessionController* m_controller = new SessionController(&sessionCreateCallback,
                                                            &sessionCloseCallback,
                                                            &errorCallback);

    TEST_EQUAL(m_controller->addUnixDomainServer("/tmp/sock.uds"), 1);
    bool isNullptr = m_controller->startUnixDomainSession("/tmp/sock.uds", "test") == nullptr;
    TEST_EQUAL(isNullptr, false);


    isNullptr = m_testSession == nullptr;
    TEST_EQUAL(isNullptr, false);

    if(isNullptr) {
        return;
    }

    sendTestMessages(m_testSession);

    usleep(100000);

    LOG_DEBUG("TEST: close session again");
    bool ret = m_testSession->closeSession();
    TEST_EQUAL(ret, true);
    LOG_DEBUG("TEST: close session finished");

    usleep(100000);

    TEST_EQUAL(m_numberOfInitSessions, 2);
    TEST_EQUAL(m_numberOfEndSessions, 2);

    delete m_controller;
}

} // namespace Sakura
} // namespace Kitsunemimi
