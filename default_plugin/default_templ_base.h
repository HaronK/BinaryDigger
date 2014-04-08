/*
 * default_templ_base.h
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

class DefaultTemplException : public std::exception
{
public:
    DefaultTemplException(bd_result result, const std::string& msg) : result(result), message(msg) {}

    DefaultTemplException(const std::string& msg) : result(BD_EUSER), message(msg) {}

    ~DefaultTemplException() _GLIBCXX_USE_NOEXCEPT {}

    bd_result getResult() const { return result; }

    const char* getMessage() const { return message.c_str(); }

    const char* what() const _GLIBCXX_USE_NOEXCEPT { return message.c_str(); }

private:
    bd_result result;
    std::string message;
};

#define bd_throw(msg) throw DefaultTemplException(msg)
#define bd_throw_f(msg, ...) bd_throw(Poco::format(msg, __VA_ARGS__))

#define bd_require_true(cond, msg) if (!(cond)) bd_throw(msg)
#define bd_require_false(cond, msg) if (cond) bd_throw(msg)

#define bd_require_is_null(cond, msg)  bd_require_true(cond == 0, msg)
#define bd_require_not_null(cond, msg) bd_require_true(cond != 0, msg)

#define bd_require_eq(v1, v2, msg) bd_require_true(v1 == v2, msg)
#define bd_require_ne(v1, v2, msg) bd_require_true(v1 != v2, msg)

#define bd_require_true_f(cond, msg, ...) if (!(cond)) bd_throw_f(msg, __VA_ARGS__)
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

class DefaultTemplBase : public bd_item
{
public:
    DefaultTemplBase(bd_templ_blob *_blob, bd_cstring _name, bd_cstring _type_name, bd_item_type _type, bd_u64 _size,
            bd_u32 _count, DefaultTemplBase *_parent);

    ~DefaultTemplBase();

    bd_u64 getSize() const
    {
        return size;
    }

    bd_u64 getPosition()
    {
        bd_u64 result;
        bd_result_throw(blob->get_pos(blob, &result), "Cannot get position");
        return result;
    }

    void setPosition(bd_u64 pos)
    {
        bd_result_throw_f(blob->set_pos(blob, pos), "Cannot set position %llu", pos);
    }

    void shiftPosition(bd_u64 offset)
    {
        bd_result_throw_f(blob->shift_pos(blob, offset), "Cannot shift position on %llu", offset);
    }

    const DefaultTemplBase& getItem(const char* item, bd_u32 index = 0) const;

    template<class T>
    const T& operator [](bd_u32 index)
    {
        bd_require_true(is_array == BD_TRUE, "Cannot access non array element by index");
        bd_require_false_f(index < count, "Index out of bounds: %u >= %u", index, count);
        bd_require_false_f(size == sizeof(T),
                "Sizes of current element and requested one are not equal: %llu != %u", size, sizeof(T));

        if (type == BD_IT_TEMPL)
        {
            return (T) *(children_vec[index]);
        }

        // simple type
        T val;
        get_data(offset + index * sizeof(T), sizeof(T), &val);
        return val;
    }

protected:
    bd_templ_blob *blob;

    typedef std::vector<DefaultTemplBase*> Children;
    Children children_vec;

    void get_data(bd_u64 _offset, bd_u64 _size, bd_pointer _val) const
    {
        bd_result_throw_f(blob->get_datap(blob, _offset, _size, _val), "Cannot read data at position %llu, size %llu",
                _offset, _size);
    }

    bd_u32 add_child(DefaultTemplBase* child)
    {
        children_vec.push_back(child);

        children.child = (bd_item**)children_vec.data();
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
template<class T, bd_item_type _type = BD_IT_TEMPL>
class DefaultTempl : public DefaultTemplBase
{
public:
    typedef DefaultTempl<T, _type> class_type;
    typedef T value_type;

//    DefaultTempl(bd_templ_blob* _blob, bd_cstring _var_name, bd_u32 _count, DefaultTemplBase* _parent)
//        : DefaultTemplBase(_blob, _var_name, (bd_cstring) typeid(T).name(), _type, sizeof(T), _count, _parent)
//    {
//    }

    DefaultTempl(bd_templ_blob* _blob, bd_cstring _var_name, bd_cstring _type_name, bd_u32 _count, DefaultTemplBase* _parent)
        : DefaultTemplBase(_blob, _var_name, _type_name, _type, sizeof(T), _count, _parent)
    {
    }

    const T& operator ()() const
    {
        bd_require_true(is_array == BD_FALSE, "Cannot get value of array element");
        bd_require_true(type != BD_IT_TEMPL, "Cannot get value of template element");

        return value();
    }

    const T& operator [](bd_u32 i)
    {
        return DefaultTemplBase::operator []<T>(i);
    }

    void getData(void* val) const
    {
        bd_require_true(type != BD_IT_TEMPL, "Cannot get data of template object");

        get_data(offset, size, val);
    }

#pragma GCC diagnostic ignored "-Wreturn-local-addr"
    inline const T& value() const
    {
        bd_require_true(type != BD_IT_TEMPL, "Cannot get value of template object");

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
        bd_require_true(type != BD_IT_TEMPL, "Cannot compare templates");

        return value() == other.value();
    }

    template<class I>
    const I& item(const char* _item, bd_u32 index = 0) const
    {
        const DefaultTemplBase& templ = getItem(_item, index);
        return static_cast<const I&>(templ);
    }

    template<class I>
    const I& valueOf(const char* _item, bd_u32 index = 0) const
    {
        return item<I>(_item, index).value();
    }

    void apply() {}

    bd_templ_blob* getBlob() { return blob; }
};

#define DECL_SIMPLE_TEMPL(type)                                                                          \
    class type : public DefaultTempl<type##_T, BD_IT_##type> {                                           \
    public: type(bd_templ_blob* _blob, bd_cstring _var_name, bd_u32 _count, DefaultTemplBase* _parent) : \
    DefaultTempl(_blob, _var_name, (bd_cstring) #type, _count, _parent) {}}

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

    virtual bd_item* applyTemplate(bd_templ_blob *blob, bd_cstring script) = 0;
    virtual void freeTemplate(bd_item *item) = 0;
};

#endif /* DEFAULT_TEMPL_BASE_H_ */
