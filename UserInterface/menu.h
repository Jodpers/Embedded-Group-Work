/*
 * @file menu.h
 *
 *  Created on 5 Feb 2012
 *     @author Pete Hemery
 */

#ifndef MENU_H_
#define MENU_H_


void menu_select(void);
void show_choice(int);
void setup_scroll_delay(void);

int continous();

extern void set_menu(BYTE);
extern void wifi_scan(void);
extern void volume(void);

extern int set_scroll_delay(int);
extern int get_scroll_delay(void);

#endif /* MENU_H_ */
