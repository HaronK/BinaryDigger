
set(CMAKE_PREFIX_PATH "$ENV{QT_HOME}/5.5/gcc_64/")

find_package(Qt5Widgets QUIET)

if(NOT Qt5Widgets_FOUND)
    message(WARNING "Cannot find Qt5Widgets package. Qt GUI will not be generated.")
    return()
endif()

set(CMAKE_AUTOMOC ON)

# Build subdirectories.
add_subdirectory(BinaryDigger)
