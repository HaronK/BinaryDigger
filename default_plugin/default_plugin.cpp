/*
 * default_plugin.cpp
 *
 *  Created on: Feb 28, 2014
 *      Author: oleg
 */

#include "default_plugin.h"

extern void register_plugin(bd_string *name, RegisteredTemplates& templ, bool& is_scripter);

std::string __lastError;
RegisteredTemplates registeredTemplates;

bd_result bd_result_message(bd_result result, bd_string msg, bd_u32 msg_size)
{
    if (msg == 0)
        return -1;
    strncpy(msg, __lastError.c_str(), msg_size);
    return BD_SUCCESS;
}

bd_result bd_initialize_plugin(bd_string *name, bd_u32 *templ_count)
{
    if (name == 0)
        return -1;
    if (templ_count == 0)
        return -1;

    bool is_scripter = false;
    try
    {
        register_plugin(name, registeredTemplates, is_scripter);
    }
    catch (const DefaultTemplException& ex)
    {
        __lastError = ex.getMessage();
        return ex.getResult();
    }
    *templ_count = is_scripter ? 0 : registeredTemplates.size();
    return BD_SUCCESS;
}

bd_result bd_template_name(bd_u32 index, bd_string *name)
{
    if (name == 0)
        return -1;
    if (index >= registeredTemplates.size())
        return -1;
    *name = (bd_string) registeredTemplates[index]->getName();
    return BD_SUCCESS;
}

bd_result bd_apply_template(bd_u32 index, bd_templ_blob *blob, bd_item **item, bd_cstring script)
{
    if (blob == 0)
        return -1;
    if (item == 0)
        return -1;
    if (index >= registeredTemplates.size())
        return -1;
    try
    {
        *item = registeredTemplates[index]->applyTemplate(blob, script);
    }
    catch (const DefaultTemplException& ex)
    {
        __lastError = ex.getMessage();
        return ex.getResult();
    }
    return BD_SUCCESS;
}

bd_result bd_free_template(bd_u32 index, bd_item *item)
{
    if (item == 0)
        return -1;
    if (index >= registeredTemplates.size())
        return -1;
    try
    {
        registeredTemplates[index]->freeTemplate(item);
    }
    catch (const DefaultTemplException& ex)
    {
        __lastError = ex.getMessage();
        return ex.getResult();
    }
    return BD_SUCCESS;
}

bd_result bd_finalize_plugin()
{
    for (RegisteredTemplates::iterator iter = registeredTemplates.begin();
            iter != registeredTemplates.end(); ++iter)
        delete *iter;
    return BD_SUCCESS;
}
