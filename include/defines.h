/*
 * definess.h
 *
 *  Created on: Jul 14, 2013
 *      Author: oleg
 */

#ifndef DEFINES_H_
#define DEFINES_H_

#ifdef  __cplusplus
#define BD_C_EXTERN_BEGIN extern "C" {
#define BD_C_EXTERN_END   }
#else
#define BD_C_EXTERN_BEGIN
#define BD_C_EXTERN_END
#endif

#define BD_EXPORT

#endif /* DEFINES_H_ */
