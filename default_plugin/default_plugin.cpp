/*
 * default_plugin.cpp
 *
 *  Created on: Feb 28, 2014
 *      Author: oleg
 */

#include "default_plugin.h"

#include <boost/format.hpp>

extern void register_plugin(std::string &name, bool &is_scripter);

PluginContext::array_type PluginContext::registeredTemplates;
std::string PluginContext::error;

bd_result bd_result_message(bd_result result, bd_string msg, bd_u32 msg_size)
{
    bd_check_not_null(msg);

    auto len = std::min((size_t) msg_size - 1, PluginContext::getError().length());
    strncpy(msg, PluginContext::getError().c_str(), len);
    msg[len] = '\0';

    return BD_SUCCESS;
}

bd_result bd_initialize_plugin(bd_string name, bd_u32 name_size, bd_u32 *templ_count)
{
    bd_check_not_null(name);
    bd_check_not_null(templ_count);

    auto is_scripter = false;

    try
    {
        auto plugin_name = std::string();
        register_plugin(plugin_name, is_scripter);
        bd_check_and_return(name_size >= plugin_name.size() - 1, "Plugin name buffer is too small");

        strncpy(name, plugin_name.c_str(), plugin_name.size());
        name[plugin_name.size()] = '\0';
    }
    catch (const BlockTemplException& ex)
    {
        PluginContext::setError(ex.getMessage());
        return ex.getResult();
    }

    *templ_count = is_scripter ? 0 : PluginContext::getTemplatesCount();

    return BD_SUCCESS;
}

bd_result bd_finalize_plugin()
{
    PluginContext::freeTemplates();
    return BD_SUCCESS;
}

bd_result bd_template_name(bd_u32 index, bd_string name, bd_u32 name_size)
{
    bd_check_not_null(name);
    bd_check_index(index);

    auto templ_name = PluginContext::getTemplate(index)->getName();
    bd_check_and_return(name_size >= templ_name.size() - 1, "Template name buffer is too small");

    strncpy(name, templ_name.c_str(), templ_name.size());
    name[templ_name.size()] = '\0';

    return BD_SUCCESS;
}

// TODO: specify properties for the top level element
bd_result bd_apply_template(bd_u32 index, bd_block_io *block_io, bd_block **block, bd_cstring script)
{
    bd_check_not_null(block_io);
    bd_check_not_null(block);
    bd_check_index(index);

    auto result = bd_result{BD_SUCCESS};
    try
    {
        auto err = std::string();
        result = PluginContext::getTemplate(index)->applyTemplate(block_io, script, block, err);
        PluginContext::setError(err);
    }
    catch (const BlockTemplException& ex)
    {
        PluginContext::setError(ex.getMessage());
        result = ex.getResult();
    }
    return result;
}

bd_result bd_free_template(bd_u32 index, bd_block *block)
{
    bd_check_not_null(block);
    bd_check_index(index);

    auto result = bd_result{BD_SUCCESS};
    try
    {
        auto err = std::string();
        result = PluginContext::getTemplate(index)->freeTemplate(block, err);
        PluginContext::setError(err);
    }
    catch (const BlockTemplException& ex)
    {
        PluginContext::setError(ex.getMessage());
        result = ex.getResult();
    }
    return result;
}

bd_result bd_get_string_value(bd_block *block, bd_string buf, bd_u32 size)
{
    bd_check_not_null(block);
    bd_check_not_null(buf);

    auto templ = (BlockTemplBase *) block;
    auto value = templ->to_string();

    if (value.size() + 1 > size)
    {
        PluginContext::setError((boost::format("String buffer for the value is too small. Expected at least %1% but was %2%")
                                               % (value.size() + 1) % size).str());
        return BD_ERROR;
    }

    strncpy(buf, value.c_str(), value.size());
    buf[value.size()] = 0;

    return BD_SUCCESS;
}
