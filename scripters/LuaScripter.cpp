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


class LuaTempl: public DefaultTempl<LuaTempl>
{
public:
    LuaTempl(bd_templ_blob* _blob, bd_cstring _var_name, bd_cstring _templ_name, bd_u32 _count, DefaultTemplBase* _parent)
        : DefaultTempl(_blob, _var_name, _templ_name, _count, _parent)
    {}
};

#define bd_require_lua_type(tp, obj) bd_require_true_f(tp == luabind::type(obj), \
                      "Wrong lua type. Expected " # tp "(%d) but was %d", tp, luabind::type(obj))

std::string tostring(const luabind::object& obj, const std::string& indent = "")
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
        return Poco::format("\"%s\"", luabind::object_cast<std::string>(obj));
    case LUA_TTABLE:
    {
        std::stringstream buf;
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
////        LuaTempl* templ = (LuaTempl*) luabind::touserdata<DefaultTemplBase>(obj);
//        DefaultTemplBase templ = luabind::object_cast<DefaultTemplBase>(obj);
//        return Poco::format("%s::%s", std::string(templ.type_name), std::string(templ.name));
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

LuaTempl* getGlobalLuaTemplate(lua_State* L, const char* name)
{
    return (LuaTempl*) getGlobalVariable<DefaultTemplBase*>(L, name);
}

void setRootTempl(lua_State* L, LuaTempl* templ)
{
//    bd_require_eq(luabind::globals(L)["root"], luabind::nil, "Root object already exists");

    luabind::globals(L)["root"] = (DefaultTemplBase*) templ;
}

LuaTempl* getCurrentTempl(lua_State* L)
{
    return getGlobalLuaTemplate(L, "cur_templ");
}

luabind::object setCurrentTempl(lua_State* L, LuaTempl* templ)
{
    luabind::object prev = luabind::globals(L)["cur_templ"];
    luabind::globals(L)["cur_templ"] = (DefaultTemplBase*) templ;
    return prev;
}

void registerTemplVariable(lua_State* L, DefaultTemplBase* templ, const std::string& var_name)
{
    luabind::object cur_obj = luabind::globals(L)["cur_templ"];

    // TODO: check variable duplication
//    bd_require_eq_f(cur_obj[var_name.c_str()], luabind::nil, "Object %s::%s already contains variable %s",
//            std::string(templ->type_name), std::string(templ->name), var_name);

    cur_obj[var_name.c_str()] = templ;
}

void getLuaCurrentFunctionName(lua_State* L, std::string& func_name)
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

void parse_apply_templ_params(const luabind::object& data, std::string& var_name, int& arr_size, luabind::object& params)
{
    bd_require_lua_type(LUA_TTABLE,  data);
    bd_require_lua_type(LUA_TSTRING, data[1]);

//    std::cout << "Data: " << tostring(data) << "\n";

    var_name = luabind::object_cast<std::string>(data[1]);

    int i = 2;
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
//    std::cout << "Apply templ func " << templ->type_name << "::" << templ->name << "." << func_name << " ...\n";

    luabind::object prev = setCurrentTempl(L, templ);

    luabind::call_function<void>(L, func_name, params);

    luabind::globals(L)["cur_templ"] = prev;

//    std::cout << "  applied\n";
}

void apply_templ(const luabind::object& data)
{
    std::string var_name;
    int arr_size = 0;
    luabind::object params;

    parse_apply_templ_params(data, var_name, arr_size, params);

    lua_State* L = data.interpreter();

    // 1. get current function name = template name
    std::string templ_name;
    getLuaCurrentFunctionName(L, templ_name);

//    std::cout << "Apply templ " << templ_name << "::" << var_name << " ...\n";

    LuaTempl* cur_templ = getCurrentTempl(L);
    bd_require_not_null(cur_templ, "Current template is null");

    // TODO: apply global template settings
    std::string settings_name = templ_name + "_settings";
//    luabind::object global_templ_settings = luabind::globals(L)[settings_name];

    // TODO: apply local template settings that are defined in 'data' table as named elements

    LuaTempl* templ = new LuaTempl(cur_templ->getBlob(), (bd_cstring) var_name.c_str(),
            (bd_cstring) templ_name.c_str(), arr_size, cur_templ);

    registerTemplVariable(L, templ, var_name);

    std::string templ_func = templ_name + "_func";

    apply_templ_func(L, templ, templ_func.c_str(), params);

//    std::cout << "  applied\n";
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

    std::string templ_name = luabind::object_cast<std::string>(data[1]);
    luabind::object func = data[2];

    lua_State* L = func.interpreter();

//    std::cout << "Create templ " << templ_name << " ...";

    // TODO: check that template with the same name doesn't already exist
    luabind::module(L)
    [
        luabind::def(templ_name.c_str(), &apply_templ)
    ];

    // register template function
    std::string templ_func = templ_name + "_func";
    luabind::globals(L)[templ_func] = func;

    // register template parameters
    std::string templ_params = templ_name + "_settings";
    luabind::globals(L)[templ_params] = data;

//    std::cout << "  created\n";
}

template<class T>
void apply_simple_templ(const luabind::object& data)
{
    std::string var_name;
    int arr_size = 0;
    luabind::object params;

    parse_apply_templ_params(data, var_name, arr_size, params);

    lua_State* L = data.interpreter();

    LuaTempl* cur_templ = getCurrentTempl(L);
    bd_require_not_null(cur_templ, "Current template is null");

//    std::cout << "Apply simple templ " << typeid(T).name() << "." << var_name << " ...";

    // TODO: apply local template settings that are defined in 'data' table as named elements

    T* templ = new T(cur_templ->getBlob(), (bd_cstring) var_name.c_str(), arr_size, cur_templ);

    registerTemplVariable(L, templ, var_name);

//    std::cout << "  applied\n";
}

template<class T>
luabind::scope register_simple_templ(const std::string& name)
{
    return luabind::def(name.c_str(), &apply_simple_templ<T>);
}

template<>
bd_item* TemplWrapper<LuaTempl>::applyTemplate(bd_templ_blob* blob, bd_cstring script)
{
    bd_require_not_null(blob, "Parameter 'blob' is null");

    // Create a new lua state
    lua_State* L = luaL_newstate();

    // Connect LuaBind to this lua state
    luabind::open(L);

    // scripter default functions
    luabind::module(L)
    [
        luabind::class_<bd_templ_blob>("TemplBlob"),
        luabind::class_<DefaultTemplBase>("LuaTempl")
            .def("getPosition", &DefaultTemplBase::getPosition)
            .def("setPosition", &DefaultTemplBase::setPosition),

        luabind::def("templ", &register_templ),
        register_simple_templ<CHAR>  ("char"),
        register_simple_templ<UCHAR> ("uchar"),
        register_simple_templ<WORD>  ("word"),
        register_simple_templ<DWORD> ("dword"),
        register_simple_templ<QWORD> ("qword"),
        register_simple_templ<DOUBLE>("double")
    ];

    LuaTempl* templ = new LuaTempl(blob, (bd_cstring) "root", (bd_cstring) "LuaScript", 0, 0);

    setRootTempl(L, templ);
    setCurrentTempl(L, templ);

//    std::cout << "Globals: " << tostring(luabind::globals(L)) << "\n";

    // load and run lua script file
    int rc = luaL_dofile(L, script);
    bd_require_ne_f(rc, 1, "Cannot load or run Lua file: %s\nLua error: %s",
            std::string(script), std::string(lua_tostring(L, -1)));

    lua_close(L);

    return templ;
}

void register_plugin(bd_string *name, RegisteredTemplates& templ, bool& is_scripter)
{
    *name = (bd_string) "LuaScripter";
    is_scripter = true;

    templ.push_back(new TemplWrapper<LuaTempl>("LuaTempl"));
}