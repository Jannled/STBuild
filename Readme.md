# STBuild
## What makes this build system different
### **S**ingle file
The entire source code for this build system is inside a single .cpp for the following reasons:
1. No other dependencies other than the std library (the embedded yaml parser) and a compiler of course. No need to install a third party build-system, Python etc.
2. `gcc -std=c++11 stbuild.cpp` is all you need to compile this build system into an executable (well except for MSVC..). This should make it easily portable to different platforms.

\* Currently the yaml parser is not included in the stbuild.cpp, so the command is a little more involved at the moment, but this is an early prototype and will change in the future (I'm currently compiling with CMake, the system I try to avoid xD )!

### **T**raversial
You might give this build system a defined list of files, or some paths with regex expressions to declare which files you would like to build. 

CMake and Make both need a list of source files, telling them to just look in a specified directory for files with a certain ending is just a nightmare (especially doing this cross platform with Make). Compiling a C/C++ program with multiple files as a newcomer to the language is just madness.

### **B**uild system
And the build description files should be straight forward.
TODO

## Why another build system?
Well there are plenty build systems out there. While some people are upset by [the sheer abundance of build systems](https://www.reddit.com/r/programming/comments/gm1dy/stop_inventing_motherfucking_build_systems/), others are mad at the current selection of build systems. They usually have a hell lot of dependences and require the most complex, unintuitive setup for the most basic tasks (I might write a list sometime to outline my fun experiences).

## stb, this remembers me of something...
First of im not affiliated with [Sean Barret (nothings)](https://github.com/nothings/) in any way. I just appreciate the librarys this guy realesed into the wild and the fact that they usually consist of a single c++ source file. And others seem to like these too, as there are other authors following this idea and they get [actively promoted by Sean Barret](https://github.com/nothings/single_file_libs).

Well if you never heard of Sean Berret and his work, it's a shame and you should definitively [go check it out](https://github.com/nothings/stb).