/*****
 Keypad UI defines and prototypes
 28/12/2011 - Pete Hemery
*****/

#define TRUE 	  1
#define FALSE 	0
#define ERROR 	-1

#define COLSX 	4
#define ROWSX 	4

#define BUF_SIZE  10

#define SERIAL_DEVICE 	"/dev/ttyACM0"
#define A 0
#define B 1
#define C 2
#define SLEEP 1400		// Lowest value needed between write and read
#define DELAY 70		// Key press delay
#define SCROLL_DELAY 100   // LED scrolling
#define CUR_TRIGGER  4       // Cursor blinking rate

#define PADDED          3
#define BLOCKING        1
#define NOT_PADDED      0
#define NOT_BLOCKING    0

#define ACCEPT_PLAY    'A'
#define BACK           'B'
#define CANCEL         'C'
#define DELETE         'D'
#define ENTER_MENU     'E'
#define FORWARD        'F'

#define MENU_STR_NUM  5

/* State Table */
enum states{
	EMERGENCY,
	WAITING_LOGGED_OUT,
	INPUTTING_PIN,
	WAITING_LOGGED_IN,
	INPUTTING_TRACK_NUMBER,
	MENU_SELECT,
	MAX_STATES
} current_state;

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int DWORD;

/******************
  7-Seg hex map
    --1--
   20   2
     40  
   10   4
    --8-- 80
*******************/
const BYTE segtab[] = {0x00,0x06,0x5B,0x4F,0x71,0x66,0x6D,0x7D,0x79,
		       0x07,0x7F,0x6F,0x5E,0x77,0x3F,0x7C,0x39};
const BYTE numtab[] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F};//0-9
const BYTE uitab[] = {0x00,'1','2', '3',FORWARD,
                           '4','5', '6',ENTER_MENU,
                           '7','8', '9',DELETE,
                   ACCEPT_PLAY,'0',BACK,CANCEL};
/*                        A,   B,   C,   d,   E,   F,   g,   H,   I,*/
const BYTE alphaU[] = {0x77,0x7F,0x39,0x5E,0x79,0x71,0x6F,0x76,0x30,
/*                        J,   K,   L,   M,   n,   O,   P,   Q,   r,*/
                        0x1E,0x76,0x38,0x15,0x54,0x3F,0x73,0x67,0x50,
/*                        S,   t,   U,   V,   W,   X,   Y,   Z*/
                        0x6D,0x78,0x3E,0x1C,0x2A,0x76,0x6E,0x5B};
/*                        A,   b,   c,   d,   E,   F,   g,   h   i,*/
const BYTE alphaL[] = {0x77,0x7C,0x58,0x5E,0x79,0x71,0x6F,0x74,0x04,
/*                        J,   K,   L,   M,   n,   o,   P,   Q,   r,*/
                        0x1E,0x76,0x38,0x15,0x54,0x5C,0x73,0x67,0x50,
/*                        S,   t,   U,   V,   W,   X,   Y,   Z*/
                        0x6D,0x78,0x3E,0x1C,0x2A,0x76,0x6E,0x5B};

void term_initio();
void term_exitio();
int rs232_open(void);
int rs232_close(void);

void setup_ports();
void write_to_port(int, BYTE);
void closing_time();
void * keypad();

void input_pin(char);
void input_track_number(char);
void menu_select(void);
void show_choice(int choice);

void reset_buffer(void);

void delay();			// Delay between button presses
void scroll_delay(BYTE);			// Delay of digits scrolling
void clear_display();
void cursor_blink();

void del_digit();
void shift_digits_left();
void shift_digits_right();
void shift_digits();

void display_char(char);
void display_string(char *,BYTE,BYTE);

