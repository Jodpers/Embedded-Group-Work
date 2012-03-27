/*
 * @file menu.h
 *
 *  Created on 5 Feb 2012
 *     @author Pete Hemery
 */

#ifndef MENU_H_
#define MENU_H_


#define MENU_STR_NUM  5

void menu_select(void);
void show_choice(int);

extern void set_menu(BYTE);
extern void wifi_scan(void);
extern void volume(void);

#endif /* MENU_H_ */
