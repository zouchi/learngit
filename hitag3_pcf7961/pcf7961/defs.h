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
| 19-Mar 12 | 1.00         | 1.00             | Change port defines for SMART1|
|           |              |                  | demo board                    |
+-----------------------------------------------------------------------------+
| 30-Mar 12 | 1.10         | 1.00             | Update hardware parameters    |
|           |              |                  | for Smart1 board              |
|           |              |                  |                               |
+-----------------------------------------------------------------------------+*/

#ifndef __DEFS_H__
#define __DEFS_H__

// *** select target board ***
#define BOARD_SMART1_50OHMS
//#define BOARD_SMART1_DEMO_SHA


#ifdef BOARD_SMART1_50OHMS

// initial port values
#define D_P1DIR  0x10                     // 00010000b
#define D_P1OUT  0x10                     // 00010000b  P14: LED=1 (off)
#define D_P2DIR  0x00                     // 00000000b
#define D_P2OUT  0x01                     // 00000001b

// LED control
#define LED_ON    P1OUT_bit.P14 = 0
#define LED_OFF   P1OUT_bit.P14 = 1

#endif

#ifdef BOARD_SMART1_DEMO_SHA

// initial port values
#define D_P1DIR  0x00                     // 00000000b
#define D_P1OUT  0x00                     // 00000000b 
#define D_P2DIR  0x04                     // 00000100b
#define D_P2OUT  0x05                     // 00000101b  P22: LED=1 (off)

// LED control
#define LED_ON    P2OUT_bit.P22 = 0
#define LED_OFF   P2OUT_bit.P22 = 1

#endif


// EEPROM pages
#define IDE_PAGE          0
  
#define RKE_CFG_PAGE      4               // EEPROM page of RKE_CFG page number (Hitag 3 encryption)

// EEPROM page offsets 
#define SI_PAGE_OFFSET    8               // EEPROM page offset between RKE_CFG start page and RSI start page

#define HW_PAGE_OFFSET    16              // EEPROM page offset between RKE_CFG start page and hardware config start page

#define WUPC_OFFSET       5               // offset in EEPROM page based on start for RKE_CFG page 0 (WUP-Count) 
#define RCB_OFFSET        5               // offset in EEPROM page based on start for RKE_CFG page 0 (RCB-Count)
#define RCBCFG_OFFSET     5               // offset in EEPROM page based on start for RKE_CFG page 0 (RCB-Count)
#define PRE_OFFSET        6               // offset in EEPROM page based on start for RKE_CFG page 0 (Preamble)
#define VBAT_OFFSET       6               // offset in EEPROM page based on start for RKE_CFG page 0 (VBAT)
#define BDEB_OFFSET       7               // offset in EEPROM page based on start for RKE_CFG page 0 (BDEB)

// Watchdog
#define WDRESET           WTCON_bit.WCLR = 1

// System clock
#define SCSL_CLK          5               // select 2 MHz CPU clock (=maximum)


// Modulator
#define D_MODCON          0x20            // MODCON = 00100000b => ASK (EAM=1, bit 5), select timer 0 (TSEL=0, bit 4)

// Transmitter
#define D_STARTUP_TXCON1  0x01            // TXCON1 (@startup) = xx000001 (x=don't care / XCD = 0 / XFCS = 0 / TM = 0 / OG1,0 = 00 / XEN=1 )
#define D_TXCON1          0x05            // TXCON1 (@operat.) = xx000101 (x=don't care / XCD = 0 / XFCS = 0 / TM = 0 / OG1,0 = 10 / XEN=1 )
#define D_TXCON2          0x24            // TXCON2 = xx1x010x (FBSL = 1 / VOSL = 0 / PAM1,0 = 10 = "high power, low stab.")
#define D_XFCON_FML       0x0B            // XFCON for FML = 0x0B (after Xtal startup)
#define D_XFCON_FMH       0x0B            // XFCON for FMH = 0x0B (after Xtal startup)
#define D_PACON           0xF0            // PACON  = 0xF0 (AMH = 1111b, AML=0000b)


// Timer 0
#define T0_CLOCK_SRC      0x03            // TCS = 11 external source (XEN=1 set with TXCON1 in transmitter_on)
#define T0_PRESCALER      0x01            // TPS = 001 prescaler 2
#define T0_RELOAD         177             // Reload = 177

// Timer 1
#define T1_REF_CLOCK      0x00            // T1CLS[1,0] = 00 reference clock (RC oszillator)
#define T1_TOSC           0x01            // T1CLS[1,0] = 01 4 MHz RC Osc. clock 

#define T1_PRESCALER_1    0x00            // prescaler = 1 (T1S)
#define T1_PRESCALER_128  0x07            // prescaler = 128 (T1S)
#define T1_OSC_DELAY      125             // Oscillator startup time: delay = 125 * 8us = 1 ms, (ref-clock = 125 kHz => 8 us)
#define T1_PLL_DELAY       25             // PLL startup time: delay = 25 * 8us = 200 us
#define T1_P_OFF_DELAY     13             // power off: delay = 13 * 8us = 100 us
#define T1_LED_DELAY      196             // LED on time (125 kHz/128 = 0,976 kHz => 1,024 ms, 1,024 ms * 196 = 200,7 ms) 

// Wake Up Pattern
#define WAKE_UP_BITS       16             // number of Manchester coded zero bits (transmit.c)
 
// Array Buffer (rke.c, hitag3.c, ee_prom.c, data_dispatch.c, si.c)
#define TX_BUF_BYTES       18             // transmit buffer 
                             
#define CHAL_BYTES          8             // Challenge 
#define MAC_BYTES           2             // MAC 
#define UDF_BYTES           2             // User Defined Field (UDF)
#define RSI_BYTES           4             // Sequence Increment (RSI)
#define SEQ_BYTES           6             // Sequence Response
                             
#define IDE_BYTES           4             // IDE 
#define PRE_BYTES           3             // Preamble (max. Preamble bytes read from EEPROM)
#define BDEB_BYTES          2             // Buttons Debounce configuration
                             
#define RSI_PAGES           8             // number of 32 bit RSI pages in EEPROM 

// Masks RCB-Config byte 
//#define CRM_MASK        0x18            // select crypto mode (rke.c) 
#define DUDF_MASK         0x80            // 1000 0000b (disable UDF)
#define DIDE_MASK         0x40            // 0100 0000b (disable IDE)
#define RSIM_MASK         0x04            // 0000 0100b (number of RSI bytes)
#define SRM_MASK          0x03            // 0000 0011b (number of SR bytes)
                        
#define RSKM_MASK         0x20            // select secret key (hitag3.c)


#endif
