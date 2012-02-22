/*
 * menu.h
 *
 *  Created on: 5 Feb 2012
 *      Author: student
 */

#ifndef MENU_H_
#define MENU_H_

#include "states.h"
#include "threads.h"

#define MENU_STR_NUM  5

void menu_select(void);
void show_choice(int);
extern void wifi_scan(void);
extern void volume(void);

#endif /* MENU_H_ */
