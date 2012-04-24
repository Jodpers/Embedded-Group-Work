/******************************************************************************************
 * debug.c                                                                                *
 * Description: This is used to allow the debug statements to be switched on or off       *
 * Author: James Sleeman                                                                  *
 *****************************************************************************************/

#ifndef DEBUG_H_
#define DEBUG_H_

#ifdef DEBUG
#define printd(...) printf (__VA_ARGS__)
#else
#define printd(...)
#endif

#endif /* DEBUG_H_ */
