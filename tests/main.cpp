/**
 *  @file    main.cpp
 *
 *  @author  Tobias Anker <tobias.anker@kitsunemimi.moe>
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

#include <iostream>
#include <libKitsunePersistence/logger/logger.h>
#include <libKitsuneProjectCommon/network_session/session_handler.h>
#include <libKitsuneProjectCommon/network_session/session.h>

using Kitsune::Persistence::initLogger;

void callback(void* target, Kitsune::Project::Common::Session* session) {
    LOG_DEBUG("callback session with id: " + std::to_string(session->sessionId));
}

int main()
{
    initLogger("/tmp", "testlog", true, true);

    Kitsune::Project::Common::SessionHandler* m_handler =
            new Kitsune::Project::Common::SessionHandler(nullptr, &callback);
    m_handler->addTcpServer(1234);

    m_handler->startTcpSession("127.0.0.1", 1234);

    m_handler->startTcpSession("127.0.0.1", 1234);


    sleep(2);
}
