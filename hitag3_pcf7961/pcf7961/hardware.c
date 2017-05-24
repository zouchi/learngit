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

+------------------------------------------------------------------------------------------+
|       Project: Generic RKE Application                                                   |
|                                                                                          |
+------------------------------------------------------------------------------------------+
| Date      | File Version | Firmware Version | Comment                                    |
+------------------------------------------------------------------------------------------+
+------------------------------------------------------------------------------------------+
| 25-Feb 11 | 1.00         | 1.00             | Initial Release                            |
+------------------------------------------------------------------------------------------+
| 19-Mar-12 | 1.0          | none             | Change LED port for Smart1                 |
|           |              |                  | board                                      |
+------------------------------------------------------------------------------------------+
| 30-Mar-12 | 1.1          | none             | Remove Timer0 enable/disable               |
|           |              |                  | and relocate frame end position            |
+------------------------------------------------------------------------------------------+
| 19-Apr-12 | 1.2          |                  | adapted start-up procedure in              |
|           |              |                  | hw_transmitter_on() acc. to recommendation.|
|           |              |                  | changed TR0 to TCON in hw_modulator_stop().|
+------------------------------------------------------------------------------------------+
| 24-Apr-12 | 1.2          |                  | removed final 2 bit tx in                  |
|           |              |                  | hw_transmitter_off().                      |
|           |              |                  |                                            |
+------------------------------------------------------------------------------------------+
| Description                                                                              |
|                                                                                          |
| Basic Hardware support functions for the PCF7961.                                        |
|                                                                                          |
+------------------------------------------------------------------------------------------+
*/

// hardware registers
#include "io.h"

// Function and data includes
#include "timer.h"
#include "defs.h"
#include "types.h"

#include "hardware.h"
#include "romlib_parser.h"

//******************************************************************************
void hw_init (void)
{
  //**********************************************************
  // Init:
  //**********************************************************
  SCSL = SCSL_CLK;                   // select 2MHz CPU clock

  P1OUT = D_P1OUT;
  P1DIR = D_P1DIR;
  P2OUT = D_P2OUT;
  P2DIR = D_P2DIR;
}

//******************************************************************************
void hw_watchdog_init (void)
{
   WTCON = 0;                       // Configure and start RKE watchdog timeout (typ 0.5sec)
}

//******************************************************************************
void hw_modulator_start (void)
{
  timer0_start (T0_CLOCK_SRC,T0_PRESCALER,T0_RELOAD);   // start transmission timer

  generic_mod_start (D_MODCON);                         // ROM-Lib: start modulator
}

//******************************************************************************
void hw_modulator_stop (void)
{
  MODCON = 0x00;                  // stop modulator

  TCON   = 0x00;                  // stop timer 0
}

//******************************************************************************
void hw_transmitter_on (void)
{
  // System call:
  // Initialize the crystal oscillator section of the built-in transmitter 
  // with factory-programmed calibration data. 
  // Remarks: This function needs to be called in any case before any transmitter 
  // configuration and operation is performed.
  // CPU cycles:  32  (approximate value) 
  PLL_XO_INIT();
  
  PACON = 0x00;                         // clear power amplifier control, output off

  // Per recommendation in DS, set FCON=0 before XTAL start-up, to apply maximum load capacitance
  XFCON = 0;
   
  // load transmitter control registers
  TXCON1 = D_STARTUP_TXCON1;            // XEN=1 means enable oscillator
  TXCON2 = D_TXCON2;                    // TXON (bit 0) is handled separatedly below

  // Oscillator startup time (1 ms)
  timer1_delay( T1_REF_CLOCK, T1_PRESCALER_1, T1_OSC_DELAY );

  TXCON1 = D_TXCON1;                    // set OG1,0 for ASK/FSK operation
  // load oscillator frequency control (ASK uses FML setting, FSK switches FML-FMH)
  TXCON1_bit.XFCS = 1;                  // write to FMH
  XFCON = D_XFCON_FMH;
  TXCON1_bit.XFCS = 0;                  // write to FML
  XFCON = D_XFCON_FML;

  TXCON2_bit.TXON = 1;                  // (TXON=bit0) enable transmitter

  // PLL startup time (200 us)
  timer1_delay( T1_REF_CLOCK, T1_PRESCALER_1, T1_PLL_DELAY );

  PACON = D_PACON;                      // load Power Amplifier Control register
}

//******************************************************************************
void hw_led_on_off (void)
{
  LED_ON;

  // wait
  timer1_delay( T1_REF_CLOCK, T1_PRESCALER_128, T1_LED_DELAY );

  LED_OFF;
}

//******************************************************************************
void hw_transmitter_off (void)
{
  PACON = 0x00;                               // clear power amplifier control

  TXCON2_bit.TXON = 0;                        // disable transmitter

  TXCON1_bit.XEN = 0;                         // disable oscillator
}

//******************************************************************************
void hw_power_off (void)
{
  P1DIR = 0;                        // sswitch all I/O ports to input
  P2DIR = 0;

  SCSL = 0;                         // set clock source to Tref (fref=125 kHz, Tref=8us)

  timer1_delay( T1_REF_CLOCK, T1_PRESCALER_1, T1_P_OFF_DELAY );    // delay to wait for Power Off

  PCON_bit.PLF = 1;                 // Disconnect VDD from battery 

  while (1);                        // wait for power down
}
//******************************************************************************

