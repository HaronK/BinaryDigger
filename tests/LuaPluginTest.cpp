/*
 * PluginTest1.cpp
 *
 *  Created on: Jul 15, 2013
 *      Author: okhryptul
 */

#include <bd.h>
#include <stdio.h>
#include <string.h>

#include <iostream>

#include "../utils/default_block_io.h"

#include <Poco/SharedLibrary.h>


struct PluginLibrary
{
    Poco::SharedLibrary library;
    bd_plugin plugin;
};

void* loadPluginFunction(Poco::SharedLibrary &library, const char* file, const char* name)
{
    if (library.hasSymbol(name))
    {
        return library.getSymbol(name);
    }
    printf("[%s] Cannot load plugin function: %s\n", file, name);
    return 0;
}

bd_result loadPlugin(const char* file, PluginLibrary *pl)
{
    pl->library.load(file);

    pl->plugin.result_message    = (bd_result_message_t)    loadPluginFunction(pl->library, file, "bd_result_message");
    pl->plugin.initialize_plugin = (bd_initialize_plugin_t) loadPluginFunction(pl->library, file, "bd_initialize_plugin");
    pl->plugin.finalize_plugin   = (bd_finalize_plugin_t)   loadPluginFunction(pl->library, file, "bd_finalize_plugin");
    pl->plugin.template_name     = (bd_template_name_t)     loadPluginFunction(pl->library, file, "bd_template_name");
    pl->plugin.apply_template    = (bd_apply_template_t)    loadPluginFunction(pl->library, file, "bd_apply_template");
    pl->plugin.free_template     = (bd_free_template_t)     loadPluginFunction(pl->library, file, "bd_free_template");
    pl->plugin.get_string_value  = (bd_get_string_value_t)  loadPluginFunction(pl->library, file, "bd_get_string_value");

    if (pl->plugin.initialize_plugin == nullptr ||
        pl->plugin.finalize_plugin   == nullptr ||
        pl->plugin.template_name     == nullptr ||
        pl->plugin.apply_template    == nullptr ||
        pl->plugin.free_template     == nullptr ||
        pl->plugin.get_string_value  == nullptr)
    {
        pl->library.unload();
        return BD_ERROR;
    }

    return BD_SUCCESS;
}

bd_u64 calc_item_tree_size(const bd_block *item)
{
    bd_u64 result = sizeof(bd_block);
    if (item->children.count > 0)
    {
        for (bd_u32 i = 0; i < item->children.count; ++i)
        {
            result += calc_item_tree_size(item->children.child[i]);
        }
    }
    return result;
}

const char *single_indent = "  ";
void dump_item(const bd_block *item, const std::string& indent = "")
{
    printf("%-7s %-15s<%d>: %8lXh %6lXh",
            item->type_name, item->name, item->children.count,
            item->offset, (item->size * item->count));
    if (item->is_array)
    {
        printf(" [%lu %u]", item->size, item->count);
    }
    printf("\n");
    if (item->children.count > 0)
    {
        for (bd_u32 i = 0; i < item->children.count; ++i)
        {
            printf("%s[%u] ", indent.c_str(), i);
            dump_item(item->children.child[i], indent + single_indent + "  ");
        }
    }
}

int main(int argc, char* argv[])
{
    const char* file = "plugins/libLuaScripter.so";
    PluginLibrary pl;
    memset(&pl.plugin, 0, sizeof(pl.plugin));

    //std::cout << Poco::Path::current() << std::endl;

    if (!BD_SUCCEED(loadPlugin(file, &pl)))
    {
        std::cerr << "Could not load plugin " << file << std::endl;
        return -1;
    }

    bd_char name[256];
    bd_u32 templ_count;
    if (!BD_SUCCEED(pl.plugin.initialize_plugin(name, sizeof(name), &templ_count)))
    {
        std::cerr << "Could not initialize plugin " << file << std::endl;
        return -1;
    }

    std::cout << "Plugin " << name << " initialized. Available " << templ_count << " template(s)." << std::endl;

    std::string dataFile = argc > 1 ? argv[1] : "data1.npk";
    bd_default_block_io blob(dataFile);
    bd_block *root_item;

    bd_result res = pl.plugin.apply_template(0, &blob, &root_item, (bd_cstring) "hd.CHAR('kkk')");
    if (!BD_SUCCEED(res))
    {
        bd_char msg[1024];
        pl.plugin.result_message(res, msg, 1024);
        std::cerr << "Could not apply template 0: " << msg << std::endl;
        return -1;
    }

//    std::cout << "Template applied successfully. Tree size: " << calc_item_tree_size(root_item) << " bytes. Dump:" << std::endl;
//
//    dump_item(root_item, single_indent);

    if (!BD_SUCCEED(pl.plugin.free_template(0, root_item)))
    {
        std::cerr << "Could not free template 0" << std::endl;
        return -1;
    }

    if (!BD_SUCCEED(pl.plugin.finalize_plugin()))
    {
        std::cerr << "Could not finalize plugin: " << name << std::endl;
        return -1;
    }

    std::cout << "Plugin " << name << " successfully finalized." << std::endl;

    return 0;
}
