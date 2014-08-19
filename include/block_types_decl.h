/*
 * block_types_decl.h
 *
 *  Created on: 19 серп. 2014
 *      Author: oleg
 */

BD_DECL_BLOCK_TYPE(CHAR,   bd_i8)
BD_DECL_BLOCK_TYPE(UCHAR,  bd_u8)
BD_DECL_BLOCK_TYPE(WORD,   bd_i16)
BD_DECL_BLOCK_TYPE(DWORD,  bd_i32)
BD_DECL_BLOCK_TYPE(QWORD,  bd_i64)
BD_DECL_BLOCK_TYPE(DOUBLE, bd_f64)

#undef BD_DECL_BLOCK_TYPE
