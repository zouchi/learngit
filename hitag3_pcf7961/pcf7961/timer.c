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
| 19-Mar 12 | 1.00         | 1.00             | Change included file for IAR1.31 |
+-----------------------------------------------------------------------------+
| 30-Mar 12 | 1.10         | 1.00             | Add Timer0 disable and enable when Timer1 delay |
+-----------------------------------------------------------------------------+
| Description                                                                 |
|                                                                             |
| Timer related functions.                                                    |
|                                                                             |
+----------------------------------------------------------------------------*/

// hardware registers
#include "io.h"
#include "intrinsics.h"

// Function and data include
#include "defs.h"
#include "timer.h"

//******************************************************************************
void timer0_start (uint8_t u8_TCSClockSrc, uint8_t u8_TPSPrescaler, uint8_t u8_CycleCount)
{
  uint8_t   u8_T0Tcon;
  uint8_t   u8_T0Reload;

  //***************************************************
  u8_TCSClockSrc  = u8_TCSClockSrc & 0x03;                         // mask valid 2 bits

  u8_TPSPrescaler = u8_TPSPrescaler & 0x07;                        // mask valid 3 bits

  u8_T0Tcon = (u8_TCSClockSrc << 2) | (u8_TPSPrescaler << 4);      // assemble TCON register for timer 0

  TCON = u8_T0Tcon;         // load timer 0 control register, TRS0 is set to zero automatically, force timer to stop

  // to ensure, the cycle count represents the number of timer ticks until
  // timer overflow, create twos-complement for upcounting timer
  u8_T0Reload = (~u8_CycleCount) + 1;

  while (TCON_bit.TRS0);          // wait, until timer is in stop mode

  TR0 = u8_T0Reload;              // load timer register with reload value

  TCON_bit.TRS0 = 1;              // finally start timer 0
}

//******************************************************************************
void timer1_delay (uint8_t u8_T1CLSClockSrc, uint8_t u8_T1SPrescaler, uint8_t u8_CompareCount)
{
  uint8_t   u8_T1Con2;

  //*********************************
  u8_T1CLSClockSrc = u8_T1CLSClockSrc & 0x03;         // mask 2 valid bits

  u8_T1SPrescaler = u8_T1SPrescaler & 0x07;           // mask 3 valid bits
  //*********************************
  T1CON1_bit.T1RUN = 0;                               // force timer 1 stop

  // assemble necessary timer control bits [0 0 0 T1S2 T1S1 T1S0 T1CLS1 T1CLS0]
  u8_T1Con2 = u8_T1CLSClockSrc | (u8_T1SPrescaler << 2);

  T1CON2 = u8_T1Con2;             // set timer 1 control register 2 (clock source and prescaler)

  T1CON2_bit.T1RCMP = 1;          // select compare timer (set T1CMP, bit 5)

  IE_bit.ET1O = 1;                // enable compare interrupt for timer 1

  T1CMP = u8_CompareCount;        // load timer compare value to timer 1 compare register

  while (T1CON1_bit.T1RUN);       // wait until timer 1 is in stop mode

  __interrupt_acknowledge(0xF7);  // acknowledge interrupt, clear FT1O flag [1111 0111b] (for safty reasons only)

  T1CON1 = 0xC0;                  // set T1RUN + T1RES bit, reset timer and force to run

  u8_T1Con2 = IE;
  if(IE_bit.ET0 == 1)             //if ET0 enabled, disable it
    IE_bit.ET0 = 0;               // disable (temporarily) timer 0 interrupt (no wake up)
  PCON_bit.IDLE = 1;              // set to IDLE mode, wait for timer 1 compare interrupt
 if((u8_T1Con2 & 0x04) == 0x04)   //if ET0 enabled before enter idle mode, enable it again
    IE_bit.ET0 = 1;               // enable timer 0 interrupt again (wake up possible)

  T1CON1_bit.T1RUN = 0;           // interrupt occurred, force timer 1 to stop

  __interrupt_acknowledge(0xF7);  // acknowledge interrupt, clear FT1O flag [1111 0111b]

  IE_bit.ET1O = 0;                // disable compare interrupt for timer 1
}

//******************************************************************************
