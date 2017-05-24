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
| 19-Apr 12 |              |                  | moved LED flash behind PA off |
|           |              |                  |                               |
+-----------------------------------------------------------------------------+
| Description                                                                 |
|                                                                             |
| Main Module of Generic RKE Application                                      |
|                                                                             |
+----------------------------------------------------------------------------*/

//******************************************************************************
// System library include (for LED_OFF macro)
//******************************************************************************
#include "io.h"

//******************************************************************************
// parser for generic romlib calls
//******************************************************************************
#include "romlib_parser.h"

//******************************************************************************
// global constants and set/get variable includes
//******************************************************************************
#include "defs.h"
#include "types.h"
#include "data_dispatch.h"

//******************************************************************************
// function includes
//******************************************************************************
#include "hardware.h"
#include "ee_prom.h"
#include "buttons.h"
#include "rke.h"

//******************************************************************************
// main function
//******************************************************************************
void main( void )
{

  uint8_t   u8_Result;
  uint8_t   u8_ButtonCode;                // local variable only to test, if button pressed
  int8_t    i8_ADCRes;                    // u8_Result of ADC_CMP
  int8_t    i8_VBATRef;                   // VBAT reference for ADC_CMP
  int8_t    i8_RkeCfgPage;                // start page of RKE configuration

  //**********************************************************
  // Initialize Ports, System Timer
  //**********************************************************
  hw_init ();

  //**********************************************************
  // EEPROM
  //**********************************************************
  u8_Result = ee_read_rke_cfg ();       // get some necessary config data from EEPROM

  if (u8_Result != 0)                   // no successful EEPROM reading, rke_cfg_page = 0 or 15!
  {
    hw_power_off ();                    // stop application
  }

  //**********************************************************
  // Watchdog
  //**********************************************************
  hw_watchdog_init ();

  //**********************************************************
  // Buttons Debounce
  //**********************************************************
  // get debounced button code
  u8_ButtonCode = btn_get_bid ();

  // u8_ButtonCode is now high active
  if ( u8_ButtonCode == 0 )
  {
     hw_power_off ();                   // stop application
  }

  // write data to data dispatcher
  dat_set_button_code (u8_ButtonCode);

  LED_OFF;

  //**********************************************************
  // Initialize Hardware
  //**********************************************************

  hw_modulator_start ();                // start transmission timer and modulator

  hw_transmitter_on ();

  WDRESET;                              // reset watchdog

  //**********************************************************
  // Check battery voltage
  //**********************************************************

  // get data from dispatcher
  i8_RkeCfgPage = dat_get_rke_cfg_page ();
  // get data from EEPROM
  ee_read_data (&i8_VBATRef,i8_RkeCfgPage+VBAT_OFFSET,24,8);    // page 6:   read bit [7-0], 1 byte VBAT reference

  // Check VBAT, comparison level: VBAT-Ref from EEPROM
  // i8_ADCRes == 1 means VBAT is *higher* than threshold => VBATL == 0
  i8_ADCRes = generic_adc_cmp (0,i8_VBATRef);

  // write data to data dispatcher
  dat_set_adc_cmp_res (i8_ADCRes);

  //**********************************************************
  // RKE
  //**********************************************************

  rke_run ();                           // start main RKE routine

  //**********************************************************
  // Power Off Peripherals
  //**********************************************************

  hw_transmitter_off();                 // switch transmitter off

  hw_modulator_stop();                  // stop modulator

  //**********************************************************
  // LED BLINK
  //**********************************************************

  hw_led_on_off ();


  hw_power_off();                       // finally set system power off (never returns)

}
//******************************************************************************
