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
            "address to connect"
        )
        (
            "port,p",
            argParser::value<uint16_t>(),
            "port where to listen"
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
    std::string address = "";
    if(vm.count("address")) {
        address = vm["address"].as<std::string>();
    }
    if(vm.count("ports")) {
        port = vm["port"].as<uint16_t>();
    }

    TestSession testSession(address, port);

    while(true)
    {

    }
}
