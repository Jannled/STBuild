#include <process.h>

#include <iostream>
#include <exception>
#include <string>
#include <queue>

#include "yaml/Yaml.hpp"

using namespace Yaml;

#define DEBUG_STBUILD 1

// Platform is windows
#if defined(_WIN32) || defined(__MINGW32__)
#define OS_WINDOWS
#if defined(DEBUG_STBUILD) 
    #pragma message("[STBuild] Detected Windows") 
#endif

#ifndef MAX_PATH 
    #define MAX_PATH 260
#endif
#define PATH_MAX MAX_PATH

// Platform is Linux
#elif defined(__linux__)
#define OS_LINUX
#if defined(DEBUG_STBUILD) 
    #pragma message("[STBuild] Detected Linux")
#endif

// Platform is FreeBSD
#elif defined(__FreeBSD__)
#if defined(DEBUG_STBUILD) 
    #pragma message("[STBuild] Detected FreeBSD")
#endif

// Platform is MAC
#elif defined(__APPLE__) && defined(__MACH__)
#if defined(DEBUG_STBUILD) 
    #pragma message("[STBuild] Detected Apple")
#endif

// Platform is Android
#elif defined(__ANDROID__)
#if defined(DEBUG_STBUILD) 
    #pragma message("[STBuild] Detected Android")
#endif


// Platform is Unixoid
#elif defined(unix) || defined(__unix__) || defined(__unix)
#if defined(DEBUG_STBUILD) 
    #pragma warn("[STBuild] Detected Unix")
#endif
#pragma warn("[STBuild] Detected Unix")

// Unknown Platform
#else 
#error("[STBuild] Unable to get information about the platform STBuild is compiled from!")
#endif

const char* workDir;

int main(int argc, char** argv)
{  
    std::cout << "Starting program with args: [";
    for(int i=0; i<argc; i++)
    {
        if(i > 0) std::cout << ", ";
        std::cout << argv[i];
    }
    std::cout << "]" << std::endl;

    workDir = argv[0];

    Yaml::Node root;

    // Load and parse the build file using https://github.com/jimmiebergmann/mini-yaml
    try {
        Parse(root, argc > 1 ? argv[1] : "stbuild.yml");
    } catch(const Exception e) {
        std::cerr << "Exception " << e.Type() << ": " << e.what() << std::endl;
        return -1;
    }

    // Generate a list of target names
    std::queue<std::string> targetNames = std::queue<std::string>();

    Node & targets = root["targets"];
    for(auto target = targets.Begin(); target != targets.End(); target++)
    {
        targetNames.push((*target).first);
    }

    // For each target compile all files and link them together
    while (!targetNames.empty())
    {
        Node & files = root["targets"][targetNames.front()]["files"];
        std::cout << targetNames.front() << std::endl;

        for(auto file = files.Begin(); file != files.End(); file++)
        {
            std::string command = "gcc -c ";
            command.append((*file).second.As<std::string>());
            command.append(" -o bin/");
            command.append((*file).second.As<std::string>());
            command.append(".o");

            system(command.c_str());
        }
        targetNames.pop();
    }
    
    // Runs command in shell
    //system("echo Test");
}

const char* toCanonicalPath(const char* path, char* buff)
{
	if(!buff)
		buff = new char[PATH_MAX];

	#ifdef OS_WINDOWS
	_fullpath(buff, path, PATH_MAX); //Maybe GetFullPathName() come in handy sometime
	return buff;

	#elif defined OS_LINUX
	return realpath(path, buff);
	#endif
}