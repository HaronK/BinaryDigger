/*
 * default_templ_base.cpp
 *
 *  Created on: Jul 14, 2013
 *      Author: oleg
 */

#include "default_templ_base.h"

DefaultTemplBase::DefaultTemplBase(bd_templ_blob *_blob, bd_cstring _name, bd_cstring _type_name, bd_item_type _type,
        bd_u64 _size, bd_u32 _count, DefaultTemplBase *_parent)
{
    this->blob = _blob;
    this->name = strdup(_name);
    this->type_name = strdup(_type_name);
    this->type = _type;
    this->parent = _parent;

    children.child = 0;
    children.count = 0;

    this->is_array = _count > 0;
    this->count = this->is_array ? _count : 1;

    offset = getPosition();

    if (is_array && _type == BD_IT_TEMPL)
    {
        // TODO: probably this array should be marked somehow in parent array of children

        // NOTE: array element lives only inside parent's constructor.
        // Outside of parent's constructor array elements are merged with other parents children.

        // initialize current element
        // we don't register current array element with parent
        // as soon as we register each element separately

        // reserve children elements upfront to avoid several resizings
        children_vec.reserve(this->count);

        for (bd_u32 i = 0; i < this->count; ++i)
        {
            // add single element
//                DefaultTemplBase child;
            new DefaultTemplBase(_blob, _name, _type_name, _type, _size, 0, this);
        }

        bd_u64 pos = getPosition();

        // set proper size of the current element
        // TODO: this is not size of single element of array but size of whole array
        this->elem_size = 0;
        this->size = pos - offset;
    }
    else // not an array or array of simple type
    {
        this->elem_size = _size;
        this->size = _size * this->count;
        if (_type != BD_IT_TEMPL)
            shiftPosition(getSize());
    }
    if (_parent != 0)
        _parent->add_child(this);
}

DefaultTemplBase::~DefaultTemplBase()
{
    free(this->name);
    free(this->type_name);
}

const DefaultTemplBase& DefaultTemplBase::getItem(const char* item, bd_u32 index) const
{
    bd_require_true(item != 0, "Parameter 'item' is null");
    bd_require_true_f(index < children.count, "Index out of bounds: %u >= %u", index, children.count);

    // find first element with such name
    bd_u32 cur_idx = 0;
    for (bd_u32 i = 0; i < children.count; ++i)
    {
        if (strcmp(children.child[i]->name, item) == 0)
        {
            if (cur_idx == index)
            {
                return *static_cast<DefaultTemplBase*>(children.child[i]);
            }
            cur_idx++;
        }
    }
    bd_throw_f("Cannot find item '%s' in '%s' with index %u", item, name, index);
}

// --------------------------------------------------------------------------------------------------------------------

bool operator ==(const CHAR& val1, const char* val2)
{
    bd_require_true(val1.is_array == BD_TRUE, "Cannot compare string with character");
//    bd_require_false(type == BD_IT_CHAR, "Array of '%s'\'s could not be compared with string", type_name);

    bd_u64 len = val1.getSize();
    if (len != strlen(val2))
        return false;

    bd_cstring str = new bd_char[len + 1];
    val1.getData((void *)str);
    str[len] = '\0';
    bool result = strcmp(str, val2) == 0;
    delete[] str;

    return result;
}