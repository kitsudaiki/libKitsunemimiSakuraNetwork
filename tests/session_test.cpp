/**
 *  @file       session_test.cpp
 *
 *  @author     Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 *  @copyright  Apache License Version 2.0
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

namespace Kitsune
{
namespace Project
{
namespace Common
{

void dataCallback(void* target,
                  Session*,
                  const bool isStream,
                  const void* data,
                  const uint64_t dataSize)
{
    Session_Test* testClass = static_cast<Session_Test*>(target);

    std::string receivedMessage(static_cast<const char*>(data), dataSize);

    if(isStream)
    {
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
    else
    {
        testClass->compare(dataSize, testClass->m_multiBlockMessage.size());
        testClass->compare(receivedMessage, testClass->m_multiBlockMessage);
    }
}

void errorCallback(void*,
                   Session*,
                   const uint8_t,
                   const std::string)
{
}

void sessionCallback(void* target,
                     Kitsune::Project::Common::Session* session)
{

    Session_Test* testClass = static_cast<Session_Test*>(target);

    const uint32_t id = session->sessionId();
    testClass->compare(id, (uint32_t)131073);

    if(session->socket()->isClientSide())
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
                                          multiblockTestString.size(),
                                          true);
        testClass->compare(ret,  true);
    }
}

Session_Test::Session_Test() :
    Kitsune::Common::UnitTest("Session_Test")
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
                                                            this, &dataCallback,
                                                            this, &errorCallback);

    UNITTEST(m_controller->addTcpServer(1234), 1);
    UNITTEST(m_controller->startTcpSession("127.0.0.1", 1234), 1);

    sleep(2);

    UNITTEST(m_controller->getSession(131073)->closeSession(), true);
    UNITTEST(m_controller->getSession(131073)->closeSession(), false);

}

} // namespace Common
} // namespace Project
} // namespace Kitsune
