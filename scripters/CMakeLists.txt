
# ----------------------------------------------------------------------------------------

find_package(Lua REQUIRED)

if(NOT LUA_FOUND)
    message(FATAL_ERROR "Cannot find Lua package")
endif()

#message("LUA_LIBRARIES:      ${LUA_LIBRARIES}")
#message("LUA_INCLUDE_DIR:    ${LUA_INCLUDE_DIR}")
#message("LUA_VERSION_STRING: ${LUA_VERSION_STRING}")
#message("LUA_VERSION_MAJOR:  ${LUA_VERSION_MAJOR}")
#message("LUA_VERSION_MINOR:  ${LUA_VERSION_MINOR}")
#message("LUA_VERSION_PATCH:  ${LUA_VERSION_PATCH}")

include_directories(${LUA_INCLUDE_DIR})

# ----------------------------------------------------------------------------------------

find_package(Luabind)

if(NOT LUABIND_FOUND)
    message(FATAL_ERROR "Cannot find Luabind package")
endif()

#message("LUABIND_DEFINITIONS:  ${LUABIND_DEFINITIONS}")
#message("LUABIND_INCLUDE_DIRS: ${LUABIND_INCLUDE_DIRS}")
#message("LUABIND_LIBRARIES:    ${LUABIND_LIBRARIES}")

include_directories(${LUABIND_INCLUDE_DIRS})

# ----------------------------------------------------------------------------------------

include_directories("${BinaryDigger_SOURCE_DIR}/default_plugin")
link_directories("${BinaryDigger_BINARY_DIR}/default_plugin")

file(GLOB plugins "*.cpp")
foreach(plugin_file ${plugins})
    get_filename_component(plugin_name ${plugin_file} NAME_WE)
    add_library(${plugin_name} SHARED ${plugin_file})
    
    target_link_libraries(${plugin_name} PocoFoundation)
    
    add_dependencies(${plugin_name} bd_default_plugin)
    target_link_libraries(${plugin_name} ${bd_default_plugin_FORCE_LINK})
    
    # installation
    install(TARGETS ${plugin_name} DESTINATION bin/plugins)
    
    add_custom_command(TARGET ${plugin_name} POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy
                           $<TARGET_FILE:${plugin_name}>
                           "${BinaryDigger_GuiBuildFolder}"
                       COMMENT "${plugin_name} copied to ${HierarchDiggerGuiBuildPath}"
                      )
endforeach()

if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64")
    set_target_properties(LuaScripter PROPERTIES COMPILE_FLAGS -fPIC)
endif("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64")

target_link_libraries(LuaScripter ${LUABIND_LIBRARIES})

# ----------------------------------------------------------------------------------------