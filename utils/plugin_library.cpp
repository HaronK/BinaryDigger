/*
 * plugin_library.cpp
 *
 *  Created on: Dec 10, 2015
 *      Author: oleg
 */

#include "plugin_library.h"

template<class F>
static bd_result load_plugin_function(boost::dll::shared_library& sl, const char* name, F* func)
{
    if (sl.has(name))
    {
        *func = sl.get<F>(name);
        return BD_SUCCESS;
    }

    *func = nullptr;
    return BD_ERROR;
}

plugin_library::plugin_library(const char* lib_path)
{
    plugin = {0};

    sl.load(lib_path);

    result = (load_plugin_function(sl, "bd_result_message",    &plugin.result_message)    != BD_SUCCESS ||
              load_plugin_function(sl, "bd_initialize_plugin", &plugin.initialize_plugin) != BD_SUCCESS ||
              load_plugin_function(sl, "bd_finalize_plugin",   &plugin.finalize_plugin)   != BD_SUCCESS ||
              load_plugin_function(sl, "bd_template_name",     &plugin.template_name)     != BD_SUCCESS ||
              load_plugin_function(sl, "bd_apply_template",    &plugin.apply_template)    != BD_SUCCESS ||
              load_plugin_function(sl, "bd_free_template",     &plugin.free_template)     != BD_SUCCESS ||
              load_plugin_function(sl, "bd_get_string_value",  &plugin.get_string_value)  != BD_SUCCESS)
             ? BD_ERROR : BD_SUCCESS;
}

plugin_library::~plugin_library()
{
    sl.unload();
}

bd_result plugin_library::get_plugin(bd_plugin& plugin)
{
    if (BD_SUCCEED(result))
    {
        plugin = this->plugin;
    }
    return result;
}
