/*
 * main.h
 *
 *  Created on: 5 Feb 2012
 *      Author: Pete Hemery
 */

#ifndef MAIN_H_
#define MAIN_H_

#include "top.h"
#include "states.h"
#include "threads.h"
#include "network.h"
#include "debug.h"

#include <signal.h>

extern void start_threads(void);
extern void closing_time(void);
extern void setup_term();

extern char button;

#endif /* MAIN_H_ */
