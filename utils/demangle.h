/*
 * demangle.h
 *
 *  Created on: 10 жовт. 2014
 *      Author: oleg
 */

#ifndef DEMANGLE_H_
#define DEMANGLE_H_

#if defined(__GNUC__) || defined(__clang__)
#include <cxxabi.h>
#endif

// Ported from sol project: https://github.com/Rapptz/sol/blob/master/sol/demangle.hpp

#ifdef _MSC_VER
template<class T>
inline std::string get_type_name() {
    return typeid(T).name();
}

#elif defined(__GNUC__) || defined(__clang__)
template<class T>
inline std::string get_type_name() {
    int status;
    char* unmangled = abi::__cxa_demangle(typeid(T).name(), 0, 0, &status);
    std::string realname = unmangled;
    std::free(unmangled);
    return realname;
}

#else
#error Compiler not supported for demangling
#endif // compilers

#endif /* DEMANGLE_H_ */
