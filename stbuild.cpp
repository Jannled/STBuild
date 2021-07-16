#include <process.h>

#include <iostream>
#include <exception>
#include <string>
#include <queue>
#include <string.h>

#include <filesystem>

#include "yaml/Yaml.hpp"

using namespace Yaml;

// --- Command ---
// g++ -std=c++17 -Iyaml/ stbuild.cpp yaml/Yaml.cpp -o ../stbuild


/* Struct definitions */
struct target_settings {
	const char* buildOutDir;
	bool debug;
	const char* execName;
	std::queue<std::string>* sourceIncludes;
};

/* Compiler Backend Function definitions */
int calGCC(std::queue<std::string>& sourceFilesCPP, std::queue<std::string>& sourceFilesCXX, struct target_settings& targetSettings);
int compileGCC(std::string& inputFile, std::string& includes, std::queue<std::string>& linkObjects, struct target_settings& targetSettings, bool cpp);
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

	// Load and parse the build file using https://github.com/jimmiebergmann/mini-yaml
	Yaml::Node root;
	try {
		Parse(root, argc > 1 ? argv[1] : "stbuild.yml");
	} catch(const Exception e) {
		std::cerr << "Exception " << e.Type() << ": " << e.what() << std::endl;
		return -1;
	}

	// Generate a list of target names
	std::queue<std::string> targetNames = std::queue<std::string>();

	Node & targets = root["targets"];
	if(argc < 2 || strcmp(argv[1], "all"))
	{
		for(auto target = targets.Begin(); target != targets.End(); target++)
			targetNames.push((*target).first);
	}
	else
	{
		std::cerr << "Test" << std::endl;

		if(targets[argv[1]].IsNone())
			targetNames.push(argv[1]);
		else {
			std::cerr << "[STBuild] Could not find target \"" << argv[1] << "\"!" << std::endl;
			return -1;
		} 
	}

	// For each target compile all files and link them together
	while(!targetNames.empty())
	{
		struct target_settings targetSettings;
		targetSettings.buildOutDir  = "bin/";
		targetSettings.debug        = true;
		targetSettings.execName     = root["targets"][targetNames.front()]["name"].As<std::string>().c_str();

		std::cout << "[STBuild] Working on target \"" << targetNames.front() << "\"" << std::endl;

		// List of objects that need compilation
		std::queue<std::string> sourceFilesCPP = std::queue<std::string>();
		std::queue<std::string> sourceFilesCXX = std::queue<std::string>();
		std::queue<std::string> sourceIncludes = std::queue<std::string>();

		// Add each C source file to the uncompiled list
		Node & cfiles = root["targets"][targetNames.front()]["cppfiles"];
		if(cfiles.IsSequence())
		{
			for(auto file = cfiles.Begin(); file != cfiles.End(); file++)
				sourceFilesCPP.push((*file).second.As<std::string>());
		}

		// Add each C++ source file to the uncompiled list
		Node & pfiles = root["targets"][targetNames.front()]["cxxfiles"];
		if(pfiles.IsSequence())
		{
			for(auto file = pfiles.Begin(); file != pfiles.End(); file++)
				sourceFilesCXX.push((*file).second.As<std::string>());
		}

		Node & ifiles = root["targets"][targetNames.front()]["includes"];
		if(ifiles.IsSequence())
		{
			for(auto file = ifiles.Begin(); file != ifiles.End(); file++)
				sourceIncludes.push((*file).second.As<std::string>());
		}

		targetSettings.sourceIncludes = &sourceIncludes;

		// Let the compiler backend do its magic
		calGCC(sourceFilesCPP, sourceFilesCXX, targetSettings);

		targetNames.pop();
	}
}

/**
 * Method to be implemented for each compiler backend
 * @param sourceFilesCPP
 * @param sourceFilesCXX
 * @param targetSettings
 */
int compileAndLink(std::queue<std::string>& sourceFilesCPP, std::queue<std::string>& sourceFilesCXX, struct target_settings& targetSettings)
{
	return 0;
}

int calGCC(std::queue<std::string>& sourceFilesCPP, std::queue<std::string>& sourceFilesCXX, struct target_settings& targetSettings)
{
	std::cout << "[STBuild] compiling target with gcc" << std::endl;

	// List of objects that need linking
	std::queue<std::string> linkObjects = std::queue<std::string>();

	// Build file tree
	std::filesystem::recursive_directory_iterator dirpos(".");

	
	// Append each include file with command argument to string
	std::string includes = "";
	while (!targetSettings.sourceIncludes->empty())
	{
		includes.append("-I");
		includes.append(targetSettings.sourceIncludes->front());
		includes.append(" ");
		targetSettings.sourceIncludes->pop();
	}

	// For each C source file do compilation
	while (!sourceFilesCPP.empty())
	{
		compileGCC(sourceFilesCPP.front(), includes, linkObjects, targetSettings, false);
		sourceFilesCPP.pop();
	}

	// For each C++ source file do compilation
	while (!sourceFilesCXX.empty())
	{
		compileGCC(sourceFilesCXX.front(), includes, linkObjects, targetSettings, true);
		sourceFilesCXX.pop();
	}

	// Link all object files from previous step into an executable
	linkGCC(linkObjects, targetSettings);

	return 0;
}

int compileGCC(std::string& inputFile, std::string& includes, std::queue<std::string>& linkObjects, struct target_settings& targetSettings, bool cpp)
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
	command.append(includes);
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

// "gcc -c ${debugString} ${inputFile} -o ${objectFile}"

// "gcc -O0 -Wl,-O0 ${linkFiles}" -o ${execFile}"