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
| EEPROM access related functions.                                              |
|                                                                             |
+----------------------------------------------------------------------------*/

// includes
#include "defs.h"
#include "ee_prom.h"
#include "data_dispatch.h"
#include "romlib_parser.h"

//******************************************************************************
uint8_t ee_read_rke_cfg (void)
{
  
  int8_t i8_RkeCfgPage;                                // RKE config page
  int8_t i8arr_udf[UDF_BYTES];                         // user defined field
  int8_t i8_RcbCfg0Byte;                               // RCB Config Byte 0, defines frame transmission settings etc.
  int8_t i8_SIPage;                                    // Sequence Increment start page in EEPROM  
  
  //*****************************************************
  // get block number of rke config pages
  generic_ee_read (&i8_RkeCfgPage,RKE_CFG_PAGE,24,8);           // read bit 7-0 of page (:4 Hitag 3 / 5: AES)
  
  // mask necessary lower 4 bits bit [3 - 0]
  i8_RkeCfgPage = i8_RkeCfgPage & 0x0F;                         // block number of RKE config pages

  if ((i8_RkeCfgPage != 0) && (i8_RkeCfgPage != 15))
  {
    i8_RkeCfgPage = i8_RkeCfgPage << 3;                         // start page of RKE config pages (8 pages each block)
    
    // write data to data dispatcher
    dat_set_rke_cfg_page (i8_RkeCfgPage);
  }
  else
  {
    return 1;
  } 
  //***************************************
  // get user defined field
  generic_ee_read (i8arr_udf,i8_RkeCfgPage+0,0,16);             // page 0:   read bit [31-16], 2 byte UDF
  
  // write data to data dispatcher
  dat_set_udf (i8arr_udf);  
  //***************************************
  generic_ee_read (&i8_RcbCfg0Byte,i8_RkeCfgPage+RCBCFG_OFFSET,16,8);       // page 5:   read bit [15-8], RCB-Config byte

  // write data to data dispatcher
  dat_set_rcb_cfg0_byte (i8_RcbCfg0Byte);
  //***************************************
  // get start page of Sequence Increment pages in EEPROM
  i8_SIPage = i8_RkeCfgPage + SI_PAGE_OFFSET;

  // write data to data dispatcher
  dat_set_si_page (i8_SIPage);
  //***************************************
    
  return 0;
}
//******************************************************************************

void ee_read_data (int8_t __idata * pi8_ReadArray, int8_t i8_Page, int8_t i8_Offset, uint8_t u8_NumOfBytes)
{
  generic_ee_read (pi8_ReadArray,i8_Page,i8_Offset,u8_NumOfBytes); 
}
//******************************************************************************
