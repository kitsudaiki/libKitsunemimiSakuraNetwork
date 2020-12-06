/**
 * @file    main.cpp
 *
 * @author  Tobias Anker <tobias.anker@kitsunemimi.moe>
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

#include <libKitsunemimiPersistence/logger/logger.h>
#include <libKitsunemimiArgs/arg_parser.h>
#include <test_session.h>

using Kitsunemimi::Persistence::initConsoleLogger;

int main(int argc, char *argv[])
{
    //initConsoleLogger(true);
    Kitsunemimi::Args::ArgParser argParser;

    argParser.registerString("address,a",
                             "address to connect (Default: 127.0.0.1)");
    argParser.registerInteger("port,p",
                              "port where to listen (Default: 4321)");
    argParser.registerString("socket,s",
                             "type: tcp or uds (Default: tcp)");
    argParser.registerString("transfer-type,t",
                             "type of transfer: stream, standalone or request (Default: stream)");
    argParser.registerInteger("package-size",
                              "Test-package-size in byte(Default: 128 KiB)",
                              true,
                              true);

    bool ret = argParser.parse(argc, argv);
    if(ret == false) {
        return 1;
    }

    uint16_t port = 4321;
    std::string address = "127.0.0.1";
    std::string socket = "tcp";
    std::string transferType = "stream";
    long packageSize = 128*1024;

    if(argParser.wasSet("address")) {
        address = argParser.getStringValues("address").at(0);
    }
    if(argParser.wasSet("port")) {
        port = static_cast<uint16_t>(argParser.getIntValues("port").at(0));
    }
    if(argParser.wasSet("socket")) {
        socket = argParser.getStringValues("socket").at(0);
    }
    if(argParser.wasSet("transfer-type")) {
        transferType = argParser.getStringValues("transfer-type").at(0);
    }

    packageSize = argParser.getIntValue("package-size");

    // precheck type
    if(socket != "tcp"
            && socket != "uds")
    {
        std::cout<<"ERROR: type \""<<socket<<"\" is unknown. Choose \"tcp\" or \"uds\"."<<std::endl;
        exit(1);
    }

    // precheck transfer-type
    if(transferType != "stream"
            && transferType != "standalone"
            && transferType != "request"
            && transferType != "stack_stream")
    {
        std::cout<<"ERROR: transfer-type \""<<transferType<<"\" is unknown. "
                   "Choose \"stream\", \"standalone\" or \"request\"."<<std::endl;;
        exit(1);
    }

    // ouptput set values
    std::cout<<"--------------------------------------"<<std::endl;
    std::cout<<"address: "<<address<<std::endl;
    std::cout<<"port: "<<static_cast<int>(port)<<std::endl;
    std::cout<<"socket: "<<socket<<std::endl;
    std::cout<<"transfer-type: "<<transferType<<std::endl;
    std::cout<<"package-size: "<<packageSize<<std::endl;
    std::cout<<"--------------------------------------"<<std::endl;

    Kitsunemimi::Sakura::TestSession testSession(address,
                                                 port,
                                                 socket,
                                                 transferType);

    testSession.runTest(packageSize);
}
