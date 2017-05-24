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

+-------------------------------------------------------------------------------------+
|       Project: Generic RKE Application                                              |
|                                                                                     |
+-------------------------------------------------------------------------------------+
| Date      | File Version | Firmware Version | Comment                               |
+-------------------------------------------------------------------------------------+
| 25-Feb 11 | 1.00         | 1.00             | Initial Release                       |
+-------------------------------------------------------------------------------------+
| 19-Mar 12 | 1.00         | 1.00             | Change included file for IAR1.31      |
+-------------------------------------------------------------------------------------+
| 30-Mar 12 | 1.10         | 1.00             | Add frame end function definition     |
+-------------------------------------------------------------------------------------+
| 24-Apr 12 | 1.20         | 1.00             | Moved preamble calculation to         |
|           |              |                  | prepare_preamble() to allow           |
|           |              |                  | correct timing also for debug build   |
|           |              |                  | (without optimizations)               |
|           |              |                  |                                       |
+-------------------------------------------------------------------------------------+
| Description                                                                         |
|                                                                                     |
| Lowlevel Transmit Functions.                                                        |
|                                                                                     |
+-------------------------------------------------------------------------------------*/

// hardware registers
#include "io.h"

// function and data includes
#include "defs.h"
#include "transmit.h"
#include "data_dispatch.h"
#include "romlib_parser.h"
#include "ee_prom.h"

//******************************************************************************
// Module variables
int8_t    __idata  m_i8arr_LocPre[PRE_BYTES];        // assembled (local) preamble
uint8_t   __idata  m_u8_NumOfBitsPreamble;

//******************************************************************************

// local function prototypes
static void prepare_preamble( void );
static void transmit_preamble (void);

//******************************************************************************

void transmit_wakeup (void)
{
  uint8_t   i;
  int8_t    i8_WupCount;        // Wake-Up pattern counter
  int8_t    i8_RkeCfgPage;      // start page of RKE configuration
  //*****************************************
  // get data from data dispatcher
  i8_RkeCfgPage = dat_get_rke_cfg_page ();

  ee_read_data (&i8_WupCount,i8_RkeCfgPage+WUPC_OFFSET,0,8);           // page 5:   read bit [31-24], Wake Up Pattern count

  // use WUP counter from EEPROM to transmit wake-up pattern
  for (i=0; i<i8_WupCount; i++)
  {
    // transmit a number of manchester coded zero bits (= wake-up pattern)
    generic_mod_send_man_const (0, WAKE_UP_BITS);
    WDRESET;
  }
}

//******************************************************************************
static void prepare_preamble( void )
{
  int8_t    i8arr_preamble[PRE_BYTES];      // Preamble from EEPROM
  uint8_t   i,k;
  int8_t    i8_RkeCfgPage;                  // start page of RKE configuration

  // get data from data dispatcher
  i8_RkeCfgPage = dat_get_rke_cfg_page ();

  ee_read_data (i8arr_preamble,i8_RkeCfgPage+PRE_OFFSET,0,24);   // page 6:   read bit [31-8], 3 byte preamble

  // evaluate preamble, search for first byte unequal zero
  i=0;
  k=0;
  while (i < PRE_BYTES)                             // search max. number of possible preamble bytes (defined in defs.h)
  {
    if ((i8arr_preamble[i] == 0) && (k==0))         // is current byte equal zero and no byte unequal zero found before?
    {
      i++;                                          // ignore current byte, check next byte
    }
    else
    {
      m_i8arr_LocPre[k++] = i8arr_preamble[i++];     // store all bytes of preamble to local buffer beginning with the first byte unequal zero
    }
  }
  m_u8_NumOfBitsPreamble = k * 8;                    // get the number of preamble bits in buffer, used for transmission ROM-Lib
}

//******************************************************************************
static void transmit_preamble (void)
{
  // transmit preamble
  generic_mod_send_man( 0, m_i8arr_LocPre, m_u8_NumOfBitsPreamble );      // transmit a number of manchester coded bits, no encryption selected
}

//******************************************************************************
void transmit_frame ( int8_t __idata * pi8_TransmitBuf, uint8_t u8_NumOfBits )
{
  uint8_t   i;
  int8_t    i8_RcbCount;        // RC-Block counter
  int8_t    i8_RkeCfgPage;      // start page of RKE configuration
  //****************************************
  // get data from data dispatcher
  i8_RkeCfgPage = dat_get_rke_cfg_page ();

  ee_read_data (&i8_RcbCount,i8_RkeCfgPage+RCB_OFFSET,8,8);           // page 5:   read bit [23-16], RCB count

  // read preamble settings from EEPROM and determine preamble pattern
  prepare_preamble();
  
  //****************************************
  // transmit Rolling Code Block (incl. RUN-IN = Preamble) depending on RCB count from EEPROM
  for (i=0; i<i8_RcbCount; i++)
  {
    // transmit preamble
    transmit_preamble ();

    // transmit frame
    generic_mod_send_man (0, pi8_TransmitBuf, u8_NumOfBits);      // transmit a number of manchester coded bits, no encryption selected
    WDRESET;
  }
}

void transmit_frame_end(void)
{
  int8_t  i8_FinalSymbol = 0x00;    
  generic_mod_send_bin (&i8_FinalSymbol,2);   // transmit 2 uncoded zero (Space) to ensure last frame data bit is "out"
}
//******************************************************************************
