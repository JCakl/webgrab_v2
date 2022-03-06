//
//  main.cpp
//  netApp
//
//  Created by Jan Cakl on 02.03.2022.
//

#include <iostream>
#include <stdlib.h>
#include <algorithm>
#include "retTypes.hpp"
#include "client.hpp"
#include "server.hpp"

#define UNKNOWN_PARAM 1

char* getUrlAdress(char ** begin, char ** end, const std::string & option)
{
    char ** it = std::find(begin, end, option);
    if (it != end && ++it != end)
    {
        return *it;
    }
    return 0;
}

bool paramExists(char** begin, char** end, const std::string& option)
{
    return std::find(begin, end, option) != end;
}

void printUnkParam()
{
    std::cout << "Unknown combination of parameter" << std::endl;
}

std::pair<roleType, messageType> checkParameters(char **argv, int argc, std::string& urlAdress){
    // ** Parameters check **
    switch (argc) {
        case countOfParameters::one: {
            return { roleType::server, messageType::unknown };
        }
        case countOfParameters::two: {
            if(paramExists(argv, argv+argc, "-h"))
            {
                return { roleType::client, messageType::help };
            }else if(paramExists(argv, argv+argc, "-s")){
                return { roleType::client, messageType::quit };
            }else{
                printUnkParam();
                return { roleType::unknown, messageType::unknown };
            }
            break;
        }
        
        case countOfParameters::three: {
            if(paramExists(argv, argv+argc, "-d")){
                urlAdress = getUrlAdress(argv, argv + argc, "-d");
                
                if( urlAdress == "" ){
                    printUnkParam();
                }
                return {roleType::client, messageType::addUrl};
            }else{
                printUnkParam();
            }
            break;
        }
        default:
            printUnkParam();
            break;
    }
    return {roleType::unknown, messageType::unknown};
}

int main(int argc, char * argv[]) {

    std::string urlAdress="";
    
    auto element = checkParameters(argv, argc, urlAdress);
    
    switch (element.first) {
        case roleType::server:
            serverApp();
            break;
        case roleType::client:
            clientApp(element.second, urlAdress);
            break;
        case roleType::unknown:
            return UNKNOWN_PARAM;
        default:
            break;
    }
    
    return EXIT_SUCCESS;
}
