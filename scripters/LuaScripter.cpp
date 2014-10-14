/*
 * LuaScripter.cpp
 *
 *  Created on: Jul 25, 2013
 *      Author: Oleg Khryptul (okreptul@yahoo.com)
 */

#include <string>
#include <algorithm>
#include <ostream>

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
    LuaTempl(bd_block_io* _blob, bd_cstring _var_name, bd_cstring _templ_name, bd_u32 _count, BlockTemplBase* _parent)
        : BlockTempl(_blob, _var_name, _templ_name, _count, _parent)
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

void parse_apply_templ_params(const luabind::object& data, string& var_name, int& arr_size, luabind::object& params)
{
    bd_require_lua_type(LUA_TTABLE,  data);
    bd_require_lua_type(LUA_TSTRING, data[1]);

    DEBUG_OUTPUT("Data: " << tostring(data) << "\n");

    var_name = luabind::object_cast<string>(data[1]);

    auto i = 2;
    if (luabind::type(data[i]) == LUA_TNUMBER)
    {
        arr_size = (int) luabind::object_cast<int>(data[i]);
        i++;
    }

    if (luabind::type(data[i]) == LUA_TTABLE)
    {
        params = data[i];
    }
}

void apply_templ_func(lua_State* L, LuaTempl* templ, const char* func_name, const luabind::object& params)
{
    DEBUG_OUTPUT("Apply templ func " << templ->type_name << "::" << templ->name << "." << func_name << " ...\n");

    auto prev = setCurrentTempl(L, templ);

    luabind::call_function<void>(L, func_name, params);

    luabind::globals(L)["self"] = prev;

    // set correct template size
    templ->size = templ->getPosition() - templ->offset;

    DEBUG_OUTPUT("  applied " << templ->type_name << "::" << templ->name << "." << func_name << "\n");
}

BlockTemplBase *apply_templ(const luabind::object& data)
{
    auto var_name = string();
    auto arr_size = 0;
    auto params = luabind::object();

    parse_apply_templ_params(data, var_name, arr_size, params);

    auto L = data.interpreter();

    // 1. get current function name = template name
    auto templ_name = string();
    getLuaCurrentFunctionName(L, templ_name);

    DEBUG_OUTPUT("Apply templ " << templ_name << "::" << var_name << " ...\n");

    auto cur_templ = getCurrentTempl(L);
    bd_require_not_null(cur_templ, "Current template is null");

    // TODO: apply global template settings
    auto settings_name = templ_name + "_settings";
//    luabind::object global_templ_settings = luabind::globals(L)[settings_name];

    // TODO: apply local template settings that are defined in 'data' table as named elements

    auto templ = new LuaTempl(cur_templ->getBlockIo(), (bd_cstring) var_name.c_str(),
            (bd_cstring) templ_name.c_str(), arr_size, cur_templ);

    registerTemplVariable(L, templ, var_name);

    auto templ_func = templ_name + "_func";

    apply_templ_func(L, templ, templ_func.c_str(), params);

    DEBUG_OUTPUT("  applied " << templ_name << "::" << var_name << "\n");

    return templ;
}

/**
 * Expecting 3 parameters:
 * 1. Template name.
 * 2. Template apply function.
 * 3. Other template parameters table.
 *
 * @param obj Lua table object
 */
void register_templ(const luabind::object& data)
{
    bd_require_lua_type(LUA_TTABLE,    data);
    bd_require_lua_type(LUA_TSTRING,   data[1]);
    bd_require_lua_type(LUA_TFUNCTION, data[2]);

    auto templ_name = luabind::object_cast<string>(data[1]);
    auto func = data[2];

    auto L = func.interpreter();

    DEBUG_OUTPUT("Create templ " << templ_name << " ...");

    // TODO: check that template with the same name doesn't already exist
    luabind::module(L)
    [
        luabind::def(templ_name.c_str(), &apply_templ)
    ];

    // register template function
    auto templ_func = templ_name + "_func";
    luabind::globals(L)[templ_func] = func;

    // register template parameters
    auto templ_params = templ_name + "_settings";
    luabind::globals(L)[templ_params] = data;

    DEBUG_OUTPUT("  created\n");
}

vector<string> split(const string &s, char delim)
{
    auto result = vector<string>();
    stringstream ss(s);
    auto item = string();
    while (getline(ss, item, delim))
    {
        result.push_back(item);
    }
    return result;
}

luabind::object get_templ_value(lua_State *L, BlockTemplBase *templ)
{
    switch (templ->getType())
    {
#define BD_BLOCK_TYPE_DECL(name, tp)                                        \
    case BD_##name: {                                                       \
        auto val = templ->value<tp>();                                      \
        DEBUG_OUTPUT("Value " << templ->getName() << " = " << val << "\n"); \
        return luabind::object(L, val); }
    BD_BLOCK_TYPES
#undef BD_BLOCK_TYPE_DECL
    }
    bd_throw_f("Unsupported type: %d", templ->getType());
}

luabind::object get_value(const luabind::object& data)
{
    bd_require_lua_type(LUA_TTABLE, data);

    BlockTemplBase *self;
    auto L = data.interpreter();
    auto paramsStr = string();

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

        auto params = split(paramsStr, '.');
        for (auto param : params)
        {
            // TODO: trim param
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

template<class T>
void apply_simple_templ(const luabind::object& data)
{
    auto var_name = string();
    auto arr_size = 0;
    auto params = luabind::object();

    parse_apply_templ_params(data, var_name, arr_size, params);

    auto L = data.interpreter();

    auto cur_templ = getCurrentTempl(L);
    bd_require_not_null(cur_templ, "Current template is null");

    DEBUG_OUTPUT("Apply simple templ " << get_type_name<T>() << "." << var_name << " ...");

    // TODO: apply local template settings that are defined in 'data' table as named elements

    auto templ = new T(cur_templ->getBlockIo(), (bd_cstring) var_name.c_str(), arr_size, cur_templ);

    registerTemplVariable(L, templ, var_name);

    DEBUG_OUTPUT("  applied\n");
}

template<class T>
luabind::scope register_simple_templ(const string& name)
{
    return luabind::def(name.c_str(), &apply_simple_templ<T>);
}

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
            luabind::class_<BlockTemplBase>("BdTempl")
                .def("getPosition", &BlockTemplBase::getPosition)
                .def("setPosition", &BlockTemplBase::setPosition),

            luabind::def("templ", &register_templ),
            luabind::def("val",   &get_value),
            register_simple_templ<CHAR>  ("char"),
            register_simple_templ<UCHAR> ("uchar"),
            register_simple_templ<WORD>  ("word"),
            register_simple_templ<DWORD> ("dword"),
            register_simple_templ<QWORD> ("qword"),
            register_simple_templ<DOUBLE>("double")
        ];

        templ = new LuaTempl(block_io, (bd_cstring) "bd", (bd_cstring) "LuaScript", 0, 0);

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

void register_plugin(string &name, bool &is_scripter)
{
    is_scripter = true;
    name = "LuaScripter";

    PluginContext::addTemplate(new TemplWrapper<LuaTempl>());
}
