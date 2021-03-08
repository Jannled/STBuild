#include <process.h>

#include <iostream>
#include <exception>
#include <string>
#include <stack>

#include "yaml/Yaml.hpp"

using namespace Yaml;

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
    std::stack<std::string> targetNames = std::stack<std::string>();

    Node & targets = root["targets"];
    for(auto target = targets.Begin(); target != targets.End(); target++)
    {
        targetNames.push((*target).first);
    }

    // For each target compile all files and link them together
    while (!targetNames.empty())
    {
        std::cout << targetNames.top() << std::endl;
        targetNames.pop();
    }
    
    // Runs command in shell
    //system("echo Test");
}