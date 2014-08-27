cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

# ----------------------------------------------------------------------------------------

project(BinaryDigger)

#set(CMAKE_USER_MAKE_RULES_OVERRIDE CMakeClang.txt)

add_definitions(-std=c++0x)
#add_definitions(-stdlib=libc++)

#set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -v -E")

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR})

find_package(Poco)

if(NOT Poco_FOUND)
    message(FATAL_ERROR "Cannot find Poco package")
endif()

# ----------------------------------------------------------------------------------------
set(BinaryDigger_PackageFolder "${BinaryDigger_BINARY_DIR}/package/")
set(BinaryDigger_PluginsFolder "${BinaryDigger_PackageFolder}/plugins/")

execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${BinaryDigger_PluginsFolder})
# ----------------------------------------------------------------------------------------

set(CMAKE_INCLUDE_CURRENT_DIR ON)
include_directories("${BinaryDigger_SOURCE_DIR}/include" ${Poco_INCLUDE_DIR})

link_directories(${Poco_LIBRARY_DIRS})

# Solving problem with linking unused static library.
# Source: http://stackoverflow.com/questions/17470350/c-cmake-how-to-add-a-linker-flag-to-an-unused-library-when-the-library-is-sp
SET(bd_default_plugin_FORCE_LINK -Wl,--whole-archive bd_default_plugin -Wl,--no-whole-archive)

# Build subdirectories.
add_subdirectory(include)
add_subdirectory(default_plugin)
add_subdirectory(gui)
add_subdirectory(plugins)
add_subdirectory(scripters)
add_subdirectory(tests)
add_subdirectory(tools)
add_subdirectory(utils)
add_subdirectory(server) # "${BinaryDigger_BINARY_DIR}/server"

# installation
#install(FILES MathFunctions.h DESTINATION include)
