cmake_minimum_required(VERSION 3.10)

# set the project name
project(STBuild)

# add the executable
add_executable(STBuild stbuild.cpp yaml/yaml.cpp)