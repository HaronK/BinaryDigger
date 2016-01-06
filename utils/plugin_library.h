/*
 * plugin_library.h
 *
 *  Created on: Dec 10, 2015
 *      Author: oleg
 */

#ifndef UTILS_PLUGIN_LIBRARY_H_
#define UTILS_PLUGIN_LIBRARY_H_

#include <bd.h>
#include <boost/dll.hpp>

class plugin_library
{
public:
    plugin_library(const char* lib_path);
    ~plugin_library();

    bd_result get_plugin(bd_plugin& plugin);
    std::string get_path() { return sl.location().string(); }

private:
    bd_result result;
    bd_plugin plugin;
    boost::dll::shared_library sl;
};

#endif /* UTILS_PLUGIN_LIBRARY_H_ */
