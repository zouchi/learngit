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
+----------------------------------------------------------------------------*/


#ifndef __DATA_DISPATCH_H__
#define __DATA_DISPATCH_H__

#include "types.h"                                      // used for type uint8_t and int8_t

void dat_set_rke_cfg_page (int8_t i8_RkeCfgPage);       // RKE config page
int8_t dat_get_rke_cfg_page ();

void dat_set_udf (int8_t __idata * pi8_udf);            // User Defined Field
void dat_get_udf (int8_t __idata * pi8_udf);

void dat_set_rcb_cfg0_byte (int8_t i8_RcbCfg0Byte);     // RCB-Config byte 0 (settings byte)
int8_t dat_get_rcb_cfg0_byte ();

void dat_set_si_page (int8_t i8_SIPage);                // Sequence Increment EEPROM start page
int8_t dat_get_si_page ();

void dat_set_button_code (uint8_t u8_ButtonCode);       // Button Code
uint8_t dat_get_button_code ();

void dat_set_adc_cmp_res (int8_t i8_ADCRes);            // ADC compare result
int8_t dat_get_adc_cmp_res ();

void dat_set_func_byte (int8_t i8_FuncByte);            // FUNC byte
int8_t dat_get_func_byte ();

#endif