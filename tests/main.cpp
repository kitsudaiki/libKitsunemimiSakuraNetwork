/**
 *  @file    main.cpp
 *
 *  @author  Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 *  @copyright MIT License
 */

#include <iostream>
#include <logger/logger.h>
#include <network_session/session_handler.h>
#include <network_session/session.h>

using Kitsune::Persistence::initLogger;

void callback(void* target, Kitsune::Project::Common::Session session) {
    Kitsune::Persistence::LOG_debug("callback session with id: "
                                    + std::to_string(session.sessionId));
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
