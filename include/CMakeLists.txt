
set(CMAKE_INCLUDE_CURRENT_DIR ON)

file(GLOB headers "*.h")

# installation
install(FILES ${headers} DESTINATION include)
