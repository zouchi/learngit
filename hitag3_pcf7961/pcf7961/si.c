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
| Functions to support the RSI handling                                       |
|                                                                             |
+----------------------------------------------------------------------------*/

// hardware registers
#include "io.h"

// function and data includes
#include "defs.h"

#include "si.h"
#include "data_dispatch.h"
#include "romlib_parser.h"

// module global variable
uint32_t __idata m_u32arr_RSIField[RSI_PAGES];          // Sequence Increment pages

//******************************************************************************
uint8_t si_rsi_init (void)
{
  uint32_t  u32_MaxRSI = 0x00000000;

  uint32_t  u32_Diff1, u32_Diff2;

  uint8_t   u8_Invalid;

  uint8_t   i;

  uint8_t   u8_Page;
  //*****************************************
  // get data from data dispatcher
  u8_Page = (uint8_t)dat_get_si_page();
  //*****************************************
  // Read all RSI pages from EEPROM
  for (i=0; i<8; i++)
  {
    generic_ee_read ((int8_t *)&m_u32arr_RSIField[i], u8_Page + i, 0, 32);      // read entire page
  }
  //*****************************************
  /* Find Invalid RSI */
  for(i=0; i<6; i++)
  {
    if (m_u32arr_RSIField[i] > m_u32arr_RSIField[i+1])
    {
      u32_Diff1 = m_u32arr_RSIField[i] - m_u32arr_RSIField[i+1];
    }
    else
    {
      u32_Diff1 = m_u32arr_RSIField[i+1] - m_u32arr_RSIField[i];
    }
    if (m_u32arr_RSIField[i] > m_u32arr_RSIField[i+2])
    {
      u32_Diff2 = m_u32arr_RSIField[i] - m_u32arr_RSIField[i+2];
    }
    else
    {
      u32_Diff2 = m_u32arr_RSIField[i+2] - m_u32arr_RSIField[i];
    }
    if (u32_Diff1 > 1)
    {
      if (u32_Diff2 > 1)
      {
        u8_Invalid = 1;
      }
      else
      {
        u8_Invalid = 2;
      }
    }
    else if (u32_Diff2 > 1)
    {
      u8_Invalid = 3;
    }
    else
    {
      u8_Invalid = 0;
    }

    if (u8_Invalid != 0)
    {
      // calc absolute number of invalid RSI
      u8_Invalid += (i-1);

      // Find the largest value
      for (i=0; i<u8_Invalid; i++)
      {
        if (m_u32arr_RSIField[i] > u32_MaxRSI)
        {
          u32_MaxRSI = m_u32arr_RSIField[i];
        }
      }
      for (i=u8_Invalid+1; i<8; i++)
      {
        if (m_u32arr_RSIField[i] > u32_MaxRSI)
        {
          u32_MaxRSI = m_u32arr_RSIField[i];
        }
      }
      //*****************************************
      // Store the largest value into corrupted rsi page.
      m_u32arr_RSIField[u8_Invalid] = u32_MaxRSI;

      generic_ee_write_ext ((int8_t*) &m_u32arr_RSIField[u8_Invalid], (u8_Page + u8_Invalid), 0, 32);

      // ensure, EEPROM programming is really finished
      while (EECON_bit.BUSY);
      //*****************************************
      // ensure only valid data written to EEPROM, otherwise return error code [0=success, 1=error]
      return (generic_ee_cmp ((int8_t*) &m_u32arr_RSIField[u8_Invalid], (u8_Page + u8_Invalid), 0, 32));
      // Only one invalid entry possible, quit searching.
      //break;
    }
  }
  // return success (no repair necessary)
  return 0;
}

//******************************************************************************
uint8_t si_rsi_increment (void)
{
  uint32_t * pu32_NewValue;
  uint32_t   u32_MinValue = 0xFFFFFFFF;

  uint8_t    u8_Smallest = 0;
  uint8_t    i;
  uint8_t    u8_Page;                      // EEPROM start page for RSI

  //*****************************************
  // get data from dispatcher
  u8_Page = (uint8_t)dat_get_si_page();
  //*****************************************
  // find smallest value
  for (i=0; i<8; i++)
  {
    if (m_u32arr_RSIField[i] < u32_MinValue)
    {
      u32_MinValue = m_u32arr_RSIField[i];
      u8_Smallest = i;
    }
  }
  //*****************************************
  // increment smallest value
  pu32_NewValue = &m_u32arr_RSIField[u8_Smallest];
  *pu32_NewValue +=  1;

  //*****************************************
  // write data to EEPROM
  generic_ee_write_ext ((int8_t*) &m_u32arr_RSIField[u8_Smallest], (u8_Page + u8_Smallest), 0, 32);
  // ensure, EEPROM programming is really finished
  while (EECON_bit.BUSY);
  // ensure only valid data written to EEPROM, otherwise return error code [0=success, 1=error]
  return (generic_ee_cmp ((int8_t*) &m_u32arr_RSIField[u8_Smallest], (u8_Page + u8_Smallest), 0, 32));
}
//******************************************************************************
void si_rsi_get (int8_t __idata * pi8_rsi)
{
  uint8_t   i;

  uint32_t  u32_Sum = 0x00000000;
  uint8_t * pu8_RSI;
  //*****************************************
  // sum all page entries
  for (i=0; i<8; i++)
  {
    u32_Sum += m_u32arr_RSIField[i];
  }
  //*****************************************
  // create byte data from long value
  pu8_RSI = (uint8_t*)(&u32_Sum);
  // observe correct byte order (MSB first)
  pi8_rsi[0] = pu8_RSI[3];
  pi8_rsi[1] = pu8_RSI[2];
  pi8_rsi[2] = pu8_RSI[1];
  pi8_rsi[3] = pu8_RSI[0];
  //*****************************************

}
//******************************************************************************