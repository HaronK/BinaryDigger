
file(GLOB sources "*.cpp")

add_library(bd_utils STATIC ${sources})

#if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64")
#    set_target_properties(bd_utils PROPERTIES COMPILE_FLAGS -fPIC)
#endif("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64")
