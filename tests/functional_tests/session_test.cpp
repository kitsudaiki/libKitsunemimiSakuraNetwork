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
namespace Project
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
                            const void* data,
                            const uint64_t dataSize)
{
    Session_Test* testClass = static_cast<Session_Test*>(target);

    std::string receivedMessage(static_cast<const char*>(data), dataSize);
    testClass->compare(dataSize, testClass->m_multiBlockMessage.size());
    testClass->compare(receivedMessage, testClass->m_multiBlockMessage);
}

/**
 * @brief errorCallback
 */
void errorCallback(void*,
                   Session*,
                   const uint8_t,
                   const std::string)
{
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
                     Kitsunemimi::Project::Session* session,
                     const uint64_t sessionIdentifier)
{

    Session_Test* testClass = static_cast<Session_Test*>(target);

    testClass->compare(session->sessionId(), (uint32_t)131072);

    if(isInit)
    {
        testClass->m_numberOfInitSessions++;

        if(session->isClientSide() == false) {
            testClass->compare(sessionIdentifier,  (uint64_t)42);
        }

        if(session->isClientSide())
        {
            bool ret = false;

            // static size
            const std::string staticTestString = testClass->m_staticMessage;
            ret = session->sendStreamData(staticTestString.c_str(),
                                          staticTestString.size(),
                                          false,
                                          true);
            testClass->compare(ret,  true);

            // dynamic size
            const std::string dynamicTestString = testClass->m_dynamicMessage;
            ret = session->sendStreamData(dynamicTestString.c_str(),
                                          dynamicTestString.size(),
                                          true,
                                          true);
            testClass->compare(ret,  true);

            // multiblock
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
    Kitsunemimi::Test("Session_Test")
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
    TEST_EQUAL(m_controller->startTcpSession("127.0.0.1", 1234, 42), 1);

    sleep(2);

    TEST_EQUAL(m_controller->getSession(131072)->closeSession(), true);
    const bool isNull = m_controller->getSession(131072) == nullptr;
    TEST_EQUAL(isNull, true);

    usleep(100000);

    TEST_EQUAL(m_numberOfInitSessions, 2);
    TEST_EQUAL(m_numberOfEndSessions, 2);

    delete m_controller;
}

} // namespace Project
} // namespace Kitsunemimi
