/*
 * LuaScripter.cpp
 *
 *  Created on: Jul 25, 2013
 *      Author: Oleg Khryptul (okreptul@yahoo.com)
 */

#include <string>
#include <map>
#include <algorithm>
#include <iostream>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}
#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>

#include <default_plugin.h>
#include <demangle.h>

using namespace std;

#define DEBUG_OUTPUT(msg) cout << msg
//#define DEBUG_OUTPUT(msg)

class LuaTempl: public BlockTempl<LuaTempl>
{
public:
    LuaTempl(bd_block_io* _blob, bd_cstring _var_name, bd_cstring _templ_name, bd_u32 _count, BlockTemplBase* _parent,
            const bd_property_records &props)
        : BlockTempl(_blob, _var_name, _templ_name, _count, _parent, props)
    {}
};

#define bd_require_lua_type(tp, obj) bd_require_true_f(tp == luabind::type(obj), \
                      "Wrong lua type. Expected " # tp "(%d) but was %d", tp, luabind::type(obj))

string tostring(const luabind::object& obj, const string& indent = "")
{
    switch (luabind::type(obj))
    {
    case LUA_TNONE:
        return "<none>";
    case LUA_TNIL:
        return "<nil>";
    case LUA_TBOOLEAN:
        return luabind::object_cast<bool>(obj) ? "true" : "false";
    case LUA_TLIGHTUSERDATA:
        return "<lighuserdata>";
    case LUA_TNUMBER:
        return Poco::format("%f", luabind::object_cast<double>(obj));
    case LUA_TSTRING:
        return Poco::format("\"%s\"", luabind::object_cast<string>(obj));
    case LUA_TTABLE:
    {
        stringstream buf;
        buf << "{\n";
        for (luabind::iterator i(obj), end; i != end; ++i)
        {
            buf << indent << "  " << tostring(i.key(), indent + "  ") << ": " << tostring(*i, indent + "  ") << "\n";
        }
        buf << indent << "}\n";
        return buf.str();
    }
    case LUA_TFUNCTION:
        return "<function>";
    case LUA_TUSERDATA:
        return "<userdata>";
//    {
////        auto templ = (LuaTempl*) luabind::touserdata<BlockTemplBase>(obj);
//        auto templ = luabind::object_cast<BlockTemplBase>(obj);
//        return Poco::format("%s::%s", string(templ.type_name), string(templ.name));
//    }
    case LUA_TTHREAD:
        return "<thread>";
    default:
        return Poco::format("<unknown:%d>", luabind::type(obj));
    }
}

template<typename T>
T getGlobalVariable(lua_State* L, const char* name)
{
    return luabind::object_cast<T>(luabind::globals(L)[name]);
}

void setRootTempl(lua_State* L, LuaTempl* templ)
{
//    bd_require_eq(luabind::globals(L)["bd"], luabind::nil, "Root object already exists");

    luabind::globals(L)["bd"] = (BlockTemplBase*) templ;
}

LuaTempl* getCurrentTempl(lua_State* L)
{
    return (LuaTempl*) getGlobalVariable<BlockTemplBase*>(L, "self");
}

luabind::object setCurrentTempl(lua_State* L, LuaTempl* templ)
{
    auto prev = luabind::object(luabind::globals(L)["self"]);
    luabind::globals(L)["self"] = (BlockTemplBase*) templ;
    return prev;
}

void registerTemplVariable(lua_State* L, BlockTemplBase* templ, const string& var_name)
{
    auto cur_obj = luabind::object(luabind::globals(L)["self"]);

    // TODO: check variable duplication
//    bd_require_eq_f(cur_obj[var_name.c_str()], luabind::nil, "Object %s::%s already contains variable %s",
//            string(templ->type_name), string(templ->name), var_name);

    cur_obj[var_name.c_str()] = templ;
}

void getLuaCurrentFunctionName(lua_State* L, string& func_name)
{
    lua_Debug ar;
    if (!lua_getstack(L, 0, &ar)) // FIXME: no stack frame?
    {
        func_name = "?"; // FIXME: Some default ?
    }
    else
    {
        lua_getinfo(L, "n", &ar);
        func_name = (ar.name == NULL) ? "?" : ar.name; // FIXME: Some default ?
    }
}

auto lua_type_name = map<int, const char*>{
    {LUA_TNONE,          "NONE"},
    {LUA_TNIL,           "NIL"},
    {LUA_TBOOLEAN,       "BOOLEAN"},
    {LUA_TLIGHTUSERDATA, "LIGHTUSERDATA"},
    {LUA_TNUMBER,        "NUMBER"},
    {LUA_TSTRING,        "STRING"},
    {LUA_TTABLE,         "TABLE"},
    {LUA_TFUNCTION,      "FUNCTION"},
    {LUA_TUSERDATA,      "USERDATA"},
    {LUA_TTHREAD,        "THREAD"},
//    {LUA_NUMTAGS,        "NUMTAGS"},
};

bool check_lua_type(const luabind::object &param, int type, string &err)
{
    if (luabind::type(param) != type)
    {
        err = (boost::format("Property has wrong type. Expected: %1% but was %2%") %
                             type % lua_type_name[luabind::type(param)]).str();
        return false;
    }
    return true;
}

bool parse_property(const luabind::object &param, bd_property &result, string &err)
{
    switch (result.type)
    {
    case BD_PROP_INTEGER:
        if (!check_lua_type(param, LUA_TNUMBER, err))
            return false;
        result.value.i = luabind::object_cast<bd_i32>(param);
        return true;
    case BD_PROP_DOUBLE:
        if (!check_lua_type(param, LUA_TNUMBER, err))
            return false;
        result.value.d = luabind::object_cast<bd_f64>(param);
        break;
    case BD_PROP_STRING:
    {
        if (!check_lua_type(param, LUA_TSTRING, err))
            return false;
        auto val = luabind::object_cast<string>(param);
        result = bd_property((bd_string) val.c_str());
        break;
    }
    case BD_PROP_TO_STRING:
        if (!check_lua_type(param, LUA_TFUNCTION, err))
            return false;
        // TODO: implement
        break;
    default:
        err = (boost::format("Unsupported type: %1%") % result.type).str();
        return false;
    }
    return true;
}

/**
 * Parse properties in a tail of a table that should be in a form:
 *
 *   { ..., endian = BdTempl.BIG_ENDIAN, to_string = function(obj) return "test" end}
 *
 * @param iter        Points to the first property.
 * @param props [OUT] Properties
 */
void parse_properties(luabind::iterator &iter, bd_property_records &props)
{
    auto end = luabind::iterator();
    for (; iter != end; ++iter)
    {
        // property name should be a string
        if (luabind::type(iter.key()) != LUA_TSTRING)
        {
            // TODO: add warning about wrong property
            continue;
        }

        auto name = luabind::object_cast<string>(iter.key());

        // check if property is registered one
        if (default_property.find(name) == default_property.end())
        {
            // TODO: add warning about unsupported property
            continue; // TODO: just add user specified property for later usage
        }

        // get property
        auto prop = bd_property();
        prop.type = default_property[name].type;
        string err;
        if (!parse_property(*iter, prop, err))
        {
            // TODO: add warning with err message
            continue;
        }

        props[name] = prop;
    }
}

/**
 * Parse template block definition call. Examples:
 *
 * 1. Single block with optional properties:
 *
 *    Templ{"name" [, endian = BdTempl.BIG_ENDIAN, ...]}
 *
 * 2. Array block with optional properties:
 *
 *    Templ{"name", 3 [, endian = BdTempl.BIG_ENDIAN, ...]}
 *
 * 3. Template element that accept parameters (up to 10) with optional properties:
 *
 *    Templ{"name", {par1, par2, ... par10} [, endian = BdTempl.BIG_ENDIAN, ...]}
 *
 * 4. Array of template elements with optional properties. Each element accept parameters (up to 10):
 *
 *    Templ{"name", 3, {par1, par2, ... par10} [, endian = BdTempl.BIG_ENDIAN, ...]}
 *
 * @param data           Table to parse
 * @param var_name [OUT] Template name
 * @param arr_size [OUT] Array size if set
 * @param params   [OUT] Template parameters if set
 * @param props    [OUT] Template properties if set
 */
void parse_apply_templ_params(const luabind::object &data, string &var_name, int &arr_size, luabind::object &params,
        bd_property_records &props)
{
    bd_require_lua_type(LUA_TTABLE,  data);

    auto iter = luabind::iterator(data);
    auto end = luabind::iterator();

    // variable name
    bd_require_lua_type(LUA_TSTRING, *iter);
    var_name = luabind::object_cast<string>(*iter);
    ++iter;

    // array size
    if (iter != end && luabind::type(iter.key()) == LUA_TNUMBER && luabind::type(*iter) == LUA_TNUMBER)
    {
        arr_size = (int) luabind::object_cast<int>(*iter);
        ++iter;
    }

    // template parameters
    if (iter != end && luabind::type(iter.key()) == LUA_TNUMBER)
    {
        bd_require_lua_type(LUA_TTABLE, *iter);
        params = *iter;
        ++iter;
    }

    // object properties
    parse_properties(iter, props);

    DEBUG_OUTPUT("Data: " << tostring(data) << "\n");
}

/**
 * Call lua function with up to 10 parameters
 *
 * @param L         Lua state
 * @param func_name Name of the lua function
 * @param params    Parameters table
 */
void call_lua_function(lua_State* L, const string &func_name, const luabind::object &params)
{
    if (params.interpreter() == nullptr || luabind::type(params) != LUA_TTABLE)
    {
        luabind::call_function<void>(L, func_name.c_str(), params);
        return;
    }

    auto iter = luabind::iterator(params);
    auto end = luabind::iterator();

    if (iter == end)
    {
        luabind::call_function<void>(L, func_name.c_str());
        return;
    }

    auto param0 = *iter;
    ++iter;
    if (iter == end)
    {
        luabind::call_function<void>(L, func_name.c_str(), param0);
        return;
    }

    auto param1 = *iter;
    ++iter;
    if (iter == end)
    {
        luabind::call_function<void>(L, func_name.c_str(), param0, param1);
        return;
    }

    auto param2 = *iter;
    ++iter;
    if (iter == end)
    {
        luabind::call_function<void>(L, func_name.c_str(), param0, param1, param2);
        return;
    }

    auto param3 = *iter;
    ++iter;
    if (iter == end)
    {
        luabind::call_function<void>(L, func_name.c_str(), param0, param1, param2, param3);
        return;
    }

    auto param4 = *iter;
    ++iter;
    if (iter == end)
    {
        luabind::call_function<void>(L, func_name.c_str(), param0, param1, param2, param3, param4);
        return;
    }

    auto param5 = *iter;
    ++iter;
    if (iter == end)
    {
        luabind::call_function<void>(L, func_name.c_str(), param0, param1, param2, param3, param4, param5);
        return;
    }

    auto param6 = *iter;
    ++iter;
    if (iter == end)
    {
        luabind::call_function<void>(L, func_name.c_str(), param0, param1, param2, param3, param4, param5, param6);
        return;
    }

    auto param7 = *iter;
    ++iter;
    if (iter == end)
    {
        luabind::call_function<void>(L, func_name.c_str(), param0, param1, param2, param3, param4, param5, param6, param7);
        return;
    }

    auto param8 = *iter;
    ++iter;
    if (iter == end)
    {
        luabind::call_function<void>(L, func_name.c_str(), param0, param1, param2, param3, param4, param5, param6, param7, param8);
        return;
    }

    auto param9 = *iter;
    ++iter;
    if (iter == end)
    {
        luabind::call_function<void>(L, func_name.c_str(), param0, param1, param2, param3, param4, param5, param6, param7, param8, param9);
        return;
    }

    throw BlockTemplException(boost::format("Luabind supports only up to 10 function parameters.").str());
}

/**
 * Call 'apply' lua function associated with the template.
 *
 * @param L         Lua state
 * @param templ     Template object
 * @param func_name Name of the lua function
 * @param params    Parameters table
 */
void apply_templ_func(lua_State *L, LuaTempl *templ, const string &func_name, const luabind::object &params)
{
    DEBUG_OUTPUT("Apply templ func " << templ->type_name << "::" << templ->name << "." << func_name << " ...\n");

    auto prev = setCurrentTempl(L, templ);

    call_lua_function(L, func_name, params);

    luabind::globals(L)["self"] = prev;

    // set correct template size
    templ->size = templ->getPosition() - templ->offset;

    DEBUG_OUTPUT("  applied " << templ->type_name << "::" << templ->name << "." << func_name << "\n");
}

/**
 * Wrapper function that creates new template object.
 *
 * @param data Template creation data. See @see(parse_apply_templ_params) for details
 * @return Pointer to the created object
 */
BlockTemplBase *apply_templ(const luabind::object& data)
{
    auto var_name = string();
    auto arr_size = 0;
    auto params = luabind::object();
    auto props = bd_property_records();

    parse_apply_templ_params(data, var_name, arr_size, params, props);

    auto L = data.interpreter();

    // 1. get current function name = template name
    auto templ_name = string();
    getLuaCurrentFunctionName(L, templ_name);

    DEBUG_OUTPUT("Apply templ " << templ_name << "::" << var_name << " ...\n");

    auto cur_templ = getCurrentTempl(L);
    bd_require_not_null(cur_templ, "Current template is null");

    auto templ_props_name = templ_name + "_props";
    auto templ_props = luabind::object_cast<bd_property_records*>(luabind::globals(L)[templ_props_name]);

    auto templ = new LuaTempl(cur_templ->getBlockIo(), (bd_cstring) var_name.c_str(),
            (bd_cstring) templ_name.c_str(), arr_size, cur_templ, *templ_props);

    // set object properties
    templ->set_properties(props);

    registerTemplVariable(L, templ, var_name);

    auto templ_func = templ_name + "_func";

    apply_templ_func(L, templ, templ_func, params);

    DEBUG_OUTPUT("  applied " << templ_name << "::" << var_name << "\n");

    return templ;
}

/**
 * Backend for 'templ' lua function. Declares new template class. Example:
 *
 *    templ{"name", function([params]) ... end [, endian = BdTempl.BIG_ENDIAN, ...]}
 *
 * @param data Table to parse
 */
void register_templ(const luabind::object& data)
{
    bd_require_lua_type(LUA_TTABLE, data);

    auto iter = luabind::iterator(data);
    auto end = luabind::iterator();

    // template name
    bd_require_lua_type(LUA_TSTRING, *iter);
    auto templ_name = luabind::object_cast<string>(*iter);
    ++iter;

    // template 'apply' function
    bd_require_lua_type(LUA_TFUNCTION, *iter);
    auto func = *iter;
    ++iter;

    // template properties (optional)
    auto props = new bd_property_records;
    parse_properties(iter, *props);

    auto L = func.interpreter();

    DEBUG_OUTPUT("Create templ " << templ_name << " ...");

    // TODO: check that template with the same name doesn't already exist
    luabind::module(L)
    [
        luabind::def(templ_name.c_str(), &apply_templ)
    ];

    // register template 'apply' function
    auto templ_func = templ_name + "_func";
    luabind::globals(L)[templ_func] = func;

    // register template properties
    auto templ_props = templ_name + "_props";
    luabind::globals(L)[templ_props] = props;

    DEBUG_OUTPUT("  created\n");
}

/**
 * Convert simple template value to the lua object.
 *
 * @param L     Lua state
 * @param templ Template object
 * @return Lua object
 */
luabind::object get_templ_value(lua_State *L, BlockTemplBase *templ)
{
    switch (templ->getType())
    {
#define BD_BLOCK_TYPE_DECL(name, tp)                                        \
    case BD_##name: {                                                       \
        auto val = (tp) *templ;                                             \
        DEBUG_OUTPUT("Value " << templ->getName() << " = " << val << "\n"); \
        return luabind::object(L, val); }
    BD_BLOCK_TYPES
#undef BD_BLOCK_TYPE_DECL
    }
    bd_throw_f("Unsupported type: %d", templ->getType());
}

/**
 * Backend for 'val' lua function. Usage examples:
 *
 *   val{"field1.field2[2].field3 ..."}
 *   val{obj, "field.field2[2].field3 ..."}
 *
 * @param data Table of function parameters.
 * @return Simple template value wrapped in lua object.
 */
luabind::object get_value(const luabind::object& data)
{
    bd_require_lua_type(LUA_TTABLE, data);

    auto L = data.interpreter();
    auto paramsStr = string();

    // get template object
    BlockTemplBase *self;
    auto params_pos = 1;
    if (LUA_TSTRING == luabind::type(data[1]))
    {
        self = getCurrentTempl(L);
    }
    else
    {
        bd_require_lua_type(LUA_TUSERDATA, data[1]);

        // TODO: is this enough for correct casting?
        self = luabind::object_cast<BlockTemplBase*>(data[1]);

        params_pos = (LUA_TSTRING == luabind::type(data[2])) ? 2 : -1;
    }

    auto templ = self;

    if (params_pos > 0)
    {
        paramsStr = luabind::object_cast<string>(data[params_pos]);

        auto params = vector<string>();
        boost::split(params, paramsStr, boost::is_any_of("."));

        for (auto param : params)
        {
            boost::trim(param);
            if (param.empty())
                continue;

            // check []
            auto bracket_pos = param.find("[");
            auto indexStr = string();
            if (bracket_pos != string::npos)
            {
                indexStr = param.substr(bracket_pos + 1, param.size() - bracket_pos - 2);
                param = param.substr(0, bracket_pos - 1);
            }

            // if index not specified (-1) take the last one
            templ = templ->get(param.c_str(), indexStr.empty() ? (bd_u32)-1 : std::stoi(indexStr));
        }
    }

    if (templ->getType() == BD_CHAR && templ->isArray())
    {
        auto val = templ->getString();
        DEBUG_OUTPUT("Value " << templ->getName() << " = " << val << "\n");
        return luabind::object(L, val);
    }

    return get_templ_value(L, templ);
}

/**
 * Wrapper function that creates new simple template object.
 *
 * @param data Template creation data. See @see(parse_apply_templ_params) for details
 * @return Pointer to the created object
 */
template<class T>
BlockTemplBase *apply_simple_templ(const luabind::object& data)
{
    auto var_name = string();
    auto arr_size = 0;
    auto params = luabind::object();
    auto props = bd_property_records();

    parse_apply_templ_params(data, var_name, arr_size, params, props);

    DEBUG_OUTPUT("Apply simple templ " << get_type_name<T>() << "." << var_name << " ...");

    auto L = data.interpreter();

    auto cur_templ = getCurrentTempl(L);
    bd_require_not_null(cur_templ, "Current template is null");

    auto templ = new T(cur_templ->getBlockIo(), (bd_cstring) var_name.c_str(), arr_size, cur_templ, bd_property_records());

    // set object properties
    templ->set_properties(props);

    registerTemplVariable(L, templ, var_name);

    DEBUG_OUTPUT("  applied\n");

    return templ;
}

/**
 * Registers lua function for simple template object creation.
 *
 * @param name Template name.
 * @return Luabind scope
 */
template<class T>
luabind::scope register_simple_templ(const string& name)
{
    return luabind::def(name.c_str(), &apply_simple_templ<T>);
}

/**
 * Backend for 'check_bit' lua function. Checks bit of the value.
 *
 * @param value Value to check.
 * @param pos   Bit position in the value.
 * @return true if bit is set otherwise false
 */
bool check_bit(int value, int pos)
{
    return (value & (1 << pos)) != 0;
}

/**
 * TemplWrapper::applyTemplate method specialization for LuaTempl object.
 *
 * @param block_io       Block IO object.
 * @param script         Script text.
 * @param result   [OUT] Created blocks hierarchy.
 * @param err      [OUT] Error message.
 * @return BD_SUCCESS if template was successfully applied otherwise error code.
 */
template<>
bd_result TemplWrapper<LuaTempl>::applyTemplate(bd_block_io* block_io, bd_cstring script, bd_block **result, std::string &err) noexcept
{
    bd_check_not_null(block_io);

    auto result_code = bd_result{BD_SUCCESS};
    LuaTempl *templ = nullptr;

    // load and run lua script file
    try
    {
        // Create a new lua state
        auto L = luaL_newstate();

        // Connect LuaBind to this lua state
        luabind::open(L);

        luaopen_base(L);
        luaopen_package(L);

        // scripter default functions
        luabind::module(L)
        [
            luabind::class_<bd_block_io>("TemplBlob"),
            luabind::class_<bd_property_records>("Properties"),
            luabind::class_<BlockTemplBase>("BdTempl")
                .def("getPosition", &BlockTemplBase::getPosition)
                .def("setPosition", &BlockTemplBase::setPosition)
                .enum_("endian")
                [
                    luabind::value("LIT_ENDIAN", BD_ENDIAN_LIT),
                    luabind::value("BIG_ENDIAN", BD_ENDIAN_BIG)
                ],

            luabind::def("check_bit", &check_bit),

            luabind::def("templ", &register_templ),
            luabind::def("val",   &get_value),
            register_simple_templ<CHAR>  ("char"),
            register_simple_templ<UCHAR> ("uchar"),
            register_simple_templ<WORD>  ("word"),
            register_simple_templ<DWORD> ("dword"),
            register_simple_templ<QWORD> ("qword"),
            register_simple_templ<DOUBLE>("double")
        ];

        // TODO: specify properties
        templ = new LuaTempl(block_io, (bd_cstring) "bd", (bd_cstring) "LuaScript", 0, 0, bd_property_records());

        *result = templ;

        setRootTempl(L, templ);
        setCurrentTempl(L, templ);

//        DEBUG_OUTPUT("Globals: " << tostring(luabind::globals(L)) << "\n");

        /**/
        auto err_code = luaL_dofile(L, script);

        if (err_code)
        {
            auto err_msg = string(lua_tostring(L, -1));

            result_code = -1;
            err = "Cannot load or run Lua file: " + string(script) + "\nLua error: " + err_msg;
        }

        lua_close(L);
    }
    catch (const luabind::error &ex)
    {
        result_code = -1;
        err = string("Got Luabind exception while executing Lua script.\nException: ") + ex.what();
    }
    catch (const exception &ex)
    {
        result_code = -1;
        err = string("Got exception while executing Lua script.\nException: ") + ex.what();
    }
    catch (...)
    {
        result_code = -1;
        err = "Got unhandled exception while executing Lua script.";
    }

    if (templ != nullptr)
    {
        // set correct template size
        templ->size = templ->getPosition() - templ->offset;
    }

    return result_code;
}

/**
 * Adds LuaTempl wrapper to the list of registered templates.
 *
 * @param name        Template name (LuaScripter).
 * @param is_scripter Shows that plugin accepts script (true for LuaScripter).
 */
void register_plugin(string &name, bool &is_scripter)
{
    is_scripter = true;
    name = "LuaScripter";

    PluginContext::addTemplate(new TemplWrapper<LuaTempl>());
}
