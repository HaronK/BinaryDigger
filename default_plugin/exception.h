/*
 * exception.h
 *
 *  Created on: 25 жовт. 2014
 *      Author: oleg
 */

#ifndef EXCEPTION_H_
#define EXCEPTION_H_

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

#endif /* EXCEPTION_H_ */
