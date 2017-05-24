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

+--------------------------------------------------------------------------------------+
|       Project: Generic RKE Application                                               |
|                                                                                      |
+--------------------------------------------------------------------------------------+
| Date      | File Version | Firmware Version | Comment                                |
+--------------------------------------------------------------------------------------+
+--------------------------------------------------------------------------------------+
| 25-Feb 11 | 1.00         | 1.00             | Initial Release                        |
| 30-Mar 12 | 1.10         | 1.00             | Add frame end after whole              |
|           |              |                  | frame sent out                         |
| 17-Apr 12 |              |                  | Update: corrected inversion            |
|           |              |                  | of VBATL flag in FUNC byte             |
|           |              |                  |                                        |
+--------------------------------------------------------------------------------------+
| Description                                                                          |
|                                                                                      |
| High level RKE related functions.                                                    |
|                                                                                      |
+-------------------------------------------------------------------------------------*/

// includes
#include "defs.h"
#include "types.h"

#include "rke.h"
#include "data_dispatch.h"
#include "transmit.h"

#include "si.h"
#include "ee_prom.h"

#include "crypt.h"

// global variables
int8_t    __idata m_i8arr_TXBuffer[TX_BUF_BYTES];       // transmit buffer ("__idata" used to prevent data overlay error!)

// local prototypes
void rke_assemble_func (void);                          // get FUNC byte out of RKE_CFG byte 0
// assemble transmission buffer depending on RKE_CFG byte 0
void rke_assemble_tx_buffer (int8_t __idata * pi8_SeqResp, int8_t __idata * pi8_rsi, uint8_t __idata * pu8_TXBits);                     

//******************************************************************************
void rke_run (void)
{  
  int8_t    i8arr_SeqResp[SEQ_BYTES];           // Sequence Response 
  uint8_t   u8_TXBits;                          // number of bits to transmit 
  int8_t    i8arr_rsi[RSI_BYTES];               // Sequence Increment counter
    
  //*****************************************************
  // Sequence Increment  
  u8_TXBits = si_rsi_init ();                   // read and repair RSI-pages, use u8_TXBits for return code first
  
  if (u8_TXBits == 0)                           // no EE read after write error, else do not transmit data!
  {
    u8_TXBits = si_rsi_increment ();            // increment current RSI, use u8_TXBits again like mentioned above

    if (u8_TXBits == 0)                         // no EE read after write error, else do not transmit data!
    {
      si_rsi_get(i8arr_rsi);                    // get new RSI (data is written to data-dispatcher)
      //*****************************************************   
  
      rke_assemble_func();                               // create FUNC byte based on RCB-Config
                
      crypt_sr_calc( i8arr_rsi, i8arr_SeqResp );         // get crypto Sequence Response
                 
      rke_assemble_tx_buffer( i8arr_SeqResp, i8arr_rsi, &u8_TXBits );       // assemble transmission buffer (u8_TXBits now used as bit counter)

      transmit_wakeup();                                // transmit a number of manchester coded zero bits as Wake Up pattern

      transmit_frame( m_i8arr_TXBuffer, u8_TXBits );    // transmit RC blocks (preamble included), select number of bits to transmit

      transmit_frame_end();
    }
  }
} 

//******************************************************************************
void rke_assemble_func (void)
{
  int8_t  i8_FuncByte;             // function byte, assembled here, delivered to extern
  int8_t  i8_RcbCfg0Byte;          // RCB Config byte
  int8_t  i8_ADCRes;               // ADC compare result
  
  //*****************************************
  // get data from data dispatcher
  i8_RcbCfg0Byte = dat_get_rcb_cfg0_byte();           // RCB Config byte
  
  i8_ADCRes = dat_get_adc_cmp_res();                  // get result of ADC compare 
  //*****************************************
  // assemble FUNC byte
  // RCB_CFG[0] = [DUDF | DIDE | RSKM | CRM1 | CRM0 | RSIM  | SRM1 | SRM0]
  //       FUNC = [DUDF | DIDE | RSIM | SRM1 | SRM0 | VBATL | FCT1 | FCT0]
  
  i8_FuncByte  = (i8_RcbCfg0Byte & 0xC0);             // DUDF | DIDE
  i8_FuncByte |= ((i8_RcbCfg0Byte & 0x07) << 3);      // RSIM | SRM1 | SRM0
  i8_FuncByte |= (((i8_ADCRes & 0x01)^0x01) << 2);    // VBATL = NOT(i8_ADCRes)
  //*****************************************
  // write result to data dispatcher
  dat_set_func_byte (i8_FuncByte);
}

//******************************************************************************
void rke_assemble_tx_buffer (int8_t __idata * pi8_SeqResp, int8_t __idata * pi8_rsi, uint8_t __idata * pu8_TXBits)
{
  uint8_t k;
  int8_t  i8arr_ide[IDE_BYTES];         // identifier (IDE)
  int8_t  i8arr_udf[UDF_BYTES];         // user defined field (UDF)
  int8_t  i8_RcbCfg0Byte;               // RCB-config byte 0
  uint8_t u8_ButtonCode;                // Button Code (BID)
  int8_t  i8_FuncByte;                  // FUNC byte
  //*****************************************************
  /* create current RC-Block depending on RCB-Config byte (used masks defined in defs.h)
         RCB  = [FUNC | UDF | IDE | BID | RSI | SR]
   RCB_CFG[0] = [DUDF | DIDE | RSKM | CRM1 | CRM0 | RSIM  | SRM1 | SRM0]
  
   DUDF_MASK  = 0x80            // 1000 0000b (disable UDF)
   DIDE_MASK  = 0x40            // 0100 0000b (disable IDE)
   RSIM_MASK  = 0x04            // 0000 0100b (number of RSI bytes)
   SRM_MASK   = 0x03            // 0000 0011b (number of SR bytes)
  */
  //*****************************************************
  // index of transmission buffer
  k=0;
  //**************************************
  // FUNC
  // get data from data dispatcher 
  i8_FuncByte = dat_get_func_byte ();
  
  m_i8arr_TXBuffer[k++] = i8_FuncByte;            // first byte in RCB is always FUNC
  //**************************************
  // DUDF
  // get data from data dispatcher
  i8_RcbCfg0Byte = dat_get_rcb_cfg0_byte ();      // RCB Config byte
  
  if ((i8_RcbCfg0Byte & DUDF_MASK) == 0)          // if DUDF = 0 => include UDF bytes
  {
    // get data from data dispatcher
    dat_get_udf (i8arr_udf);
    
    m_i8arr_TXBuffer[k++] = i8arr_udf[0];         // MSB
    m_i8arr_TXBuffer[k++] = i8arr_udf[1];
  }
    //**************************************
  // DIDE
  if ((i8_RcbCfg0Byte & DIDE_MASK) == 0)          // if DIDE = 0 => include IDE bytes
  {
    // get data from EEPROM
    ee_read_data (i8arr_ide, IDE_PAGE, 0, 32);
    
    m_i8arr_TXBuffer[k++] = i8arr_ide[0];         // MSB
    m_i8arr_TXBuffer[k++] = i8arr_ide[1];
    m_i8arr_TXBuffer[k++] = i8arr_ide[2];
    m_i8arr_TXBuffer[k++] = i8arr_ide[3];
  }
  //**************************************
  // BID
  // get data from dispatcher 
  u8_ButtonCode = dat_get_button_code ();
  
  m_i8arr_TXBuffer[k++] = u8_ButtonCode;          // BID is always part of the RCB
  //**************************************
  // RSIM   
  if ((i8_RcbCfg0Byte & RSIM_MASK) == 0)          // if RSIM = 0 => include 4 RSI bytes
  {       
    m_i8arr_TXBuffer[k++] = pi8_rsi[0];         // MSB
    m_i8arr_TXBuffer[k++] = pi8_rsi[1];
    m_i8arr_TXBuffer[k++] = pi8_rsi[2];
    m_i8arr_TXBuffer[k++] = pi8_rsi[3];
  }
  else                                            // if RSIM = 1 => include lower 2 RSI bytes
  {
    m_i8arr_TXBuffer[k++] = pi8_rsi[2];
    m_i8arr_TXBuffer[k++] = pi8_rsi[3];         // LSB
  }
  //**************************************
  // SRM
  if (((i8_RcbCfg0Byte & SRM_MASK) == 0) || ((i8_RcbCfg0Byte & SRM_MASK) == 3))  // if SRM = 0 or 3 => include 6 SR bytes
  {
    m_i8arr_TXBuffer[k++] = pi8_SeqResp[0];     // MSB
    m_i8arr_TXBuffer[k++] = pi8_SeqResp[1];
    m_i8arr_TXBuffer[k++] = pi8_SeqResp[2];
    m_i8arr_TXBuffer[k++] = pi8_SeqResp[3];
    m_i8arr_TXBuffer[k++] = pi8_SeqResp[4];
    m_i8arr_TXBuffer[k++] = pi8_SeqResp[5];
  }
  else if ((i8_RcbCfg0Byte & SRM_MASK) == 1)      // if SRM = 1 => include 5 MSB SR bytes
  {
    m_i8arr_TXBuffer[k++] = pi8_SeqResp[0];     // MSB
    m_i8arr_TXBuffer[k++] = pi8_SeqResp[1];
    m_i8arr_TXBuffer[k++] = pi8_SeqResp[2];
    m_i8arr_TXBuffer[k++] = pi8_SeqResp[3];
    m_i8arr_TXBuffer[k++] = pi8_SeqResp[4];    
  }
  else if ((i8_RcbCfg0Byte & SRM_MASK) == 2)      // if SRM = 2 => include 4 MSB SR bytes
  {
    m_i8arr_TXBuffer[k++] = pi8_SeqResp[0];     // MSB
    m_i8arr_TXBuffer[k++] = pi8_SeqResp[1];
    m_i8arr_TXBuffer[k++] = pi8_SeqResp[2];
    m_i8arr_TXBuffer[k++] = pi8_SeqResp[3];    
  }
  //**************************************
 
  *pu8_TXBits = k*8;                              // get number of assembled bits

}
//******************************************************************************
