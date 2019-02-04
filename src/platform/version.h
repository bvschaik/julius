#ifndef _VERSION_H_
#define _VERSION_H_

#define VERSION_MAJOR 1
#define VERSION_MINOR 0
#define VERSION_PATCH 0
#define VERSION_NUMBER VERSION_MAJOR.VERSION_MINOR.VERSION_PATCH

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#include <string.h>
#include <stdio.h>

#include "git_version.h"

const char* version_information(void)
{
    if(strcmp(git_version_information(), "unknown") == 0)
    {
        return (const char*)TOSTRING(VERSION_NUMBER);
    }
    else
    {
        return git_version_information();
    }
    
}

#endif // _VERSION_H_