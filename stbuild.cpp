#include <process.h>

#include <iostream>
#include <exception>
#include <string>
#include <queue>

#include "yaml/Yaml.hpp"

using namespace Yaml;

/*
// Platform is windows
#if defined _WIN32 || defined __MINGW32__
#pragma message("[STBuild] Detected Windows")

// Platform is Linux
#elif defined(__linux__)
#pragma message("[STBuild] Detected Linux")

// Platform is FreeBSD
#elif defined(__FreeBSD__)
#pragma message("[STBuild] Detected FreeBSD")

// Platform is MAC
#elif defined(__APPLE__) && defined(__MACH__)
#pragma message("[STBuild] Detected Apple")

// Platform is Android
#elif defined(__ANDROID__)
#pragma message("[STBuild] Detected Android")

// Platform is Unixoid
#elif defined(unix) || defined(__unix__) || defined(__unix)
#pragma warn("[STBuild] Detected Unix")

// Unknown Platform
#else 
#error("[STBuild] Unable to get information about the platform STBuild is compiled from!")
#endif
*/


int main(int argc, char** argv)
{
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