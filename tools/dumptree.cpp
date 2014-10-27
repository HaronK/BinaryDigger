/*
 * dumptree.cpp
 *
 *  Created on: Feb 24, 2014
 *      Author: oleg
 */

#include <Poco/Util/Application.h>
#include <Poco/Util/Option.h>
#include <Poco/Util/OptionSet.h>
#include <Poco/Util/HelpFormatter.h>
#include <Poco/Util/AbstractConfiguration.h>
#include <Poco/AutoPtr.h>
#include <Poco/SharedLibrary.h>
#include <Poco/Exception.h>

#include <bd.h>
#include "../utils/default_block_io.h"

#include <iostream>
#include <boost/scope_exit.hpp>

using Poco::Util::Application;
using Poco::Util::Option;
using Poco::Util::OptionSet;
using Poco::Util::HelpFormatter;
using Poco::Util::AbstractConfiguration;
using Poco::Util::OptionCallback;
using Poco::SharedLibrary;
using Poco::AutoPtr;


// ---------------------------------------------------------------------------------------

#define DT_EXEC_CHECK(plugin, func, msg) {\
    bd_result res = plugin.func; \
    if (!BD_SUCCEED(res)) {\
        char buf[1024]; \
        plugin.result_message(res, (bd_string) buf, sizeof(buf)); \
        _errorMessage = Poco::format("Error:\n  %s\n  %s", std::string(msg), std::string(buf)); \
        return res; \
    } }

#define DT_EXEC_CHECK_F(plugin, func, msg, ...) {\
    bd_result res = plugin.func; \
    if (!BD_SUCCEED(res)) {\
        char buf[1024]; \
        plugin.result_message(res, (bd_string) buf, sizeof(buf)); \
        std::string err_msg = Poco::format(msg, __VA_ARGS__); \
        _errorMessage = Poco::format("Error:\n  %s\n  %s", err_msg, std::string(buf)); \
        return res; \
    } }

class DumpTreeApp: public Application
{
public:
    DumpTreeApp() : _helpRequested(false), _showPluginInfo(false), _pluginDir("./")
    {
    }

protected:
    void initialize(Application& self)
    {
        loadConfiguration(); // load default configuration files, if present
        Application::initialize(self);
        // add your own initialization code here
    }

    void uninitialize()
    {
        // add your own uninitialization code here
        Application::uninitialize();
    }

    void reinitialize(Application& self)
    {
        Application::reinitialize(self);
        // add your own reinitialization code here
    }

    void defineOptions(OptionSet& options)
    {
        Application::defineOptions(options);

        options.addOption(
            Option("help", "h", "display help information on command line arguments")
                .required(false)
                .repeatable(false)
                .callback(OptionCallback<DumpTreeApp>(this, &DumpTreeApp::handleHelp)));

        options.addOption(
            Option("plugin", "p", "specify plugin name")
                .required(true)
                .repeatable(false)
                .argument("plugin")
                .callback(OptionCallback<DumpTreeApp>(this, &DumpTreeApp::handlePluginName)));

        options.addOption(
            Option("plugin-dir", "P", "specify plugins folder")
                .required(false)
                .repeatable(false)
                .argument("dir")
                .binding("plugin.dir")
                .callback(OptionCallback<DumpTreeApp>(this, &DumpTreeApp::handlePluginDir)));

        options.addOption(
            Option("script", "s", "specify script file")
                .required(false)
                .repeatable(false)
                .argument("script")
                .binding("plugin.script")
                .callback(OptionCallback<DumpTreeApp>(this, &DumpTreeApp::handleScriptFile)));

        options.addOption(
            Option("plugin-info", "i", "show plugin info")
                .required(false)
                .repeatable(false)
                .callback(OptionCallback<DumpTreeApp>(this, &DumpTreeApp::handlePluginInfo)));

//        options.addOption(
//            Option("define", "D", "define a configuration property")
//                .required(false)
//                .repeatable(true)
//                .argument("name=value")
//                .callback(OptionCallback<DumpTreeApp>(this, &DumpTreeApp::handleDefine)));

        options.addOption(
            Option("config-file", "f", "load configuration data from a file")
                .required(false)
                .repeatable(true)
                .argument("file")
                .callback(OptionCallback<DumpTreeApp>(this, &DumpTreeApp::handleConfig)));

//        options.addOption(
//            Option("bind", "b", "bind option value to test.property")
//                .required(false)
//                .repeatable(false)
//                .argument("value")
//                .binding("test.property"));
    }

    void handleHelp(const std::string& name, const std::string& value)
    {
        _helpRequested = true;
        displayHelp();
        stopOptionsProcessing();
    }

    void handlePluginName(const std::string& name, const std::string& value)
    {
        _pluginName = value;
    }

    void handlePluginDir(const std::string& name, const std::string& value)
    {
        _pluginDir = value;
    }

    void handleScriptFile(const std::string& name, const std::string& value)
    {
        _scriptFile = value;
    }

    void handlePluginInfo(const std::string& name, const std::string& value)
    {
        _showPluginInfo = true;
    }

//    void handleDefine(const std::string& name, const std::string& value)
//    {
//        defineProperty(value);
//    }

    void handleConfig(const std::string& name, const std::string& value)
    {
        loadConfiguration(value);
    }

    void displayHelp()
    {
        HelpFormatter helpFormatter(options());
        helpFormatter.setCommand(commandName());
        helpFormatter.setUsage("OPTIONS file1 [file2 ...]");
        helpFormatter.setHeader("Tool to print out generated binary tree.");
        helpFormatter.format(std::cout);
    }

//    void defineProperty(const std::string& def)
//    {
//        std::string name;
//        std::string value;
//        std::string::size_type pos = def.find('=');
//        if (pos != std::string::npos)
//        {
//            name.assign(def, 0, pos);
//            value.assign(def, pos + 1, def.length() - pos);
//        }
//        else name = def;
//        config().setString(name, value);
//    }

    // ------------------------------------------------------------------------------
    struct PluginInfo
    {
        std::string pluginName;
        bd_plugin plugin;
        bool isScripter;
        std::vector<std::string> templates;
    };

    void loadPluginFunctions(SharedLibrary& pluginLibrary, bd_plugin& plugin)
    {
        plugin.result_message    = (bd_result_message_t)    pluginLibrary.getSymbol("bd_result_message");
        plugin.initialize_plugin = (bd_initialize_plugin_t) pluginLibrary.getSymbol("bd_initialize_plugin");
        plugin.finalize_plugin   = (bd_finalize_plugin_t)   pluginLibrary.getSymbol("bd_finalize_plugin");
        plugin.template_name     = (bd_template_name_t)     pluginLibrary.getSymbol("bd_template_name");
        plugin.apply_template    = (bd_apply_template_t)    pluginLibrary.getSymbol("bd_apply_template");
        plugin.free_template     = (bd_free_template_t)     pluginLibrary.getSymbol("bd_free_template");
        plugin.get_string_value  = (bd_get_string_value_t)  pluginLibrary.getSymbol("bd_get_string_value");
    }

    bd_result initializePlugin(PluginInfo& pluginInfo)
    {
        bd_char name[256];
        bd_u32 templ_count;

        DT_EXEC_CHECK(pluginInfo.plugin, initialize_plugin(name, sizeof(name), &templ_count), "Cannot initialize plugin");

        pluginInfo.pluginName = name;
        pluginInfo.isScripter = templ_count == 0;

        for (bd_u32 i = 0; i < templ_count; ++i)
        {
            DT_EXEC_CHECK_F(pluginInfo.plugin, template_name(i, name, sizeof(name)), "Cannot retrieve template name: %d", i);

            pluginInfo.templates.push_back(name);
        }

        return BD_SUCCESS;
    }

    void showPluginInfo(PluginInfo& pluginInfo)
    {
        poco_information_f1(logger(), "Name:        %s", pluginInfo.pluginName);
        poco_information_f1(logger(), "Is scripter: %b", pluginInfo.isScripter);
        poco_information_f1(logger(), "Temlates[%u]:",   pluginInfo.templates.size());

        for (std::vector<std::string>::const_iterator it = pluginInfo.templates.begin(); it != pluginInfo.templates.end(); ++it)
        {
            poco_information_f1(logger(), "    %s", *it);
        }
    }

    bd_result finalizePlugin(PluginInfo& pluginInfo)
    {
        DT_EXEC_CHECK(pluginInfo.plugin, finalize_plugin(), "Could not finalize plugin");

        return BD_SUCCESS;
    }

    bd_result applyTemplate(PluginInfo& pluginInfo, bd_default_block_io& templBlob, bd_block **block)
    {
        freeTemplate(pluginInfo, *block);

//        templBlob.dataFile.seekg(0);

        // TODO: set correct template index
        bd_u32 templIndex = 0;

        DT_EXEC_CHECK_F(pluginInfo.plugin,
                apply_template(templIndex, &templBlob, block, _scriptFile.empty() ? 0 : (bd_cstring) _scriptFile.c_str()),
                "Could not apply template %u", templIndex);

        return BD_SUCCESS;
    }

    bd_result freeTemplate(PluginInfo& pluginInfo, bd_block *block)
    {
        if (block == 0)
            return BD_SUCCESS;

        // TODO: set correct template index
        bd_u32 templIndex = 0;

        DT_EXEC_CHECK_F(pluginInfo.plugin, free_template(templIndex, block), "Could not free template %u", templIndex);

        return BD_SUCCESS;
    }

    bd_result dumpTree(std::ostream& output, bd_plugin& plugin, bd_default_block_io& templBlob, const bd_block *block, const std::string& indent = "")
    {
        if (block == 0)
        {
            output << indent << "NULL\n";
            return BD_SUCCESS;
        }

        output << indent << block->type_name << "(" << block->type << ") " << block->name;
        if (block->is_array == BD_TRUE || block->type == BD_STRING)
        {
            output << "[" << block->count << "] (" << block->elem_size << "/" << block->size << ")";

            if (is_string(block))
            {
                bd_u32 val_size = block->size + 1;
                bd_string val = new bd_char[val_size];
                BOOST_SCOPE_EXIT(&val)
                {
                    delete[] val;
                }
                BOOST_SCOPE_EXIT_END

                DT_EXEC_CHECK(plugin, get_string_value((bd_block *) block, val, val_size), "Could not get string value");

                output << ": \"" << val << "\"";
            }
        }
        else
        {
            output << "(" << block->size << "): ";

            bd_char val[100];
            DT_EXEC_CHECK(plugin, get_string_value((bd_block *) block, val, sizeof(val)), "Could not get value");

            output << val;
        }

        if (block->children.count > 0)
        {
            output << " [" << block->children.count << "]:\n";

            for (bd_u32 i = 0; i < block->children.count; ++i)
            {
                dumpTree(output, plugin, templBlob, block->children.child[i], indent + "  ");
            }
        }
        else
        {
            output << "\n";
        }

        return BD_SUCCESS;
    }

    int main(const std::vector<std::string>& args)
    {
        if (_helpRequested)
            return Application::EXIT_OK;

        if (!_showPluginInfo && args.size() == 0)
        {
            poco_warning(logger(), "At least one file to process should be specified.");
            return Application::EXIT_NOINPUT;
        }

        if (!_scriptFile.empty() && args.size() != 1)
        {
            poco_warning(logger(), "Only one plugin should be provided to load script.");
            return Application::EXIT_USAGE;
        }

        // 1. Load plugin
        std::string pluginPath = _pluginDir + "/" + _pluginName + SharedLibrary::suffix();
        SharedLibrary pluginLibrary(pluginPath);
        PluginInfo pluginInfo;

        loadPluginFunctions(pluginLibrary, pluginInfo.plugin);

        if (!BD_SUCCEED(initializePlugin(pluginInfo)))
        {
            poco_error(logger(), _errorMessage);
            return Application::EXIT_DATAERR;
        }

        if (_showPluginInfo)
        {
            showPluginInfo(pluginInfo);
        }

        auto result = Application::EXIT_OK;
        if (args.size() > 0)
        {
            // 2. Dump each file
            for (std::vector<std::string>::const_iterator it = args.begin(); it != args.end(); ++it)
            {
                bd_default_block_io templBlob(*it);
                bd_block *item = NULL;

                // 1. Apply plugin to the file
                if (!BD_SUCCEED(applyTemplate(pluginInfo, templBlob, &item)))
                {
                    poco_error(logger(), _errorMessage);
                    result = Application::EXIT_DATAERR;
                }

                // 2. Dump tree hierarchy
                if (!BD_SUCCEED(dumpTree(std::cout, pluginInfo.plugin, templBlob, item)))
                {
                    poco_error(logger(), _errorMessage);
                    return Application::EXIT_DATAERR;
                }

                // 3. Cleanup tree
                if (!BD_SUCCEED(freeTemplate(pluginInfo, item)))
                {
                    poco_error(logger(), _errorMessage);
                    return Application::EXIT_DATAERR;
                }
            }
        }

        if (!BD_SUCCEED(finalizePlugin(pluginInfo)))
        {
            poco_error(logger(), _errorMessage);
            return Application::EXIT_DATAERR;
        }

        return result;
    }

    void printProperties(const std::string& base)
    {
        AbstractConfiguration::Keys keys;
        config().keys(base, keys);
        if (keys.empty())
        {
            if (config().hasProperty(base))
            {
                std::string msg;
                msg.append(base);
                msg.append(" = ");
                msg.append(config().getString(base));
                logger().information(msg);
            }
        }
        else
        {
            for (AbstractConfiguration::Keys::const_iterator it = keys.begin(); it != keys.end(); ++it)
            {
                std::string fullKey = base;
                if (!fullKey.empty()) fullKey += '.';
                fullKey.append(*it);
                printProperties(fullKey);
            }
        }
    }

private:
    bool _helpRequested;
    bool _showPluginInfo;

    std::string _errorMessage;

    std::string _pluginName;
    std::string _pluginDir;
    std::string _scriptFile;
};


POCO_APP_MAIN(DumpTreeApp)
