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
#include <demangle.h>

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

    bd_cstring getName() const
    {
        return name;
    }

    bd_block_type getType() const
    {
        return type;
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

    void setPosition(bd_u64 pos)
    {
        bd_result_throw_f(block_io->set_pos(block_io, pos), "Cannot set position %llu", pos);
    }

    void shiftPosition(bd_u64 offset)
    {
        bd_result_throw_f(block_io->shift_pos(block_io, offset), "Cannot shift position on %llu", offset);
    }

    BlockTemplBase* get(const char *block_name, bd_u32 index = 0) const;

    const BlockTemplBase& getBlock(const char *block_name, bd_u32 index = 0) const
    {
        return *get(block_name, index);
    }

#pragma GCC diagnostic ignored "-Wreturn-local-addr"
    template<class T>
    inline T value() const
    {
        bd_require_true(type != BD_TEMPL, "Cannot get value of template object");

        T val;
        get_data(offset, sizeof(T), &val);
        return val;
    }
#pragma GCC diagnostic pop

    std::string getString() const;

    bd_block_io* getBlockIo() { return block_io; }

    void getData(void* val) const
    {
        bd_require_true(type != BD_TEMPL, "Cannot get data of template object");

        get_data(offset, size, val);
    }

    // casting operators
#define BD_BLOCK_TYPE_DECL(name, tp)                                                                                 \
        operator tp() const {                                                                                        \
            bd_require_true_f(type == BD_##name, "Cannot cast value of template object of type %d to " #name, type); \
            tp val; get_data(offset, sizeof(tp), &val); return val; }
    BD_BLOCK_TYPES
#undef BD_BLOCK_TYPE_DECL

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
    BlockTempl(bd_block_io* _blob, bd_cstring _var_name, bd_u32 _count, BlockTemplBase* _parent)
        : BlockTemplBase(_blob, _var_name, (bd_cstring) get_type_name<T>().c_str(), _type, sizeof(T), _count, _parent)
    {
    }

    BlockTempl(bd_block_io* _block_io, bd_cstring _var_name, bd_cstring _type_name, bd_u32 _count, BlockTemplBase* _parent)
        : BlockTemplBase(_block_io, _var_name, _type_name, _type, sizeof(T), _count, _parent)
    {
    }

    inline T value() const
    {
        return BlockTemplBase::value<T>();
    }

    void apply() {}
};

// Simple type templates
#define BD_BLOCK_TYPE_DECL(name, tp)                                                                                 \
        class name : public BlockTempl<name##_T, BD_##name> {                                                        \
        public: name(bd_block_io* _block_io, bd_cstring _var_name, bd_u32 _count, BlockTemplBase* _parent) :         \
        BlockTempl(_block_io, _var_name, _count, _parent) {}};
    BD_BLOCK_TYPES
#undef BD_BLOCK_TYPE_DECL

bool operator ==(const BlockTemplBase& val1, const char* val2);

#endif /* DEFAULT_TEMPL_BASE_H_ */
