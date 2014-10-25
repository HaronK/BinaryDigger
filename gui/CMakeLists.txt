
set(CMAKE_PREFIX_PATH "$ENV{QT_HOME}/5.3/gcc_64/")

find_package(Qt5Widgets REQUIRED)

set(CMAKE_AUTOMOC ON)

# Build subdirectories.
add_subdirectory(BinaryDigger)
