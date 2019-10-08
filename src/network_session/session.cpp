/**
 *  @file       session.h
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

#include <libKitsuneProjectCommon/network_session/session.h>
#include <libKitsuneNetwork/abstract_socket.h>

namespace Kitsune
{
namespace Project
{
namespace Common
{

Session::Session()
{
    initStatemachine();
}

Session::~Session()
{

}

void Session::initStatemachine()
{
    // init states
    assert(m_statemachine.createNewState("not connected"));
    assert(m_statemachine.createNewState("connected"));
    assert(m_statemachine.createNewState("session not init"));
    assert(m_statemachine.createNewState("session init"));
    assert(m_statemachine.createNewState("normal"));
    assert(m_statemachine.createNewState("in datatransfer"));

    // set child state
    assert(m_statemachine.addChildState("connected", "session not init"));
    assert(m_statemachine.addChildState("connected", "session init"));
    assert(m_statemachine.addChildState("session init", "normal"));
    assert(m_statemachine.addChildState("session init", "in datatransfer"));

    // set initial states
    assert(m_statemachine.setInitialChildState("connected", "session not init"));
    assert(m_statemachine.setInitialChildState("session init", "normal"));

    // init transitions
    assert(m_statemachine.addTransition("not connected",
                                        "connect",
                                        "connected"));

    assert(m_statemachine.addTransition("connected",
                                        "disconnect",
                                        "not connected"));

    assert(m_statemachine.addTransition("session not init",
                                        "start session",
                                        "session init"));

    assert(m_statemachine.addTransition("session init",
                                        "stop session",
                                        "session not init"));

    assert(m_statemachine.addTransition("normal",
                                        "start datatransfer",
                                        "in datatransfer"));

    assert(m_statemachine.addTransition("in datatransfer",
                                        "stop datatransfer",
                                        "normal"));


}

} // namespace Common
} // namespace Project
} // namespace Kitsune
