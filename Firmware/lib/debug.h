/*
 * debug.h
 *
 *  Created on: 24.05.2015
 *      Author: zaphod
 */

#ifndef DEBUG_H_
#define DEBUG_H_

#include "usb.h"

#define DEBUG_ENABLED

#ifdef DEBUG_ENABLED
	#define DEBUG(X) usb_writeln_string(X);
	#define DEBUGF(X, ...) usb_writeln_formatted(X, __VA_ARGS__);
#else
	#define DEBUG(X)
	#define DEBUGF(X, ...)
#endif


#endif /* DEBUG_H_ */
