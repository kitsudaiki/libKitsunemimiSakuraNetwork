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
#include <boost/program_options.hpp>
#include <test_session.h>

using Kitsunemimi::Persistence::initLogger;
namespace argParser = boost::program_options;


int main(int argc, char *argv[])
{
    argParser::options_description desc("Allowed options");
    desc.add_options()
        (
            "help,h", // -h and --help for help-text
            "produce help message"
        )
        (
            "address,a",
            argParser::value<std::string>(),
            "address to connect (Default: 127.0.0.1)"
        )
        (
            "port,p",
            argParser::value<uint16_t>(),
            "port where to listen"
        )
        (
            "socket,s",
            argParser::value<std::string>(),
            "type: tcp or uds (Default: tcp)"
        )
        (
            "transfer-type,t",
            argParser::value<std::string>(),
            "type of transfer: stream, standalone or request (Default: stream)"
        )
    ;

    argParser::variables_map vm;
    argParser::store(argParser::parse_command_line(argc, argv, desc), vm);
    argParser::notify(vm);

    // help-arg
    if(vm.count("help"))
    {
        std::cout << desc << std::endl;
        return 0;
    }

    uint16_t port = 0;
    std::string address = "127.0.0.1";
    std::string socket = "tcp";
    std::string transferType = "stream";

    if(vm.count("address")) {
        address = vm["address"].as<std::string>();
    }
    if(vm.count("port")) {
        port = vm["port"].as<uint16_t>();
    }
    if(vm.count("socket")) {
        socket = vm["socket"].as<std::string>();
    }
    if(vm.count("transfer-type")) {
        transferType = vm["transfer-type"].as<std::string>();
    }

    // precheck type
    if(socket != "tcp"
            && socket != "uds")
    {
        std::cout<<"ERROR: type \""<<socket<<"\" is unknown. Choose \"tcp\" or \"uds\"."<<std::endl;;
        exit(1);
    }

    // precheck transfer-type
    if(transferType != "stream"
            && transferType != "standalone"
            && transferType != "request")
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
    std::cout<<"--------------------------------------"<<std::endl;

    TestSession testSession(address, port, socket, transferType);

    testSession.runTest();
}
