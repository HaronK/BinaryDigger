
include_directories("${BinaryDigger_SOURCE_DIR}/core")

file(GLOB tools "*.cpp")
foreach(tool_file ${tools})
    get_filename_component(tool_name ${tool_file} NAME_WE)
    add_executable(${tool_name} ${tool_file})
    target_link_libraries(${tool_name} PocoFoundation PocoUtil)
    
#    file(GLOB plugins "../plugins/*.cpp")
#    foreach(plugin_file ${plugins})
#        get_filename_component(plugin_name ${plugin_file} NAME_WE)
#        
#        set(target_test_name ${plugin_name}_${test_name})
#        add_executable(${target_test_name} ${test_file})
#
#        add_dependencies(${target_test_name} ${plugin_name})
#        target_link_libraries(${target_test_name} ${plugin_name}_s)
#    
        # installation
#        install(TARGETS ${target_test_name} DESTINATION bin/test)
#    endforeach()
endforeach()
