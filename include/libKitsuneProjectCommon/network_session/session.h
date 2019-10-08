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

#ifndef SESSION_H
#define SESSION_H

#include <iostream>
#include <assert.h>
#include <libKitsuneCommon/statemachine.h>

namespace Kitsune
{
namespace Common {
class Statemachine;
}
namespace Network {
class AbstractSocket;
}
namespace Project
{
namespace Common
{

class Session
{
public:
    Session();
    ~Session();

    uint32_t sessionId = 0;
    Network::AbstractSocket* socket = nullptr;

private:
    Kitsune::Common::Statemachine m_statemachine;

    void initStatemachine();

};

} // namespace Common
} // namespace Project
} // namespace Kitsune

#endif // SESSION_H
