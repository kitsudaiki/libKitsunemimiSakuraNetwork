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

/**
 * @brief streamDataCallback
 * @param target
 * @param data
 * @param dataSize
 */
void streamDataCallback(void* target,
                        Session*,
                        const void* data,
                        const uint64_t dataSize)
{
    Session_Test* testClass = static_cast<Session_Test*>(target);

    std::string receivedMessage(static_cast<const char*>(data), dataSize);

    bool ret = false;

    if(dataSize == testClass->m_staticMessage.size())
    {
        ret = true;
        testClass->compare(receivedMessage, testClass->m_staticMessage);
    }

    if(dataSize == testClass->m_dynamicMessage.size())
    {
        ret = true;
        testClass->compare(receivedMessage, testClass->m_dynamicMessage);
    }

    testClass->compare(ret,  true);
}

/**
 * @brief standaloneDataCallback
 * @param target
 * @param data
 * @param dataSize
 */
void standaloneDataCallback(void* target,
                            Session*,
                            const uint64_t,
                            DataBuffer* data)
{
    Session_Test* testClass = static_cast<Session_Test*>(target);

    std::string receivedMessage(static_cast<const char*>(data->data), data->bufferPosition);

    if(data->bufferPosition <= 1024)
    {
        testClass->compare(data->bufferPosition, testClass->m_singleBlockMessage.size());
        testClass->compare(receivedMessage, testClass->m_singleBlockMessage);
    }
    else
    {
        testClass->compare(data->bufferPosition, testClass->m_multiBlockMessage.size());
        testClass->compare(receivedMessage, testClass->m_multiBlockMessage);
    }

    delete data;
}

/**
 * @brief errorCallback
 */
void errorCallback(void*,
                   Kitsunemimi::Sakura::Session*,
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
 * @param sessionIdentifier
 */
void sessionCallback(void* target,
                     bool isInit,
                     Kitsunemimi::Sakura::Session* session,
                     const std::string sessionIdentifier)
{

    Session_Test* testClass = static_cast<Session_Test*>(target);

    testClass->compare(session->sessionId(), (uint32_t)131073);

    if(isInit)
    {
        testClass->m_numberOfInitSessions++;

        testClass->compare(sessionIdentifier, std::string("test"));

        if(session->isClientSide())
        {
            bool ret = false;

            // stream-message
            const std::string staticTestString = testClass->m_staticMessage;
            ret = session->sendStreamData(staticTestString.c_str(),
                                          staticTestString.size(),
                                          true);
            testClass->compare(ret,  true);

            // singleblock-message
            const std::string singleblockTestString = testClass->m_singleBlockMessage;
            ret = session->sendStandaloneData(singleblockTestString.c_str(),
                                              singleblockTestString.size());
            testClass->compare(ret,  true);

            // multiblock-message
            const std::string multiblockTestString = testClass->m_multiBlockMessage;
            ret = session->sendStandaloneData(multiblockTestString.c_str(),
                                              multiblockTestString.size());
            testClass->compare(ret,  true);
        }
    }
    else
    {
        testClass->m_numberOfEndSessions++;
    }
}

/**
 * @brief Session_Test::Session_Test
 */
Session_Test::Session_Test() :
    Kitsunemimi::CompareTestHelper("Session_Test")
{
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
 * @brief runTest
 */
void
Session_Test::runTest()
{
    SessionController* m_controller = new SessionController(this, &sessionCallback,
                                                            this, &streamDataCallback,
                                                            this, &standaloneDataCallback,
                                                            this, &errorCallback);

    TEST_EQUAL(m_controller->addTcpServer(1234), 1);
    bool isNullptr = m_controller->startTcpSession("127.0.0.1", 1234, "test") == nullptr;
    TEST_EQUAL(isNullptr, false);

    TEST_EQUAL(m_controller->getSession(131073)->closeSession(), true);
    const bool isNull = m_controller->getSession(131073) == nullptr;
    TEST_EQUAL(isNull, true);

    usleep(100000);

    TEST_EQUAL(m_numberOfInitSessions, 2);
    TEST_EQUAL(m_numberOfEndSessions, 2);

    delete m_controller;
}

} // namespace Sakura
} // namespace Kitsunemimi
