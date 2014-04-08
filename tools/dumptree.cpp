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
#include <Poco/FileStream.h>

#include <iostream>

#include <bd.h>


using Poco::Util::Application;
using Poco::Util::Option;
using Poco::Util::OptionSet;
using Poco::Util::HelpFormatter;
using Poco::Util::AbstractConfiguration;
using Poco::Util::OptionCallback;
using Poco::SharedLibrary;
using Poco::AutoPtr;


// ---------------------------------------------------------------------------------------
struct FileTemplBlob : bd_templ_blob
{
    FileTemplBlob(const std::string& path);
    ~FileTemplBlob();

    Poco::FileStream dataFile;
};

bd_result _get_pos(bd_templ_blob *self, bd_u64 *_pos)
{
    FileTemplBlob *blob = (FileTemplBlob *)self;
    try
    {
        *_pos = blob->dataFile.tellg();
    }
    catch (const std::ios_base::failure&)
    {
        return -1;
    }
    return BD_SUCCESS;
}

bd_result _set_pos(bd_templ_blob *self, bd_u64 pos)
{
    FileTemplBlob *blob = (FileTemplBlob *)self;
    try
    {
        blob->dataFile.seekg(pos);
    }
    catch (const std::ios_base::failure&)
    {
        return -1;
    }
    return BD_SUCCESS;
}

bd_result _shift_pos(bd_templ_blob *self, bd_u64 offset)
{
    FileTemplBlob *blob = (FileTemplBlob *)self;
    try
    {
        blob->dataFile.seekg(offset, std::ios_base::cur);
    }
    catch (const std::ios_base::failure&)
    {
        return -1;
    }
    return BD_SUCCESS;
}

bd_result _get_data(bd_templ_blob *self, bd_u64 size, bd_pointer val)
{
    FileTemplBlob *blob = (FileTemplBlob *)self;
    try
    {
        blob->dataFile.read((char*)val, size);
    }
    catch (const std::ios_base::failure&)
    {
        return -1;
    }
    return BD_SUCCESS;
}

bd_result _get_datap(bd_templ_blob *self, bd_u64 pos, bd_u64 size, bd_pointer val)
{
    FileTemplBlob *blob = (FileTemplBlob *)self;
    try
    {
        std::streampos cur_pos = blob->dataFile.tellg();
        blob->dataFile.seekg(pos);
        blob->dataFile.read((char*)val, size);
        blob->dataFile.seekg(cur_pos);
    }
    catch (const std::ios_base::failure&)
    {
        return -1;
    }
    return BD_SUCCESS;
}

FileTemplBlob::FileTemplBlob(const std::string& path) : dataFile(path, std::ios::in)
{
    // TODO: set correct internal state flags for dataFile if needed

    get_pos   = _get_pos;
    set_pos   = _set_pos;
    shift_pos = _shift_pos;
    get_data  = _get_data;
    get_datap = _get_datap;
}

FileTemplBlob::~FileTemplBlob()
{
}
// ---------------------------------------------------------------------------------------

#define DT_EXEC_CHECK(plugin, func, msg) {\
    bd_result res = plugin.func; \
    if (!BD_SUCCEED(res)) {\
        char buf[1024]; \
        plugin.result_message(res, (bd_string) buf, 1024); \
        throw Poco::Exception(Poco::format("Error:\n  %s\n  %s", std::string(msg), std::string(buf))); \
    } }

#define DT_EXEC_CHECK_F(plugin, func, msg, ...) {\
    bd_result res = plugin.func; \
    if (!BD_SUCCEED(res)) {\
        char buf[1024]; \
        plugin.result_message(res, (bd_string) buf, 1024); \
        std::string err_msg = Poco::format(msg, __VA_ARGS__); \
        throw Poco::Exception(Poco::format("Error:\n  %s\n  %s", err_msg, std::string(buf))); \
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
        plugin.template_name     = (bd_template_name_t)     pluginLibrary.getSymbol("bd_template_name");
        plugin.apply_template    = (bd_apply_template_t)    pluginLibrary.getSymbol("bd_apply_template");
        plugin.free_template     = (bd_free_template_t)     pluginLibrary.getSymbol("bd_free_template");
        plugin.finalize_plugin   = (bd_finalize_plugin_t)   pluginLibrary.getSymbol("bd_finalize_plugin");
    }

    void initializePlugin(PluginInfo& pluginInfo)
    {
        bd_string name = 0;
        bd_u32 templ_count;

        DT_EXEC_CHECK(pluginInfo.plugin, initialize_plugin(&name, &templ_count), "Cannot initialize plugin");

        pluginInfo.pluginName = name;
        pluginInfo.isScripter = templ_count == 0;

        for (bd_u32 i = 0; i < templ_count; ++i)
        {
            bd_string name;
            DT_EXEC_CHECK_F(pluginInfo.plugin, template_name(i, (bd_string*)&name), "Cannot retrieve template name: %d", i);

            pluginInfo.templates.push_back(name);
        }
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

    void finalizePlugin(PluginInfo& pluginInfo)
    {
        DT_EXEC_CHECK(pluginInfo.plugin, finalize_plugin(), "Could not finalize plugin");
    }

    void applyTemplate(PluginInfo& pluginInfo, FileTemplBlob& templBlob, bd_item **item)
    {
        freeTemplate(pluginInfo, *item);

//        templBlob.dataFile.seekg(0);

        // TODO: set correct template index
        bd_u32 templIndex = 0;

        DT_EXEC_CHECK_F(pluginInfo.plugin,
                apply_template(templIndex, &templBlob, item, _scriptFile.empty() ? 0 : (bd_cstring) _scriptFile.c_str()),
                "Could not apply template %u", templIndex);
    }

    void freeTemplate(PluginInfo& pluginInfo, bd_item *item)
    {
        if (item == 0)
            return;

        // TODO: set correct template index
        bd_u32 templIndex = 0;

        DT_EXEC_CHECK_F(pluginInfo.plugin, free_template(templIndex, item), "Could not free template %u", templIndex);
    }

    void dumpTree(std::ostream& output, FileTemplBlob& templBlob, const bd_item *item, const std::string& indent = "")
    {
        if (item == 0)
        {
            output << indent << "NULL\n";
            return;
        }

        output << indent << item->type_name << "(" << item->type << ") " << item->name;
        if (item->is_array == BD_TRUE)
        {
            output << "[" << item->count << "] (" << item->elem_size << "/" << item->size << ")";
        }
        else
        {
            output << "(" << item->size << ")";
        }

        if (item->children.count > 0)
        {
            output << " [" << item->children.count << "]:\n";

            for (bd_u32 i = 0; i < item->children.count; ++i)
            {
                dumpTree(output, templBlob, item->children.child[i], indent + "  ");
            }
        }
        else
        {
            output << "\n";
        }
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

        initializePlugin(pluginInfo);

        if (_showPluginInfo)
        {
            showPluginInfo(pluginInfo);
        }

        if (args.size() > 0)
        {
            // 2. Dump each file
            for (std::vector<std::string>::const_iterator it = args.begin(); it != args.end(); ++it)
            {
                FileTemplBlob templBlob(*it);
                bd_item *item = NULL;

                // 1. Apply plugin to the file
                try
                {
                    applyTemplate(pluginInfo, templBlob, &item);
                }
                catch (const std::exception& ex)
                {
                    poco_error(logger(), ex.what());
                }

                // 2. Dump tree hierarchy
                dumpTree(std::cout, templBlob, item);

                // 3. Cleanup tree
                try
                {
                    freeTemplate(pluginInfo, item);
                }
                catch (const std::exception& ex)
                {
                    poco_error(logger(), ex.what());
                }
            }
        }

        finalizePlugin(pluginInfo);

//        logger().information("Arguments to main():");
//        for (std::vector<std::string>::const_iterator it = args.begin(); it != args.end(); ++it)
//        {
//            logger().information(*it);
//        }
//        logger().information("Application properties:");
//        printProperties("");
        return Application::EXIT_OK;
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

    std::string _pluginName;
    std::string _pluginDir;
    std::string _scriptFile;
};


POCO_APP_MAIN(DumpTreeApp)
