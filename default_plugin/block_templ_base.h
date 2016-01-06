/*
 * block_templ_base.h
 *
 *  Created on: Jul 13, 2013
 *      Author: oleg
 */

#ifndef DEFAULT_TEMPL_BASE_H_
#define DEFAULT_TEMPL_BASE_H_

#include <string>
#include <vector>
#include <map>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <bd.h>
#include <demangle.h>
#include "property.h"
#include "exception.h"

class BlockTemplBase : public bd_block
{
public:
    BlockTemplBase(bd_block_io *_block_io, bd_cstring _name, bd_cstring _type_name, bd_block_type _type, bd_u64 _size,
            bd_u32 _count, BlockTemplBase *_parent, const bd_property_records &props);

    ~BlockTemplBase();

    bd_cstring getName() const
    {
        return name;
    }

    bd_block_type getType() const
    {
        return type;
    }

    bd_cstring getTypeName()
    {
        return type_name;
    }

    bool isArray() const
    {
        return is_array == BD_TRUE;
    }

    bd_u64 getSize() const
    {
        return size;
    }

    bd_u64 getPosition()
    {
        bd_u64 result;
        bd_result_throw(block_io->get_pos(block_io, &result), "Cannot get position");
        return result;
    }

    BlockTemplBase *getParent()
    {
        return (BlockTemplBase *) parent;
    }

    void setPosition(bd_u64 pos)
    {
        bd_result_throw_f(block_io->set_pos(block_io, pos), "Cannot set position %llu", pos);
    }

    void shiftPosition(bd_u64 offset)
    {
        bd_result_throw_f(block_io->shift_pos(block_io, offset), "Cannot shift position on %llu", offset);
    }

    BlockTemplBase* get(const char *block_name, bd_u32 index = 0) const;

    BlockTemplBase& getBlock(const char *block_name, bd_u32 index = 0) const
    {
        return *get(block_name, index);
    }

    std::string getString() const;

    std::string to_string();

    bd_block_io* getBlockIo() { return block_io; }

    void getData(void* val) const
    {
        bd_require_true(type != BD_TEMPL, "Cannot get data of template object");

        get_data(offset, size, val);
    }

    // casting operators for default types
#define BD_BLOCK_TYPE_DECL(name, tp)                                                    \
        operator tp() {                                                                 \
            bd_require_true_f(type == BD_##name,                                        \
                    "Cannot cast value of template object of type %d to " #name, type); \
            tp val; get_data(offset, sizeof(tp), &val);                                 \
            return correct_endian(val, (bd_endian) get_property("endian").get<int>()); }
    BD_BLOCK_TYPES
#undef BD_BLOCK_TYPE_DECL

    void set_properties(const bd_property_records &props)
    {
        obj_properties = props;
    }

    bd_property get_property(const std::string &name);

protected:
    bd_block_io *block_io;
    bd_property_records templ_properties;
    bd_property_records obj_properties;

    typedef std::vector<BlockTemplBase*> Children;
    Children children_vec;

    void get_data(bd_u64 _offset, bd_u64 _size, bd_pointer _val) const
    {
        bd_result_throw_f(block_io->get_datap(block_io, _offset, _size, _val),
                "Cannot read data at position %llu, size %llu", _offset, _size);
    }

    bd_u32 add_child(BlockTemplBase* child)
    {
        children_vec.push_back(child);

        children.child = (bd_block**)children_vec.data();
        children.count = children_vec.size();
        return children.count - 1;
    }
};

bool operator ==(const BlockTemplBase& val1, const char* val2);

#endif /* DEFAULT_TEMPL_BASE_H_ */
