#include <process.h>

#include <iostream>
#include <exception>
#include <string>
#include <queue>

#include <filesystem>

#include "yaml/Yaml.hpp"

using namespace Yaml;

/* Struct definitions */
struct target_settings {
    const char* buildOutDir;
    bool debug;
    const char* execName;
};

/* Function definitions */
int calGCC(std::queue<std::string>& sourceFilesC, std::queue<std::string>& sourceFilesCPP, struct target_settings& targetSettings);
int compileGCC(std::string& inputFile, std::queue<std::string>& linkObjects, struct target_settings& targetSettings, bool cpp);
int linkGCC(std::queue<std::string>& linkObjects, struct target_settings& targetSettings);

int main(int argc, char** argv)
{  
    std::cout << "[STBuild] Starting program with args: [";
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
    while(!targetNames.empty())
    {
        struct target_settings targetSettings;
        targetSettings.buildOutDir  = "bin/";
        targetSettings.debug        = true;
        targetSettings.execName     = root["targets"][targetNames.front()]["name"].As<std::string>().c_str();

        std::cout << "[STBuild] Working on target \"" << targetNames.front() << "\"" << std::endl;

        // List of objects that need compilation
        std::queue<std::string> sourceFilesC = std::queue<std::string>();
        std::queue<std::string> sourceFilesCPP = std::queue<std::string>();

        // Add each C source file to the uncompiled list
        Node & cfiles = root["targets"][targetNames.front()]["cfiles"];
        if(cfiles.IsSequence())
        {
            for(auto file = cfiles.Begin(); file != cfiles.End(); file++)
                sourceFilesC.push((*file).second.As<std::string>());
        }

        // Add each C++ source file to the uncompiled list
        Node & pfiles = root["targets"][targetNames.front()]["c++files"];
        if(pfiles.IsSequence())
        {
            for(auto file = pfiles.Begin(); file != pfiles.End(); file++)
                sourceFilesCPP.push((*file).second.As<std::string>());
        }

        // Let the compiler backend do its magic
        calGCC(sourceFilesC, sourceFilesCPP, targetSettings);

        targetNames.pop();
    }
}

int compileAndLink(std::queue<std::string>& sourceFiles, struct target_settings& targetSettings)
{
    return 0;
}

int calGCC(std::queue<std::string>& sourceFilesC, std::queue<std::string>& sourceFilesCPP, struct target_settings& targetSettings)
{
    std::cout << "[STBuild] compiling target with gcc" << std::endl;

    // List of objects that need linking
    std::queue<std::string> linkObjects = std::queue<std::string>();

    // For each C/C++ source file do compilation
    while (!sourceFilesC.empty())
    {
        compileGCC(sourceFilesC.front(), linkObjects, targetSettings, false);
        sourceFilesC.pop();
    }

    // For each C/C++ source file do compilation
    while (!sourceFilesCPP.empty())
    {
        compileGCC(sourceFilesCPP.front(), linkObjects, targetSettings, true);
        sourceFilesCPP.pop();
    }

    // Link all object files from previous step into an executable
    linkGCC(linkObjects, targetSettings);

    return 0;
}

int compileGCC(std::string& inputFile, std::queue<std::string>& linkObjects, struct target_settings& targetSettings, bool cpp)
{
    // Build object file path from binDir, sourceFileName and the ending .o
    std::string objectFileName = targetSettings.buildOutDir;
    objectFileName.append(inputFile);
    objectFileName.append(".o");

    // Add above created file to files that need linking
    linkObjects.push(objectFileName);

    // Get parent directory of object file to build directory structure
    std::filesystem::create_directories(std::filesystem::path(objectFileName).parent_path());

    // Append and run compile command
    std::string command = cpp ? "g++ -c " : "gcc -c " ;
    if(targetSettings.debug) command.append("-O0 -g3 ");
    command.append(inputFile);
    command.append(" -o ");
    command.append(objectFileName);

    std::cout << command << std::endl;
    return system(command.c_str());
}

int linkGCC(std::queue<std::string>& linkObjects, struct target_settings& targetSettings)
{
    // Append and run link command
    std::string command = "gcc";
    if(targetSettings.debug) command.append(" -O0 -Wl,-O0 ");
    while(!linkObjects.empty())
    {
        command.append(" ");
        command.append(linkObjects.front());
        linkObjects.pop();
    }
    command.append(" -o ");
    command.append(targetSettings.buildOutDir);
    command.append(targetSettings.execName);

    // Just make sure the exe postfix is added on windows
    #if defined _WIN32 || defined __MINGW32__ || defined _WIN64
        command.append(".exe");
    #endif

    std::cout << command << std::endl;
    return system(command.c_str());
}