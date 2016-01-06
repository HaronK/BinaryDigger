# - Find the Poco includes and libraries.
# The following variables are set if Poco is found.  If Poco is not
# found, Poco_FOUND is set to false.
#  Poco_FOUND        - True when the Poco include directory is found.
#  Poco_INCLUDE_DIRS - the path to where the poco include files are.
#  Poco_LIBRARY_DIRS - The path to where the poco library files are.
#  Poco_BINARY_DIRS  - The path to where the poco dlls are.

# ----------------------------------------------------------------------------
# If you have installed Poco in a non-standard location.
# Then you have three options. 
# In the following comments, it is assumed that <Your Path>
# points to the root directory of the include directory of Poco. e.g
# If you have put poco in C:\development\Poco then <Your Path> is
# "C:/development/Poco" and in this directory there will be two
# directories called "include" and "lib".
# 1) After CMake runs, set Poco_INCLUDE_DIR to <Your Path>/poco<-version>
# 2) Use CMAKE_INCLUDE_PATH to set a path to <Your Path>/poco<-version>. This will allow find_path()
#    to locate Poco_INCLUDE_DIR by utilizing the PATH_SUFFIXES option. e.g.
#    set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "<Your Path>/include")
# 3) Set an environment variable called ${POCO_ROOT} that points to the root of where you have
#    installed Poco, e.g. <Your Path>. It is assumed that there is at least a subdirectory called
#    Foundation/include/Poco in this path.
#
# Note:
#  1) If you are just using the poco headers, then you do not need to use
#     Poco_LIBRARY_DIRS in your CMakeLists.txt file.
#  2) If Poco has not been installed, then when setting Poco_LIBRARY_DIRS
#     the script will look for /lib first and, if this fails, then for /stage/lib.
#
# Usage:
# In your CMakeLists.txt file do something like this:
# ...
# # Poco
# FIND_PACKAGE(Poco)
# ...
# INCLUDE_DIRECTORIES(${Poco_INCLUDE_DIRS})
# LINK_DIRECTORIES(${Poco_LIBRARY_DIRS})
#
# In Windows, we make the assumption that, if the Poco files are installed, the default directory
# will be C:\poco or C:\Program Files\Poco.

set(POCO_INCLUDE_PATH_DESCRIPTION "top-level directory containing the poco include directories. E.g /usr/local/include/poco-1.2.1 or c:\\poco\\include\\poco-1.2.1")
set(POCO_INCLUDE_DIR_MESSAGE "Set the Poco_INCLUDE_DIR cmake cache entry to the ${POCO_INCLUDE_PATH_DESCRIPTION}")
set(POCO_LIBRARY_PATH_DESCRIPTION "top-level directory containing the poco libraries.")
set(POCO_LIBRARY_DIR_MESSAGE "Set the Poco_LIBRARY_DIR cmake cache entry to the ${POCO_LIBRARY_PATH_DESCRIPTION}")


set(POCO_DIR_SEARCH $ENV{POCO_ROOT})
if(POCO_DIR_SEARCH)
  file(TO_CMAKE_PATH ${POCO_DIR_SEARCH} POCO_DIR_SEARCH)
endif(POCO_DIR_SEARCH)


# Add in some path suffixes. These will have to be updated whenever a new Poco version comes out.
set(SUFFIX_FOR_LIBRARY_PATH
  lib
  )

set(Poco_FOUND 0)

#
# Look for an installation.
#
find_path(Poco_INCLUDE_DIR Poco/Foundation.h PATH_SUFFIXES include PATHS

  # Look in other places.
  ${POCO_DIR_SEARCH}

  # Help the user find it if we cannot.
  DOC "The ${POCO_INCLUDE_DIR_MESSAGE}"
)

if(Poco_INCLUDE_DIR) 
  set(Poco_FOUND 1)
endif(Poco_INCLUDE_DIR)

set(POCO_MODULES
  Foundation
  Util
  XML
  Net
  NetSSL
  Zip
  Data
  DataSQLite
  )

set(CmakeFindLibrarySuffixes ${CMAKE_FIND_LIBRARY_SUFFIXES})

set(lib_dirs)
set(lib_dirs_debug)

foreach(module ${POCO_MODULES})
  if(UNIX)
    set(CMAKE_FIND_LIBRARY_SUFFIXES ${CmakeFindLibrarySuffixes})
  endif(UNIX)

  set(LibName Poco_${module}_LIBRARY)
  set(lib lib-NOTFOUND)
  set(lib_debug lib_debug-NOTFOUND)

  find_library(lib Poco${module} PATH_SUFFIXES lib PATHS ${POCO_DIR_SEARCH})
  find_library(lib_debug Poco${module}d PATH_SUFFIXES lib PATHS ${POCO_DIR_SEARCH})

  get_filename_component(lib_dir ${lib} PATH)
  get_filename_component(lib_dir_debug ${lib_debug} PATH)

  list(APPEND lib_dirs ${lib_dir})
  list(APPEND lib_dirs_debug ${lib_dir_debug})

  set(LibMessage "Poco ${module} Library")

  if(lib AND lib_debug)
    set(${LibName} optimized ${lib} debug ${lib_debug} CACHE STRING ${LibMessage})
  elseif(lib)
    set(${LibName} ${lib} CACHE STRING ${LibMessage})
  elseif(lib_debug)
    set(${LibName} ${lib_debug} CACHE STRING ${LibMessage})
  endif(lib AND lib_debug)

  If(UNIX)
    set(CMAKE_FIND_LIBRARY_SUFFIXES .a)
  endif(UNIX)

  find_library(lib NAMES Poco${module} Poco${module}mt PATH_SUFFIXES lib PATHS ${POCO_DIR_SEARCH})
  find_library(lib_debug Poco${module}d Poco${module}mtd PATH_SUFFIXES lib PATHS ${POCO_DIR_SEARCH})

  set(LibMessage "Poco ${module} Static Library")

  if(lib AND lib_debug)
    set(${LibName}_STATIC optimized ${lib} debug ${lib_debug} CACHE STRING ${LibMessage})
  elseif(lib)
    set(${LibName}_STATIC ${lib} CACHE STRING ${LibMessage})
  elseif(lib_debug)
    set(${LibName}_STATIC ${lib_debug} CACHE STRING ${LibMessage})
  endif(lib AND lib_debug)

  get_filename_component(lib_dir ${lib} PATH)
  get_filename_component(lib_dir_debug ${lib_debug} PATH)

  list(APPEND lib_dirs ${lib_dir})
  list(APPEND lib_dirs_debug ${lib_dir_debug})

  set(lib lib-NOTFOUND)
  set(lib_debug lib_debug-NOTFOUND)
endforeach(module)

set(CMAKE_FIND_LIBRARY_SUFFIXES ${CmakeFindLibrarySuffixes})

list(REMOVE_DUPLICATES lib_dirs)
list(REMOVE_DUPLICATES lib_dirs_debug)

set(Poco_LIBRARY_DIRS ${lib_dirs})
set(Poco_LIBRARY_DIRS_DEBUG ${lib_dirs_debug})

find_program(Poco_BINARIES NAMES cpspc f2cpsp PATHS ${POCO_DIR_SEARCH}
  PATH_SUFFIXES bin)

if(Poco_BINARIES)
  get_filename_component(Poco_BINARY_DIR ${Poco_BINARIES} PATH)
endif(Poco_BINARIES)

if(NOT Poco_FOUND)
  if(NOT Poco_FIND_QUIETLY)
    message(STATUS "Poco was not found. ${POCO_DIR_MESSAGE}")
  else(NOT Poco_FIND_QUIETLY)
    if(Poco_FIND_REQUIRED)
      message(FATAL_ERROR "Poco was not found. ${POCO_DIR_MESSAGE}")
    endif(Poco_FIND_REQUIRED)
  endif(NOT Poco_FIND_QUIETLY)
endif(NOT Poco_FOUND)
