/*
 * block_templ_base.cpp
 *
 *  Created on: Jul 14, 2013
 *      Author: oleg
 */

#include "block_templ_base.h"

BlockTemplBase::BlockTemplBase(bd_block_io *_block_io, bd_cstring _name, bd_cstring _type_name, bd_block_type _type,
        bd_u64 _size, bd_u32 _count, BlockTemplBase *_parent)
{
    block_io  = _block_io;
    name      = strdup(_name);
    type_name = strdup(_type_name);
    type      = _type;
    parent    = _parent;

    children.child = 0;
    children.count = 0;

    is_array = _count > 0;
    count    = is_array ? _count : 1;

    offset = getPosition();

    if (is_array && _type == BD_TEMPL)
    {
        // TODO: probably this array should be marked somehow in parent array of children

        // NOTE: array element lives only inside parent's constructor.
        // Outside of parent's constructor array elements are merged with other parents children.

        // initialize current element
        // we don't register current array element with parent
        // as soon as we register each element separately

        // reserve children elements upfront to avoid several resizings
        children_vec.reserve(count);

        for (auto i = 0; i < count; ++i)
        {
            // add single element
            new BlockTemplBase(_block_io, _name, _type_name, _type, _size, 0, this);
        }

        bd_u64 pos = getPosition();

        // set proper size of the current element
        // TODO: this is not size of single element of array but size of whole array
        elem_size = 0;
        size = pos - offset;
    }
    else // not an array or array of simple type
    {
        elem_size = _size;
        size = _size * count;

        if (_type != BD_TEMPL)
            shiftPosition(getSize());
    }

    if (_parent != 0)
        _parent->add_child(this);
}

BlockTemplBase::~BlockTemplBase()
{
    free(name);
    free(type_name);

    // TODO: cleanup children properly
}

BlockTemplBase *BlockTemplBase::get(const char* block_name, bd_u32 index) const
{
    bd_require_true(block_name != nullptr, "Parameter 'block_name' is null");

    if (index == (bd_u32) -1) // search for the last most element
    {
        for (auto i = children.count; i > 0; --i)
        {
            if (strcmp(children.child[i - 1]->name, block_name) == 0)
            {
                return static_cast<BlockTemplBase*>(children.child[i - 1]);
            }
        }
    }
    else
    {
        bd_require_true_f(index < children.count, "Index out of bounds: %u >= %u", index, children.count);

        // find first element with such name
        auto cur_idx = bd_u32{0};
        for (auto i = 0; i < children.count; ++i)
        {
            if (strcmp(children.child[i]->name, block_name) == 0)
            {
                if (cur_idx == index)
                {
                    return static_cast<BlockTemplBase*>(children.child[i]);
                }
                cur_idx++;
            }
        }
    }
    bd_throw_f("Cannot find item '%s' in '%s' with index %u", block_name, name, index);
}

std::string BlockTemplBase::getString() const
{
    bd_require_true(type == BD_CHAR && is_array == BD_TRUE, "Template is not a string");

    auto len = getSize();
    auto str = new bd_char[len + 1];
    getData((void *)str);
    str[len] = '\0';

    auto result = std::string(str, len);
    delete[] str;

    return result;
}

// --------------------------------------------------------------------------------------------------------------------

bool operator ==(const BlockTemplBase& val1, const char* val2)
{
    return (val1.getString() == val2);
}
