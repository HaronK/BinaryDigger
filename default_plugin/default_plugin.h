/*
 * BinaryDiggerMacro.h
 *
 *  Created on: Jul 12, 2013
 *      Author: oleg
 */

#ifndef HIERARCHYDIGGERMACRO_H_
#define HIERARCHYDIGGERMACRO_H_

#include "default_templ_base.h"

#define VAL(var) var->value()
#define GET(obj, type, field) const type::value_type field = obj->block<type>(#field).value()
//#define LOCAL(name, val)

#define THROW(msg, ...) throw BlockTemplException(msg, __VA_ARGS__)
#define REQUIRE(cond, msg, ...) if (!cond) THROW(msg, __VA_ARGS__)

// Registering user defined structure
//#define REGISTER_STRUCT(name) const int BD_##name = BD_STRUCT;

#define ARR(type, var, count, ...) \
    type* var = new type(block_io, (bd_cstring) #var, count, this); var->apply(__VA_ARGS__)
#define VAR(type, var, ...) ARR(type, var, 0, __VA_ARGS__)

// -----------------------------------------------------------------------------------------
// Templates
// -----------------------------------------------------------------------------------------

#define TEMPL_DECL(templ_name, ...)                                                                       \
    class templ_name : public BlockTempl<templ_name> { public:                                            \
        templ_name(bd_block_io* _block_io, bd_cstring _var_name, bd_u32 _count, BlockTemplBase* _parent); \
        void apply(__VA_ARGS__); };

#define TEMPL_IMPL(templ_name, ...)                                                                              \
    templ_name::templ_name(bd_block_io* _block_io, bd_cstring _var_name, bd_u32 _count, BlockTemplBase* _parent) \
        : BlockTempl(_block_io, _var_name, (bd_cstring) #templ_name, _count, _parent) {}                         \
    void templ_name::apply(__VA_ARGS__) {

#define TEMPL(templ_name, ...)          \
    TEMPL_DECL(templ_name, __VA_ARGS__) \
    TEMPL_IMPL(templ_name, __VA_ARGS__)

#define TEMPL_END \
    this->size = getPosition() - this->offset; }

// -----------------------------------------------------------------------------------------
// Plugin
// -----------------------------------------------------------------------------------------

template<class T>
class TemplWrapper: public RegisteredTemplWrapper
{
public:
//    TemplWrapper() : RegisteredTemplWrapper(typeid(T).name()) {}
    TemplWrapper(const char* type_name) : RegisteredTemplWrapper(type_name) {}

    bd_block* applyTemplate(bd_block_io *block_io, bd_cstring /*script*/)
    {
        bd_require_true(block_io != 0, ("Parameter 'blob' is null"));

        T* val = new T(block_io, (bd_cstring) "root", 0, 0);
        val->apply();

        return val;
    }

    void freeTemplate(bd_block *block)
    {
        bd_require_true(block != 0, ("Parameter 'block' is null"));

        delete (T *) block;
    }
};


typedef std::vector<RegisteredTemplWrapper*> RegisteredTemplates;

#define PLUGIN(plugin_name) void register_plugin(bd_string *name, RegisteredTemplates& templ, bool& is_scripter) { \
        *name = (bd_string) #plugin_name; is_scripter = false;

#define TEMPL_REGISTER(name) templ.push_back(new TemplWrapper<name>(#name))

#define PLUGIN_END }

#endif /* HIERARCHYDIGGERMACRO_H_ */
