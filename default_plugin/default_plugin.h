/*
 * BinaryDiggerMacro.h
 *
 *  Created on: Jul 12, 2013
 *      Author: oleg
 */

#ifndef HIERARCHYDIGGERMACRO_H_
#define HIERARCHYDIGGERMACRO_H_

#include "block_templ.h"
#include <demangle.h>

#define VAL(var) var->value()
#define GET(obj, type, field) type##_T field = (type##_T) obj->getBlock(#field)
// TODO: implement local variables
//#define LOCAL(name, val)

#define THROW(msg, ...) throw BlockTemplException(msg, __VA_ARGS__)
#define REQUIRE(cond, msg, ...) if (!cond) THROW(msg, __VA_ARGS__)

// Register user defined structure
//#define REGISTER_STRUCT(name) const int BD_##name = BD_STRUCT;

#define ARR(type, var, count, ...) \
    auto var = new type(block_io, (bd_cstring) #var, count, this, bd_property_records()); var->apply(__VA_ARGS__)
#define VAR(type, var, ...) ARR(type, var, 0, __VA_ARGS__)

// -----------------------------------------------------------------------------------------
// Templates
// -----------------------------------------------------------------------------------------

#define TEMPL_DECL(templ_name, ...)                                                                       \
    class templ_name : public BlockTempl<templ_name> { public:                                            \
        templ_name(bd_block_io* _block_io, bd_cstring _var_name, bd_u32 _count, BlockTemplBase* _parent,  \
                const bd_property_records &props); \
        void apply(__VA_ARGS__); };

#define TEMPL_IMPL(templ_name, ...)                                                                              \
    templ_name::templ_name(bd_block_io* _block_io, bd_cstring _var_name, bd_u32 _count, BlockTemplBase* _parent, \
            const bd_property_records &props) \
        : BlockTempl(_block_io, _var_name, _count, _parent, props) {}                                                   \
    void templ_name::apply(__VA_ARGS__) {

#define TEMPL(templ_name, ...)          \
    TEMPL_DECL(templ_name, __VA_ARGS__) \
    TEMPL_IMPL(templ_name, __VA_ARGS__)

#define TEMPL_END \
    this->size = getPosition() - this->offset; }

// -----------------------------------------------------------------------------------------
// Plugin
// -----------------------------------------------------------------------------------------

#define bd_check_and_return(cond, msg) if (!(cond)) { \
        PluginContext::setError(msg); return -1; }
#define bd_check_not_null(var) bd_check_and_return(var != nullptr, #var " == null")
#define bd_check_index(index) bd_check_and_return(index < PluginContext::getTemplatesCount(), "Template index is out of range")

class RegisteredTemplWrapper
{
public:
    RegisteredTemplWrapper(const std::string &type_name) : type_name(type_name) {}
    virtual ~RegisteredTemplWrapper() {}

    std::string getName() { return type_name; }

    // TODO: specify properties
    virtual bd_result applyTemplate(bd_block_io *block_io, bd_cstring script, bd_block **result, std::string &err) noexcept = 0;
    virtual bd_result freeTemplate(bd_block *block, std::string &err) noexcept = 0;

private:
    std::string type_name;
};

class PluginContext
{
public:
    static std::string getError()
    {
        return error;
    }

    static void setError(const std::string &err)
    {
        error = err;
    }

    static const size_t getTemplatesCount()
    {
        return registeredTemplates.size();
    }

    static RegisteredTemplWrapper *getTemplate(size_t index)
    {
        return registeredTemplates[index];
    }

    static void addTemplate(RegisteredTemplWrapper *templ)
    {
        return registeredTemplates.push_back(templ);
    }

    static void freeTemplates()
    {
        for (auto iter = registeredTemplates.begin(); iter != registeredTemplates.end(); ++iter)
        {
            delete *iter;
        }
    }

private:
    typedef std::vector<RegisteredTemplWrapper*> array_type;

    static array_type registeredTemplates;
    static std::string error;
};

template<class T>
class TemplWrapper: public RegisteredTemplWrapper
{
public:
    TemplWrapper() : RegisteredTemplWrapper(get_type_name<T>()) {}

    bd_result applyTemplate(bd_block_io *block_io, bd_cstring /*script*/, bd_block **result, std::string &err) noexcept
    {
        bd_check_not_null(block_io);
        bd_check_not_null(result);

        try
        {
            auto val = new T(block_io, (bd_cstring) "root", 0, 0, bd_property_records());
            val->apply();

            *result = val;
        }
        catch (BlockTemplException &ex)
        {
            PluginContext::setError(ex.getMessage());
            return ex.getResult();
        }

        return BD_SUCCESS;
    }

    bd_result freeTemplate(bd_block *block, std::string &err) noexcept
    {
        bd_check_not_null(block);

        delete (T *) block;

        return BD_SUCCESS;
    }
};


#define PLUGIN(plugin_name) void register_plugin(std::string &name, bool &is_scripter) { \
        is_scripter = false; name = #plugin_name;

#define TEMPL_REGISTER(name) PluginContext::addTemplate(new TemplWrapper<name>())

#define PLUGIN_END }

#endif /* HIERARCHYDIGGERMACRO_H_ */
