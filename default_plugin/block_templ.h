/*
 * block_templ.h
 *
 *  Created on: 25 жовт. 2014
 *      Author: oleg
 */

#ifndef BLOCK_TEMPL_H_
#define BLOCK_TEMPL_H_

#include "block_templ_base.h"

/**
 * Base class for the template structures declared via BD_TEMPL_DECL or BD_TEMPL macros.
 * Also it used directly for simple C datatypes (char, int, ... or plain structures).
 *
 * @param T Data type
 * @param simple_type true if
 */
template<class T, bd_block_type _type = BD_TEMPL>
class BlockTempl : public BlockTemplBase
{
public:
    BlockTempl(bd_block_io* _blob, bd_cstring _var_name, bd_u32 _count, BlockTemplBase* _parent, const bd_property_records &props)
        : BlockTemplBase(_blob, _var_name, (bd_cstring) get_type_name<T>().c_str(), _type, sizeof(T), _count, _parent, props)
    {
    }

    BlockTempl(bd_block_io* _block_io, bd_cstring _var_name, bd_cstring _type_name, bd_u32 _count, BlockTemplBase* _parent, const bd_property_records &props)
        : BlockTemplBase(_block_io, _var_name, _type_name, _type, sizeof(T), _count, _parent, props)
    {
    }

    inline T value()
    {
        return static_cast<T>(*this);
    }

    void apply() {}
};

// Simple type templates
class STRING : public BlockTempl<STRING_T, BD_STRING>
{
public:
    STRING(bd_block_io* _block_io, bd_cstring _var_name, bd_u32 _count, BlockTemplBase* _parent,
           const bd_property_records &props) : BlockTempl(_block_io, _var_name, (bd_cstring) "string", _count, _parent, props)
    {}
};

#define BD_BLOCK_TYPE_DECL(name, tp)                                                                         \
        class name : public BlockTempl<name##_T, BD_##name> {                                                \
        public: name(bd_block_io* _block_io, bd_cstring _var_name, bd_u32 _count, BlockTemplBase* _parent,   \
                const bd_property_records &props) : \
        BlockTempl(_block_io, _var_name, _count, _parent, props) {}};
    BD_BLOCK_TYPES
#undef BD_BLOCK_TYPE_DECL

#endif /* BLOCK_TEMPL_H_ */
