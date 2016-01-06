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

#include "test_common.h"
#include "../utils/default_block_io.h"
#include "../utils/plugin_library.h"

int main(int argc, char* argv[])
{
    const char* file = "plugins/libLuaScripter.so";
    plugin_library pl(file);

    //std::cout << Poco::Path::current() << std::endl;

    bd_plugin plugin;
    if (!BD_SUCCEED(pl.get_plugin(plugin)))
    {
        std::cerr << "Could not load plugin " << file << std::endl;
        return -1;
    }

    bd_char name[256];
    bd_u32 templ_count;
    if (!BD_SUCCEED(plugin.initialize_plugin(name, sizeof(name), &templ_count)))
    {
        std::cerr << "Could not initialize plugin " << file << std::endl;
        return -1;
    }

    std::cout << "Plugin " << name << " initialized. Available " << templ_count << " template(s)." << std::endl;

    std::string dataFile = argc > 1 ? argv[1] : "data1.npk";
    bd_default_block_io blob(dataFile);
    bd_block *root_item;

    bd_result res = plugin.apply_template(0, &blob, &root_item, (bd_cstring) "hd.CHAR('kkk')");
    if (!BD_SUCCEED(res))
    {
        bd_char msg[1024];
        plugin.result_message(res, msg, 1024);
        std::cerr << "Could not apply template 0: " << msg << std::endl;
        return -1;
    }

//    std::cout << "Template applied successfully. Tree size: " << calc_item_tree_size(root_item) << " bytes. Dump:" << std::endl;
//
//    dump_item(root_item, single_indent);

    if (!BD_SUCCEED(plugin.free_template(0, root_item)))
    {
        std::cerr << "Could not free template 0" << std::endl;
        return -1;
    }

    if (!BD_SUCCEED(plugin.finalize_plugin()))
    {
        std::cerr << "Could not finalize plugin: " << name << std::endl;
        return -1;
    }

    std::cout << "Plugin " << name << " successfully finalized." << std::endl;

    return 0;
}
