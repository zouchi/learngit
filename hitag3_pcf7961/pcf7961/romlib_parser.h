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

 $Revision: 5 $

+---------------------------------------------------------------------------------------+
|       Project: Generic RKE Application                                                |
|                                                                                       |
+---------------------------------------------------------------------------------------+
| Date      | File Version | Firmware Version | Comment                                 |
+---------------------------------------------------------------------------------------+
+---------------------------------------------------------------------------------------+
| 25-Feb 11 | 1.00         | 1.00             | Initial Release                         |
| 14-Nov 11 |              |                  | (MMr) adapted to IAR EWMRKII 1.30RC3    |
|           |              |                  |                                         |
+---------------------------------------------------------------------------------------+
                                                                                      
+----------------------------------------------------------------------------------------
| Description:
| redefine ROM-Lib names to get generic access. For other platforms, just
| include new compile switch and add appropriate ROM-Lib names. Keep all
| defined names using [generic_] unchanged, otherwise compilation will fail.
|
| Please beware that the parsing is only possible, if the parameters, which are sometimes
| necessary for a function call, are the same for all different platform-versions.
|
| This must be checked individually before extending this parser!
+-----------------------------------------------------------------------------*/

#ifndef __ROMLIB_PARSER_H__
#define __ROMLIB_PARSER_H__

// System include with ROM-Libraries
#include "romlib.h"

// Aliases for System Calls

// EEPROM
#define generic_ee_read                   EE_READ
#define generic_ee_write_ext              EE_WRITE_EXT
#define generic_ee_cmp                    EE_CMP
// Modulator
#define generic_mod_start                 MOD_START
#define generic_mod_stop                  MOD_STOP
// Transmit
#define generic_mod_send_man              MOD_SEND_MAN
#define generic_mod_send_man_const        MOD_SEND_MAN_CONST
#define generic_mod_send_bin              MOD_SEND_BIN
// Crypto
#define generic_crypto_load_ram           CRYPTO_LOAD_RAM
#define generic_crypto_load_eeprom        CRYPTO_LOAD_EEPROM
#define generic_crypto_load_const         CRYPTO_LOAD_CONST
#define generic_crypto_get                CRYPTO_GET
// ADC
#define generic_adc_cmp                   ADC_CMP

#endif