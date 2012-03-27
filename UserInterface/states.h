/*
 * @file states.h
 *
 *  Created on 5 Feb 2012
 *     @author Pete Hemery
 */

#ifndef STATES_H_
#define STATES_H_


/* State Table */
enum ui_states{
    INIT_STATE,
    EMERGENCY,
    WAITING_LOGGED_OUT,
    INPUTTING_PIN,
    WAITING_LOGGED_IN,
    INPUTTING_TRACK_NUMBER,
    MENU_SELECT,
    SUBMENU_SELECT
} current_state;

enum gst_states{
    STOPPED,
    PLAYING,
    PAUSED
} gstreamer_state;

/* External Variables */
extern int state;
extern BYTE playing;

/* Local Prototypes */
void * state_machine(void);

/* External Prototypes */
extern void input_pin(char);
extern void input_track_number(char);
extern void menu_select(void);

#endif /* STATES_H_ */
