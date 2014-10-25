cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

# ----------------------------------------------------------------------------------------

project(BinaryDigger)

#set(CMAKE_USER_MAKE_RULES_OVERRIDE CMakeClang.txt)

add_definitions(-std=c++11)
#add_definitions(-stdlib=libc++)

#set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -v -E")

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR})

cmake_policy(SET CMP0015 NEW)

# ----------------------------------------------------------------------------------------
# Packages
# ----------------------------------------------------------------------------------------

# Boost
set(Boost_USE_STATIC_LIBS    OFF) 
set(Boost_USE_MULTITHREADED  ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 
find_package(Boost 1.56.0 REQUIRED COMPONENTS program_options) 

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS}) 
endif()

# Poco
find_package(Poco REQUIRED)

if(NOT Poco_FOUND)
    message(FATAL_ERROR "Cannot find Poco package")
endif()


# ----------------------------------------------------------------------------------------
set(BD_PACKAGE_FOLDER "${BinaryDigger_BINARY_DIR}/package/")
set(BD_PLUGINS_FOLDER "${BD_PACKAGE_FOLDER}/plugins/")
set(BD_SCRIPTS_FOLDER "${BD_PACKAGE_FOLDER}/scripts/")

execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${BD_PLUGINS_FOLDER})
# ----------------------------------------------------------------------------------------

set(CMAKE_INCLUDE_CURRENT_DIR ON)
include_directories("include"
                    "external/boost_endian/include"
                    "utils"
                    "default_plugin"
                     ${Poco_INCLUDE_DIR})

link_directories("default_plugin" ${Poco_LIBRARY_DIRS})

# Solving problem with linking unused static library.
# Source: http://stackoverflow.com/questions/17470350/c-cmake-how-to-add-a-linker-flag-to-an-unused-library-when-the-library-is-sp
SET(bd_default_plugin_FORCE_LINK -Wl,--whole-archive bd_default_plugin -Wl,--no-whole-archive)

# Build subdirectories.
add_subdirectory(external)
add_subdirectory(include)
add_subdirectory(default_plugin)
add_subdirectory(gui)
add_subdirectory(plugins)
add_subdirectory(scripters)
add_subdirectory(tests)
add_subdirectory(tools)
add_subdirectory(utils)
add_subdirectory(server)

# installation
#install(FILES MathFunctions.h DESTINATION include)
