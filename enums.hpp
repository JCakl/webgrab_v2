//
//  enums.h
//  netApp
//
//  Created by Jan Cakl on 03.03.2022.
//

#ifndef enums_hpp
#define enums_hpp

enum countOfParameters
{
    one = 1,
    two = 2,
    three = 3,
};

enum class roleType
{
    client = 1,
    server = 2,
    unknown = 3,
};

enum class messageType
{
    quit = 1,
    addUrl = 2,
    unknown = 3,
    help = 4,
};

#endif /* enums_hpp */
