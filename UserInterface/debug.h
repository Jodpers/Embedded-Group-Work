/*
 * @file debug.h
 *
 *  Created on 07 Mar 2012
 *     @author James Sleeman
 */
#ifndef DEBUG_H_
#define DEBUG_H_

#ifdef DEBUG
#define printd(...) printf (__VA_ARGS__)
#else
#define printd(...)
#endif

#endif /* DEBUG_H_ */
