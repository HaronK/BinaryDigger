
enable_testing()

file(GLOB tests "*Test.cpp")
foreach(test_file ${tests})
    get_filename_component(test_name ${test_file} NAME_WE)
    add_executable(${test_name} ${test_file})
    #target_link_libraries(${test_name} PocoFoundation)
    target_link_libraries(${test_name} bd_utils lua dl ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY})
    
    add_test(NAME bd_${test_name} COMMAND ${test_name})
    
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
