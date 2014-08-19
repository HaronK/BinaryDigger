
set(CMAKE_PREFIX_PATH "~/Programs/Qt/5.3/gcc_64/")

find_package(Qt5Widgets)

include_directories("${BinaryDigger_BINARY_DIR}/gui/BinaryDigger" "${BinaryDigger_SOURCE_DIR}/gui/BinaryDigger")

set(CMAKE_AUTOMOC ON)

# Build subdirectories.
add_subdirectory(BinaryDigger)
