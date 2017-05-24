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
| HT3 calculation related functions.                                          |
|                                                                             |
+----------------------------------------------------------------------------*/


// includes
#include "defs.h"
//#include "types.h"

#include "hitag3.h"
#include "data_dispatch.h"
#include "romlib_parser.h"

//******************************************************************************

void hitag3_sr_calc(int8_t __idata * pi8_rsi, int8_t __idata * pi8_SeqResp)
{
  int8_t i8arr_challenge[CHAL_BYTES];     // Challenge
  int8_t i8_SKPage;                       // EEPROM page to extract secret key from
  int8_t i8arr_udf[UDF_BYTES];            // User Defined Field
  int8_t i8_FuncByte;                     // FUNC byte
  int8_t i8_RcbCfg0Byte;                  // RCB-Config byte 0
  uint8_t u8_ButtonCode;                  // Button Code (BID)
  
  //**********************************************************
  // get necessary data from data dispatcher
  //**********************************************************
  
  dat_get_udf (i8arr_udf);                          // get User Defined Field
    
  i8_FuncByte = dat_get_func_byte ();               // get FUNC byte
  
  i8_RcbCfg0Byte = dat_get_rcb_cfg0_byte();         // get RCB Config byte
  
  u8_ButtonCode = dat_get_button_code();            // get button code (BID) 
  
  //*********************************************
  if (i8_RcbCfg0Byte & RSKM_MASK)
  {
    i8_SKPage = IDE_PAGE;                           // use Immobilizer Secret Key [ISK] (use start page 0 in EEPROM, offset to SK is set in ROM-Lib call)
  }
  else
  {        
    i8_SKPage = dat_get_rke_cfg_page();             // use Remote Secret Key [RSK]
  }
  
  //**********************************************************
  // Mix Up1:
  //**********************************************************
  // load 64 bit SK bit [63-0] to crypto unit, use crypto function code Load0-enhanced = [00000011b], no offset
  generic_crypto_load_eeprom (i8_SKPage+2,0,64,3);
  
  // load 32 bit SK bit [95-64] to crypto unit, use crypto function code OWF-enhanced  = [00000111b], no offset
  generic_crypto_load_eeprom (i8_SKPage+1,0,32,7);

  // assemble challenge, MSB is in challenge[0]
  i8arr_challenge[0] = pi8_rsi[0];        // MSB challenge, MSB RSI
  i8arr_challenge[1] = pi8_rsi[1];
  i8arr_challenge[2] = pi8_rsi[2];
  i8arr_challenge[3] = pi8_rsi[3];

  i8arr_challenge[4] = i8arr_udf[0];        // MSB of UDF
  i8arr_challenge[5] = i8arr_udf[1];

  i8arr_challenge[6] = i8_FuncByte;

  i8arr_challenge[7] = u8_ButtonCode;       // LSB challenge
  
  // load 64 bit Challenge bit [63-0] to crypto unit, use crypto function code OWF-enhanced  = [00000111b]
  generic_crypto_load_ram (i8arr_challenge,64,7);
  
  // load 96 bit SK bit [95-0] to crypto unit, use crypto function code OWF-enhanced  = [00000111b], no offset
  generic_crypto_load_eeprom (i8_SKPage+1,0,96,7);
  
  // load 32 bit constant "1" to crypto unit, use crypto function code OWF-enhanced  = [00000111b]
  generic_crypto_load_const (1,32,7);  
  
  // load 32 bit IDE bit [31-0] to crypto unit, use crypto function code OWF-enhanced  = [00000111b], no offset
  generic_crypto_load_eeprom (IDE_PAGE,0,32,7);

  //**********************************************************
  // get 16 bit MAC [15-0] from crypto unit, use crypto function code LF-enhanced = [00000101b], no XOR with data   
  generic_crypto_get (0,pi8_SeqResp,16,5);    // remark: this data is not used, only extracted
  
  //**********************************************************
  // Mix Up2:
  //**********************************************************

  // load 96 bit SK bit [95-32] to crypto unit, use crypto function code OWF-enhanced  = [00000111b], no offset  
  generic_crypto_load_eeprom (i8_SKPage+1,0,64,7);
    
  // load 64 bit constant "0" to crypto unit, use crypto function code OWF-enhanced  = [00000111b]
  generic_crypto_load_const (0,64,7);

  //**********************************************************
  // get 48 bit SR [47-0] from crypto unit, use crypto function code LF-enhanced = [00000101b], no XOR with data
  generic_crypto_get (0,pi8_SeqResp,48,5);
  
}
//******************************************************************************
