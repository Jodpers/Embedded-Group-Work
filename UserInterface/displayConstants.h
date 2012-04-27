/*
 * @file displayConstants.h
 *
 *  Created on 22 Feb 2012
 *     @author Pete Hemery
 */

#ifndef DISPLAYCONSTANTS_H_
#define DISPLAYCONSTANTS_H_

/******************
  7-Seg hex map
   ---1--
   20---2
   --40--
   10---4
   ---8-- 80
*******************/
/* 0-9 */
const BYTE numtab[] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F};
/* Keypad button assignment */
const BYTE uitab[] = {0x00,'1', '2', '3', FORWARD,
                           '4', '5', '6', ENTER_MENU,
                           '7', '8', '9', DELETE,
                   ACCEPT_PLAY, '0',BACK, CANCEL};
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


#endif /* DISPLAYCONSTANTS_H_ */
