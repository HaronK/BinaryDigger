/*
 * demangle.h
 *
 *  Created on: 10 жовт. 2014
 *      Author: oleg
 */

#ifndef DEMANGLE_H_
#define DEMANGLE_H_

#include <string>

#if defined(__GNUC__) || defined(__clang__)
#include <cxxabi.h>
#endif

// Ported from sol project: https://github.com/Rapptz/sol/blob/master/sol/demangle.hpp

#ifdef _MSC_VER
inline std::string get_type_name(const std::type_info &type)
{
    return type.name();
}

#elif defined(__GNUC__) || defined(__clang__)
inline std::string get_type_name(const std::type_info &type)
{
    int status;
    char* unmangled = abi::__cxa_demangle(type.name(), 0, 0, &status);
    std::string realname = unmangled;
    std::free(unmangled);
    return realname;
}

#else
#error Compiler not supported for demangling
#endif // compilers

template<class T>
inline std::string get_type_name()
{
    return get_type_name(typeid(T));
}


#endif /* DEMANGLE_H_ */
