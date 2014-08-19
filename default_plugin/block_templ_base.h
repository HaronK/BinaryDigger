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
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <bd.h>

#include <Poco/Format.h>

class BlockTemplException : public std::exception
{
public:
    BlockTemplException(bd_result result, const std::string& msg) : result(result), message(msg)
    {}

    BlockTemplException(const std::string& msg) : result(BD_EUSER), message(msg)
    {}

    ~BlockTemplException() throw()
    {}

    bd_result getResult() const
    {
        return result;
    }

    const char* getMessage() const
    {
        return message.c_str();
    }

    const char* what() const throw()
    {
        return message.c_str();
    }

private:
    bd_result result;
    std::string message;
};

#define bd_throw(msg)        throw BlockTemplException(msg)
#define bd_throw_f(msg, ...) bd_throw(Poco::format(msg, __VA_ARGS__))

#define bd_require_true(cond, msg)  if (!(cond)) bd_throw(msg)
#define bd_require_false(cond, msg) if (cond) bd_throw(msg)

#define bd_require_is_null(cond, msg)  bd_require_true(cond == 0, msg)
#define bd_require_not_null(cond, msg) bd_require_true(cond != 0, msg)

#define bd_require_eq(v1, v2, msg) bd_require_true(v1 == v2, msg)
#define bd_require_ne(v1, v2, msg) bd_require_true(v1 != v2, msg)

#define bd_require_true_f(cond, msg, ...)  if (!(cond)) bd_throw_f(msg, __VA_ARGS__)
#define bd_require_false_f(cond, msg, ...) if (cond) bd_throw_f(msg, __VA_ARGS__)

#define bd_require_is_null_f(cond, msg, ...)  bd_require_true_f(cond == 0, msg, __VA_ARGS__)
#define bd_require_not_null_f(cond, msg, ...) bd_require_true_f(cond != 0, msg, __VA_ARGS__)

#define bd_require_eq_f(v1, v2, msg, ...) bd_require_true_f(v1 == v2, msg, __VA_ARGS__)
#define bd_require_ne_f(v1, v2, msg, ...) bd_require_true_f(v1 != v2, msg, __VA_ARGS__)

#define bd_result_throw(result, msg) { \
    bd_result res = result;            \
    bd_require_true(BD_SUCCEED(res), msg); }

#define bd_result_throw_f(result, msg, ...) { \
    bd_result res = result;                   \
    bd_require_true_f(BD_SUCCEED(res), msg, ## __VA_ARGS__); }

class BlockTemplBase : public bd_block
{
public:
    BlockTemplBase(bd_block_io *_block_io, bd_cstring _name, bd_cstring _type_name, bd_block_type _type, bd_u64 _size,
            bd_u32 _count, BlockTemplBase *_parent);

    ~BlockTemplBase();

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

    void setPosition(bd_u64 pos)
    {
        bd_result_throw_f(block_io->set_pos(block_io, pos), "Cannot set position %llu", pos);
    }

    void shiftPosition(bd_u64 offset)
    {
        bd_result_throw_f(block_io->shift_pos(block_io, offset), "Cannot shift position on %llu", offset);
    }

    const BlockTemplBase& getBlock(const char* block_name, bd_u32 index = 0) const;

    template<class T>
    const T& operator [](bd_u32 index)
    {
        bd_require_true(is_array == BD_TRUE, "Cannot access non array element by index");
        bd_require_false_f(index < count, "Index out of bounds: %u >= %u", index, count);
        bd_require_false_f(size == sizeof(T),
                "Sizes of current element and requested one are not equal: %llu != %u", size, sizeof(T));

        if (type == BD_TEMPL)
        {
            return (T) *(children_vec[index]);
        }

        // simple type
        T val;
        get_data(offset + index * sizeof(T), sizeof(T), &val);
        return val;
    }

protected:
    bd_block_io *block_io;

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
    typedef BlockTempl<T, _type> class_type;
    typedef T value_type;

//    BlockTempl(bd_block_io* _blob, bd_cstring _var_name, bd_u32 _count, BlockTemplBase* _parent)
//        : BlockTemplBase(_blob, _var_name, (bd_cstring) typeid(T).name(), _type, sizeof(T), _count, _parent)
//    {
//    }

    BlockTempl(bd_block_io* _block_io, bd_cstring _var_name, bd_cstring _type_name, bd_u32 _count, BlockTemplBase* _parent)
        : BlockTemplBase(_block_io, _var_name, _type_name, _type, sizeof(T), _count, _parent)
    {
    }

    const T& operator ()() const
    {
        bd_require_true(is_array == BD_FALSE, "Cannot get value of array element");
        bd_require_true(type != BD_TEMPL, "Cannot get value of template element");

        return value();
    }

    const T& operator [](bd_u32 i)
    {
        return BlockTemplBase::operator []<T>(i);
    }

    void getData(void* val) const
    {
        bd_require_true(type != BD_TEMPL, "Cannot get data of template object");

        get_data(offset, size, val);
    }

#pragma GCC diagnostic ignored "-Wreturn-local-addr"
    inline const T& value() const
    {
        bd_require_true(type != BD_TEMPL, "Cannot get value of template object");

        T val;
        get_data(offset, sizeof(T), &val);
        return val;
    }
#pragma GCC diagnostic pop

    template<class I>
    bool operator ==(const I* other) const;

    bool operator ==(const class_type& other) const
    {
        bd_require_true(is_array == BD_FALSE, "Cannot compare arrays");
        bd_require_true(type != BD_TEMPL, "Cannot compare templates");

        return value() == other.value();
    }

    template<class I>
    const I& block(const char* _item, bd_u32 index = 0) const
    {
        const BlockTemplBase& templ = getBlock(_item, index);
        return static_cast<const I&>(templ);
    }

    template<class I>
    const I& valueOf(const char* block_name, bd_u32 index = 0) const
    {
        return block<I>(block_name, index).value();
    }

    void apply() {}

    bd_block_io* getBlockIo() { return block_io; }
};

// Simple type templates
#define DECL_SIMPLE_TEMPL(type)                                                                          \
    class type : public BlockTempl<type##_T, BD_##type> {                                             \
    public: type(bd_block_io* _block_io, bd_cstring _var_name, bd_u32 _count, BlockTemplBase* _parent) : \
    BlockTempl(_block_io, _var_name, (bd_cstring) #type, _count, _parent) {}}

DECL_SIMPLE_TEMPL(CHAR);
DECL_SIMPLE_TEMPL(UCHAR);
DECL_SIMPLE_TEMPL(WORD);
DECL_SIMPLE_TEMPL(DWORD);
DECL_SIMPLE_TEMPL(QWORD);
DECL_SIMPLE_TEMPL(DOUBLE);

bool operator ==(const CHAR& val1, const char* val2);

class RegisteredTemplWrapper
{
    const char* type_name;

public:
    RegisteredTemplWrapper(const char* type_name) : type_name(type_name) {}
    virtual ~RegisteredTemplWrapper() {}

    const char* getName() { return type_name; }

    virtual bd_block* applyTemplate(bd_block_io *block_io, bd_cstring script) = 0;
    virtual void freeTemplate(bd_block *block) = 0;
};

#endif /* DEFAULT_TEMPL_BASE_H_ */
