/*----------------------------------------------------------------------------+
 (c) NXP B.V. 2009-2011. All rights reserved.

 Disclaimer
 1. The NXP Software/Source Code is provided to Licensee "AS IS" without any
    warranties of any kind. NXP makes no warranties to Licensee and shall not
    indemnify Licensee or hold it harmless for any reason related to the NXP
    Software/Source Code or otherwise be liable to the NXP customer. The NXP
    customer acknowledges and agrees that the NXP Software/Source Code is
    provided AS-IS and accepts all risks of utilizing the NXP Software under
    the conditions set forth according to this disclaimer.

 2. NXP EXPRESSLY DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED, INCLUDING,
    BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
    FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT OF INTELLECTUAL PROPERTY
    RIGHTS. NXP SHALL HAVE NO LIABILITY TO THE NXP CUSTOMER, OR ITS
    SUBSIDIARIES, AFFILIATES, OR ANY OTHER THIRD PARTY FOR ANY DAMAGES,
    INCLUDING WITHOUT LIMITATION, DAMAGES RESULTING OR ALLEGDED TO HAVE
    RESULTED FROM ANY DEFECT, ERROR OR OMMISSION IN THE NXP SOFTWARE/SOURCE
    CODE, THIRD PARTY APPLICATION SOFTWARE AND/OR DOCUMENTATION, OR AS A
    RESULT OF ANY INFRINGEMENT OF ANY INTELLECTUAL PROPERTY RIGHT OF ANY
    THIRD PARTY. IN NO EVENT SHALL NXP BE LIABLE FOR ANY INCIDENTAL,
    INDIRECT, SPECIAL, EXEMPLARY, PUNITIVE, OR CONSEQUENTIAL DAMAGES
    (INCLUDING LOST PROFITS) SUFFERED BY NXP CUSTOMER OR ITS SUBSIDIARIES,
    AFFILIATES, OR ANY OTHER THIRD PARTY ARISING OUT OF OR RELATED TO THE NXP
    SOFTWARE/SOURCE CODE EVEN IF NXP HAS BEEN ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGES.

+-----------------------------------------------------------------------------+
|       Project: Generic RKE Application                                      |
|                                                                             |
+-----------------------------------------------------------------------------+
| Date      | File Version | Firmware Version | Comment                       |
+-----------------------------------------------------------------------------+
+-----------------------------------------------------------------------------+
| 25-Feb 11 | 1.00         | 1.00             | Initial Release               |
+-----------------------------------------------------------------------------+
| Description                                                                 |
|                                                                             |
| Button debouncing related functions.                                        |
|                                                                             |
+----------------------------------------------------------------------------*/


//******************************************************************************
// Includes
//******************************************************************************

// hardware registers
#include "io.h"

// function and data include
#include "defs.h"
#include "data_dispatch.h"

#include "buttons.h"
#include "timer.h"
#include "ee_prom.h"
//******************************************************************************
// local prototypes
//******************************************************************************

uint8_t btn_detect_bid (void);

uint8_t btn_buttons_debounce (void);

//******************************************************************************
// global function
//******************************************************************************
uint8_t btn_get_bid (void)
{
  uint8_t u8_ButtonCode;

  u8_ButtonCode = 0x00;

  u8_ButtonCode = btn_buttons_debounce ();

  return (u8_ButtonCode);
}
//******************************************************************************
// local functions
//******************************************************************************
uint8_t btn_detect_bid (void)
{
  uint8_t u8_ButtonCode;

  u8_ButtonCode = 0x00;

  // Buttons are low active

#ifdef BOARD_SMART1_50OHMS
  // 50 Ohm board:
  // SW1 = P11 = P1INS.1
  // SW2 = P21 = P2INS.1
  // SW3 = P10 = P1INS.0
  u8_ButtonCode  = (((~P1INS) & 0x01)<<2);        // SW3 = MSB
  u8_ButtonCode |= ((~P2INS) & 0x02);             // SW2
  u8_ButtonCode |= (((~P1INS) & 0x02)>>1);        // SW1
  // a pressed button delivers a logical ONE: u8_ButtonCode = [0 0 0 0  0 sw3 sw2 sw1]
#endif

#ifdef BOARD_SMART1_DEMO_SHA
// for new demo board by S&A Shanghai (2012 March)
  // SW1 = P14 = P1INS.4
  // SW2 = P10 = P1INS.0
  // SW3 = P11 = P1INS.1
  // SW4 = P21 = P2INS.1
  u8_ButtonCode  = (((~P2INS) & 0x02)<<2);        // SW4 = MSB
  u8_ButtonCode |= (((~P1INS) & 0x02)<<1);        // SW3
  u8_ButtonCode |= (((~P1INS) & 0x01)<<1);        // SW2
  u8_ButtonCode |= (((~P1INS) & 0x10)>>4);        // SW1
  // a pressed button delivers a logical ONE: u8_ButtonCode = [0 0 0 0  sw4 sw3 sw2 sw1]
#endif

  return (u8_ButtonCode);
}


//******************************************************************************
uint8_t btn_buttons_debounce (void)
{
  uint8_t   u8arr_ReloadArray[]={2,3,5,7,9,11,13,17,23,29,31,37,41,47,53,59};

  uint8_t   u8_NumOfSamples;                     // number of samples points in a single loop

  uint8_t   u8_CurBID;                           // current button code (BID)

  uint8_t   u8_Error;                            // error counter for bouncing events

  uint8_t   u8_T1Prescaler;                      // selectable prescaler for timer 1

  uint8_t   u8_LoopCounter;                      // overall debouncing loops

  uint8_t   u8_ErrorLimit;                       // max. number of debouncing events (errors)

  uint8_t   i,k;                                 // indeces

  int8_t    i8arr_bdeb[2];                        // settings array (read from EEPROM)

  int8_t    i8_RkeCfgPage;                        // start page of RKE configuration

  //*************************************************
  /*
  BDEB Coding:
  EEPROM: BDEB [1,0] is in page 7, bit [31:16]

  The data is read from EEPROM BDEB[1,0] to an array i8arr_bdeb[0,1] MSB first!
  Thus the array indexing is equal to the definition in the specification.

  => EE_BDEB[1] bit [7:4] => BDEB[0] bit [7:4] => number of errors (error threshold)
     EE_BDEB[1] bit [3:0] => BDEB[0] bit [3:0] => number of detection samples in sample round
     EE_BDEB[0] bit [7:4] => BDEB[1] bit [7:4] => number of debouncing loops
     EE_BDEB[0] bit [3:0] => BDEB[1] bit [3:0] => scaling factor for time base between two detection samples (nominal 1 us)
  */
  //***********************************************************
  //dat_get_bdeb (i8arr_bdeb);
  i8_RkeCfgPage = dat_get_rke_cfg_page ();

  ee_read_data (i8arr_bdeb,i8_RkeCfgPage+BDEB_OFFSET,0,16);            // page 7:   read bit [31-16], 2 byte buttons debounce config
  //***********************************************************
  u8_NumOfSamples = (i8arr_bdeb[0] & 0x0F) + 1;             // 0: 1 samples, F: 16 samples (EE_BDEB byte 1, LSB)
  //***********************************************************
  u8_T1Prescaler = (i8arr_bdeb[1] & 0x07);                  // mask valid bits (EE_BDEB byte 0, LSB)

  /*
    select final prescaler for timer 1

    bdeb = 0: T1S[2:0] = 2 => prescaler 4,   Tosz = 1  us  (min. value)
           1: T1S[2:0] = 3 => prescaler 8,   Tosz = 2  us
           2: T1S[2:0] = 4 => prescaler 16,  Tosz = 4  us
           3: T1S[2:0] = 5 => prescaler 32,  Tosz = 8  us
           4: T1S[2:0] = 6 => prescaler 64,  Tosz = 16 us
           5: T1S[2:0] = 7 => prescaler 128, Tosz = 32 us

    bdeb = 6,7 => don't care, use bdeb = 0 instead
  */

  if (u8_T1Prescaler < 6)               // bdeb = [0:5]
  {
    u8_T1Prescaler += 2;
  }
  else
  {
    u8_T1Prescaler = 2;                 // set fixed prescaler, Tosz = 1 us
  }
  //***********************************************************
  u8_LoopCounter = ((i8arr_bdeb[1] & 0xF0) >> 4) + 1;      // get total number of sample loops 0: 1 loop, F: 16 loops (EE_BDEB byte 0, MSB)
  //***********************************************************
  u8_ErrorLimit = (i8arr_bdeb[0] & 0xF0) >> 4;             // get max. number of errors (EE_BDEB byte 1, MSB)
  // to get an overall error threshold, the given error limit
  // is multiplied to the number of debouncing loops
  u8_ErrorLimit *= u8_LoopCounter;                         // calculate overall error limit (N loops * max. number of errors)
  //***********************************************************
  u8_Error = 0;                                            // reset error counter
  //***********************************************************

  for (k=0; k<u8_LoopCounter; k++)                         // debouncing loop, depend on loop counter BDEB[1] bit [7:4]
  {
    for (i=0; i<u8_NumOfSamples; i++)                      // detection loop, depends on set number of detection samples BDEB[0] bit [3:0]
    {
      //timer1_delay (i8_T1Tosc,u8_T1Prescaler,u8arr_ReloadArray[i]); // select Fosz = 4 MHz, Prescaler depends on above selection
      timer1_delay (T1_TOSC,u8_T1Prescaler,u8arr_ReloadArray[i]);   // select Fosz = 4 MHz, Prescaler depends on above selection

      u8_CurBID = btn_detect_bid();

      if (u8_CurBID == 0)                     // if button pressed, bid >0, if bid=0, no button is pressed or button is bouncing
      {
        u8_Error++;                           // increment error counter until upper error limit (threshold) is reached
      }

      if (u8_Error > u8_ErrorLimit)          // error limit exceeded?
      {
        return 0;                            //  return value to ensure power-off on error
      }
    }
  }
  return (u8_CurBID);                       // get debounced BID
}
//******************************************************************************