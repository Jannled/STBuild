#include <process.h>

#include <iostream>
#include <exception>
#include <string>
#include <queue>

#include <filesystem>

#include "yaml/Yaml.hpp"

using namespace Yaml;

/* Function definitions */
int compile(std::string& inputFile, std::string& outputDir, std::queue<std::string>& linkObjects);
int link(std::queue<std::string>& linkObjects, std::string& outputDir, std::string& execName);

int main(int argc, char** argv)
{  
    std::cout << "Starting program with args: [";
    for(int i=0; i<argc; i++)
    {
        if(i > 0) std::cout << ", ";
        std::cout << argv[i];
    }
    std::cout << "]" << std::endl;

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

    std::string binDir = "bin/";

    // For each target compile all files and link them together
    while (!targetNames.empty())
    {
        Node & files = root["targets"][targetNames.front()]["files"];
        std::cout << targetNames.front() << std::endl;


        // List of objects that need linking
        std::queue<std::string> linkObjects = std::queue<std::string>();

        // For each C/C++ source file do compilation
        for(auto file = files.Begin(); file != files.End(); file++)
        {
            // Input file path
            std::string fileName = (*file).second.As<std::string>();

            compile(fileName, binDir, linkObjects);
        }

        // Link all object files from previous step into an executable
        std::string execName = root["targets"][targetNames.front()]["name"].As<std::string>();
        link(linkObjects, binDir, execName);

        targetNames.pop();
    }
}

int compileAndLink()
{
    
}

int compile(std::string& inputFile, std::string& outputDir, std::queue<std::string>& linkObjects)
{
    // Build object file path
    std::string objectFileName = outputDir;
    objectFileName.append(inputFile);
    objectFileName.append(".o");

    linkObjects.push(objectFileName);

    // Build object file
    std::filesystem::path objectFile = std::filesystem::path(objectFileName);

    // Create directorys
    std::filesystem::create_directories(objectFile.parent_path());

    // Append and run compile command
    std::string command = "gcc -c ";
    command.append(inputFile);
    command.append(" -o ");
    command.append(objectFileName);

    return system(command.c_str());
}

int link(std::queue<std::string>& linkObjects, std::string& outputDir, std::string& execName) 
{
    // Append and run link command
    std::string command = "gcc";
    while(!linkObjects.empty())
    {
        command.append(" ");
        command.append(linkObjects.front());
        linkObjects.pop();
    }
    command.append(" -o ");
    command.append(outputDir);
    command.append(execName);

    // Just make sure the exe postfix is added on windows
    #if defined _WIN32 || defined __MINGW32__
        command.append(".exe");
    #endif

    return system(command.c_str());
}