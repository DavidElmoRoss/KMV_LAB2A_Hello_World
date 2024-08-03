/**************************************************************************//**
 * @file     qspi.h
 * @version  V0.10
 * @brief    M251 series QSPI driver header file
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2019 Nuvoton Technology Corp. All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. Neither the name of Nuvoton Technology Corp. nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/
#ifndef __QSPI_H__
#define __QSPI_H__

#ifdef __cplusplus
extern "C"
{
#endif


/** @addtogroup Standard_Driver Standard Driver
  @{
*/

/** @addtogroup QSPI_Driver QSPI Driver
  @{
*/

/** @addtogroup QSPI_EXPORTED_CONSTANTS QSPI Exported Constants
  @{
*/

#define QSPI_MODE_0        (QSPI_CTL_TXNEG_Msk)                             /*!< CLKPOL=0; RXNEG=0; TXNEG=1 \hideinitializer */
#define QSPI_MODE_1        (QSPI_CTL_RXNEG_Msk)                             /*!< CLKPOL=0; RXNEG=1; TXNEG=0 \hideinitializer */
#define QSPI_MODE_2        (QSPI_CTL_CLKPOL_Msk | QSPI_CTL_RXNEG_Msk)       /*!< CLKPOL=1; RXNEG=1; TXNEG=0 \hideinitializer */
#define QSPI_MODE_3        (QSPI_CTL_CLKPOL_Msk | QSPI_CTL_TXNEG_Msk)       /*!< CLKPOL=1; RXNEG=0; TXNEG=1 \hideinitializer */

#define QSPI_SLAVE         (QSPI_CTL_SLAVE_Msk)                             /*!< Set as slave \hideinitializer */
#define QSPI_MASTER        (0x0UL)                                           /*!< Set as master \hideinitializer */

#define QSPI_SS                (QSPI_SSCTL_SS_Msk)                          /*!< Set SS \hideinitializer */
#define QSPI_SS_ACTIVE_HIGH    (QSPI_SSCTL_SSACTPOL_Msk)                    /*!< SS active high \hideinitializer */
#define QSPI_SS_ACTIVE_LOW     (0x0UL)                                       /*!< SS active low \hideinitializer */

/* QSPI Interrupt Mask */
#define QSPI_UNIT_INT_MASK                (0x001UL)                          /*!< Unit transfer interrupt mask \hideinitializer */
#define QSPI_SSACT_INT_MASK               (0x002UL)                          /*!< Slave selection signal active interrupt mask \hideinitializer */
#define QSPI_SSINACT_INT_MASK             (0x004UL)                          /*!< Slave selection signal inactive interrupt mask \hideinitializer */
#define QSPI_SLVUR_INT_MASK               (0x008UL)                          /*!< Slave under run interrupt mask \hideinitializer */
#define QSPI_SLVBE_INT_MASK               (0x010UL)                          /*!< Slave bit count error interrupt mask \hideinitializer */
#define QSPI_SLVTO_INT_MASK               (0x020UL)                          /*!< Slave Mode Time-out interrupt mask \hideinitializer */
#define QSPI_TXUF_INT_MASK                (0x040UL)                          /*!< Slave TX underflow interrupt mask \hideinitializer */
#define QSPI_FIFO_TXTH_INT_MASK           (0x080UL)                          /*!< FIFO TX threshold interrupt mask \hideinitializer */
#define QSPI_FIFO_RXTH_INT_MASK           (0x100UL)                          /*!< FIFO RX threshold interrupt mask \hideinitializer */
#define QSPI_FIFO_RXOV_INT_MASK           (0x200UL)                          /*!< FIFO RX overrun interrupt mask \hideinitializer */
#define QSPI_FIFO_RXTO_INT_MASK           (0x400UL)                          /*!< FIFO RX time-out interrupt mask \hideinitializer */

/* QSPI Status Mask */
#define QSPI_BUSY_MASK                    (0x01UL)                           /*!< Busy status mask \hideinitializer */
#define QSPI_RX_EMPTY_MASK                (0x02UL)                           /*!< RX empty status mask \hideinitializer */
#define QSPI_RX_FULL_MASK                 (0x04UL)                           /*!< RX full status mask \hideinitializer */
#define QSPI_TX_EMPTY_MASK                (0x08UL)                           /*!< TX empty status mask \hideinitializer */
#define QSPI_TX_FULL_MASK                 (0x10UL)                           /*!< TX full status mask \hideinitializer */
#define QSPI_TXRX_RESET_MASK              (0x20UL)                           /*!< TX or RX reset status mask \hideinitializer */
#define QSPI_QSPIEN_STS_MASK              (0x40UL)                           /*!< QSPIEN status mask \hideinitializer */
#define QSPI_SSLINE_STS_MASK              (0x80UL)                           /*!< QSPIx_SS line status mask \hideinitializer */

/*@}*/ /* end of group QSPI_EXPORTED_CONSTANTS */


/** @addtogroup QSPI_EXPORTED_FUNCTIONS QSPI Exported Functions
  @{
*/

/**
  * @brief      Clear the unit transfer interrupt flag.
  * @param[in]  qspi The pointer of the specified QSPI module.
  * @return     None.
  * @details    Write 1 to UNITIF bit of QSPI_STATUS register to clear the unit transfer interrupt flag.
  * \hideinitializer
  */
#define QSPI_CLR_UNIT_TRANS_INT_FLAG(qspi)   ((qspi)->STATUS = QSPI_STATUS_UNITIF_Msk)

/**
  * @brief      Trigger RX PDMA function.
  * @param[in]  qspi The pointer of the specified QSPI module.
  * @return     None.
  * @details    Set RXPDMAEN bit of QSPI_PDMACTL register to enable RX PDMA transfer function.
  * \hideinitializer
  */
#define QSPI_TRIGGER_RX_PDMA(qspi)   ((qspi)->PDMACTL |= QSPI_PDMACTL_RXPDMAEN_Msk)

/**
  * @brief      Trigger TX PDMA function.
  * @param[in]  qspi The pointer of the specified QSPI module.
  * @return     None.
  * @details    Set TXPDMAEN bit of QSPI_PDMACTL register to enable TX PDMA transfer function.
  * \hideinitializer
  */
#define QSPI_TRIGGER_TX_PDMA(qspi)   ((qspi)->PDMACTL |= QSPI_PDMACTL_TXPDMAEN_Msk)

/**
  * @brief      Disable RX PDMA transfer.
  * @param[in]  qspi The pointer of the specified QSPI module.
  * @return     None.
  * @details    Clear RXPDMAEN bit of QSPI_PDMACTL register to disable RX PDMA transfer function.
  * \hideinitializer
  */
#define QSPI_DISABLE_RX_PDMA(qspi) ( (qspi)->PDMACTL &= ~QSPI_PDMACTL_RXPDMAEN_Msk )

/**
  * @brief      Disable TX PDMA transfer.
  * @param[in]  qspi The pointer of the specified QSPI module.
  * @return     None.
  * @details    Clear TXPDMAEN bit of QSPI_PDMACTL register to disable TX PDMA transfer function.
  * \hideinitializer
  */
#define QSPI_DISABLE_TX_PDMA(qspi) ( (qspi)->PDMACTL &= ~QSPI_PDMACTL_TXPDMAEN_Msk )

/**
  * @brief      Get the count of available data in RX FIFO.
  * @param[in]  qspi The pointer of the specified QSPI module.
  * @return     The count of available data in RX FIFO.
  * @details    Read RXCNT (QSPI_STATUS[27:24]) to get the count of available data in RX FIFO.
  * \hideinitializer
  */
#define QSPI_GET_RX_FIFO_COUNT(qspi)   (((qspi)->STATUS & QSPI_STATUS_RXCNT_Msk) >> QSPI_STATUS_RXCNT_Pos)

/**
  * @brief      Get the RX FIFO empty flag.
  * @param[in]  qspi The pointer of the specified QSPI module.
  * @retval     0 RX FIFO is not empty.
  * @retval     1 RX FIFO is empty.
  * @details    Read RXEMPTY bit of QSPI_STATUS register to get the RX FIFO empty flag.
  * \hideinitializer
  */
#define QSPI_GET_RX_FIFO_EMPTY_FLAG(qspi)   (((qspi)->STATUS & QSPI_STATUS_RXEMPTY_Msk)>>QSPI_STATUS_RXEMPTY_Pos)

/**
  * @brief      Get the TX FIFO empty flag.
  * @param[in]  qspi The pointer of the specified QSPI module.
  * @retval     0 TX FIFO is not empty.
  * @retval     1 TX FIFO is empty.
  * @details    Read TXEMPTY bit of QSPI_STATUS register to get the TX FIFO empty flag.
  * \hideinitializer
  */
#define QSPI_GET_TX_FIFO_EMPTY_FLAG(qspi)   (((qspi)->STATUS & QSPI_STATUS_TXEMPTY_Msk)>>QSPI_STATUS_TXEMPTY_Pos)

/**
  * @brief      Get the TX FIFO full flag.
  * @param[in]  qspi The pointer of the specified QSPI module.
  * @retval     0 TX FIFO is not full.
  * @retval     1 TX FIFO is full.
  * @details    Read TXFULL bit of QSPI_STATUS register to get the TX FIFO full flag.
  * \hideinitializer
  */
#define QSPI_GET_TX_FIFO_FULL_FLAG(qspi)   (((qspi)->STATUS & QSPI_STATUS_TXFULL_Msk)>>QSPI_STATUS_TXFULL_Pos)

/**
  * @brief      Get the datum read from RX register.
  * @param[in]  qspi The pointer of the specified QSPI module.
  * @return     Data in RX register.
  * @details    Read QSPI_RX register to get the received datum.
  * \hideinitializer
  */
#define QSPI_READ_RX(qspi)   ((qspi)->RX)

/**
  * @brief      Write datum to TX register.
  * @param[in]  qspi The pointer of the specified QSPI module.
  * @param[in]  u32TxData The datum which user attempt to transfer through QSPI bus.
  * @return     None.
  * @details    Write u32TxData to QSPI_TX register.
  * \hideinitializer
  */
#define QSPI_WRITE_TX(qspi, u32TxData)   ((qspi)->TX = (u32TxData))

/**
  * @brief      Set QSPIx_SS pin to high state.
  * @param[in]  qspi The pointer of the specified QSPI module.
  * @return     None.
  * @details    Disable automatic slave selection function and set QSPIx_SS pin to high state.
  * \hideinitializer
  */
#define QSPI_SET_SS_HIGH(qspi)   ((qspi)->SSCTL = ((qspi)->SSCTL & (~QSPI_SSCTL_AUTOSS_Msk)) | (QSPI_SSCTL_SSACTPOL_Msk | QSPI_SSCTL_SS_Msk))

/**
  * @brief      Set QSPIx_SS pin to low state.
  * @param[in]  qspi The pointer of the specified QSPI module.
  * @return     None.
  * @details    Disable automatic slave selection function and set QSPIx_SS pin to low state.
  * \hideinitializer
  */
#define QSPI_SET_SS_LOW(qspi)   ((qspi)->SSCTL = ((qspi)->SSCTL & (~(QSPI_SSCTL_AUTOSS_Msk | QSPI_SSCTL_SSACTPOL_Msk))) | QSPI_SSCTL_SS_Msk)

/**
  * @brief      Enable Byte Reorder function.
  * @param[in]  qspi The pointer of the specified QSPI module.
  * @return     None.
  * @details    Enable Byte Reorder function. The suspend interval depends on the setting of SUSPITV (QSPI_CTL[7:4]).
  * \hideinitializer
  */
#define QSPI_ENABLE_BYTE_REORDER(qspi)   ((qspi)->CTL |=  QSPI_CTL_REORDER_Msk)

/**
  * @brief      Disable Byte Reorder function.
  * @param[in]  qspi The pointer of the specified QSPI module.
  * @return     None.
  * @details    Clear REORDER bit field of QSPI_CTL register to disable Byte Reorder function.
  * \hideinitializer
  */
#define QSPI_DISABLE_BYTE_REORDER(qspi)   ((qspi)->CTL &= ~QSPI_CTL_REORDER_Msk)

/**
  * @brief      Set the length of suspend interval.
  * @param[in]  qspi The pointer of the specified QSPI module.
  * @param[in]  u32SuspCycle Decides the length of suspend interval. It could be 0 ~ 15.
  * @return     None.
  * @details    Set the length of suspend interval according to u32SuspCycle.
  *             The length of suspend interval is ((u32SuspCycle + 0.5) * the length of one QSPI bus clock cycle).
  * \hideinitializer
  */
#define QSPI_SET_SUSPEND_CYCLE(qspi, u32SuspCycle)   ((qspi)->CTL = ((qspi)->CTL & ~QSPI_CTL_SUSPITV_Msk) | ((u32SuspCycle) << QSPI_CTL_SUSPITV_Pos))

/**
  * @brief      Set the QSPI transfer sequence with LSB first.
  * @param[in]  qspi The pointer of the specified QSPI module.
  * @return     None.
  * @details    Set LSB bit of QSPI_CTL register to set the QSPI transfer sequence with LSB first.
  * \hideinitializer
  */
#define QSPI_SET_LSB_FIRST(qspi)   ((qspi)->CTL |= QSPI_CTL_LSB_Msk)

/**
  * @brief      Set the QSPI transfer sequence with MSB first.
  * @param[in]  qspi The pointer of the specified QSPI module.
  * @return     None.
  * @details    Clear LSB bit of QSPI_CTL register to set the QSPI transfer sequence with MSB first.
  * \hideinitializer
  */
#define QSPI_SET_MSB_FIRST(qspi)   ((qspi)->CTL &= ~QSPI_CTL_LSB_Msk)

/**
  * @brief      Set the data width of a QSPI transaction.
  * @param[in]  qspi The pointer of the specified QSPI module.
  * @param[in]  u32Width The bit width of one transaction.
  * @return     None.
  * @details    The data width can be 8 ~ 32 bits.
  * \hideinitializer
  */
#define QSPI_SET_DATA_WIDTH(qspi, u32Width)   ((qspi)->CTL = ((qspi)->CTL & ~QSPI_CTL_DWIDTH_Msk) | (((u32Width)&0x1F) << QSPI_CTL_DWIDTH_Pos))

/**
  * @brief      Get the QSPI busy state.
  * @param[in]  qspi The pointer of the specified QSPI module.
  * @retval     0 QSPI controller is not busy.
  * @retval     1 QSPI controller is busy.
  * @details    This macro will return the busy state of QSPI controller.
  * \hideinitializer
  */
#define QSPI_IS_BUSY(qspi)   ( ((qspi)->STATUS & QSPI_STATUS_BUSY_Msk)>>QSPI_STATUS_BUSY_Pos )

/**
  * @brief      Enable QSPI controller.
  * @param[in]  qspi The pointer of the specified QSPI module.
  * @return     None.
  * @details    Set QSPIEN (QSPI_CTL[0]) to enable QSPI controller.
  * \hideinitializer
  */
#define QSPI_ENABLE(qspi)   ((qspi)->CTL |= QSPI_CTL_QSPIEN_Msk)

/**
  * @brief      Disable QSPI controller.
  * @param[in]  qspi The pointer of the specified QSPI module.
  * @return     None.
  * @details    Clear QSPIEN (QSPI_CTL[0]) to disable QSPI controller.
  * \hideinitializer
  */
#define QSPI_DISABLE(qspi)   ((qspi)->CTL &= ~QSPI_CTL_QSPIEN_Msk)

/**
  * @brief      Disable 2-bit Transfer mode.
  * @param[in]  qspi The pointer of the specified QSPI module.
  * @return     None.
  * @details    Clear TWOBIT bit of QSPI_CTL register to disable 2-bit Transfer mode.
  */
#define QSPI_DISABLE_2BIT_MODE(qspi)   ( (qspi)->CTL &= ~QSPI_CTL_TWOBIT_Msk )

/**
  * @brief      Enable 2-bit Transfer mode.
  * @param[in]  qspi The pointer of the specified QSPI module.
  * @return     None.
  * @details    Set TWOBIT bit of QSPI_CTL register to enable 2-bit Transfer mode.
  */
#define QSPI_ENABLE_2BIT_MODE(qspi)   ( (qspi)->CTL |= QSPI_CTL_TWOBIT_Msk )

/**
  * @brief      Disable Slave 3-wire mode.
  * @param[in]  qspi The pointer of the specified QSPI module.
  * @return     None.
  * @details    Clear SLV3WIRE bit of QSPI_SSCTL register to disable Slave 3-wire mode.
  */
#define QSPI_DISABLE_3WIRE_MODE(qspi)   ( (qspi)->SSCTL &= ~QSPI_SSCTL_SLV3WIRE_Msk )

/**
  * @brief      Enable Slave 3-wire mode.
  * @param[in]  qspi The pointer of the specified QSPI module.
  * @return     None.
  * @details    Set SLV3WIRE bit of QSPI_SSCTL register to enable Slave 3-wire mode.
  */
#define QSPI_ENABLE_3WIRE_MODE(qspi)   ( (qspi)->SSCTL |= QSPI_SSCTL_SLV3WIRE_Msk )

/**
  * @brief  Disable QSPI Dual IO function.
  * @param[in]  qspi is the base address of QSPI module.
  * @return none
  * \hideinitializer
  */
#define QSPI_DISABLE_DUAL_MODE(qspi) ( (qspi)->CTL &= ~QSPI_CTL_DUALIOEN_Msk )

/**
  * @brief  Enable Dual IO function and set QSPI Dual IO direction to input.
  * @param[in]  qspi is the base address of QSPI module.
  * @return none
  * \hideinitializer
  */
#define QSPI_ENABLE_DUAL_INPUT_MODE(qspi) ( (qspi)->CTL = ((qspi)->CTL & ~QSPI_CTL_DATDIR_Msk) | QSPI_CTL_DUALIOEN_Msk )

/**
  * @brief  Enable Dual IO function and set QSPI Dual IO direction to output.
  * @param[in]  qspi is the base address of QSPI module.
  * @return none
  * \hideinitializer
  */
#define QSPI_ENABLE_DUAL_OUTPUT_MODE(qspi) ( (qspi)->CTL |= QSPI_CTL_DATDIR_Msk | QSPI_CTL_DUALIOEN_Msk )

/**
  * @brief  Disable QSPI Dual IO function.
  * @param[in]  qspi is the base address of QSPI module.
  * @return none
  * \hideinitializer
  */
#define QSPI_DISABLE_QUAD_MODE(qspi) ( (qspi)->CTL &= ~QSPI_CTL_QUADIOEN_Msk )

/**
  * @brief  Set QSPI Quad IO direction to input.
  * @param[in]  qspi is the base address of QSPI module.
  * @return none
  * \hideinitializer
  */
#define QSPI_ENABLE_QUAD_INPUT_MODE(qspi) ( (qspi)->CTL = ((qspi)->CTL & ~QSPI_CTL_DATDIR_Msk) | QSPI_CTL_QUADIOEN_Msk )

/**
  * @brief  Set QSPI Quad IO direction to output.
  * @param[in]  qspi is the base address of QSPI module.
  * @return none
  * \hideinitializer
  */
#define QSPI_ENABLE_QUAD_OUTPUT_MODE(qspi) ( (qspi)->CTL |= QSPI_CTL_DATDIR_Msk | QSPI_CTL_QUADIOEN_Msk )




/* Function prototype declaration */
uint32_t QSPI_Open(QSPI_T *qspi, uint32_t u32MasterSlave, uint32_t u32QSPIMode, uint32_t u32DataWidth, uint32_t u32BusClock);
void QSPI_Close(QSPI_T *qspi);
void QSPI_ClearRxFIFO(QSPI_T *qspi);
void QSPI_ClearTxFIFO(QSPI_T *qspi);
void QSPI_DisableAutoSS(QSPI_T *qspi);
void QSPI_EnableAutoSS(QSPI_T *qspi, uint32_t u32SSPinMask, uint32_t u32ActiveLevel);
uint32_t QSPI_SetBusClock(QSPI_T *qspi, uint32_t u32BusClock);
void QSPI_SetFIFO(QSPI_T *qspi, uint32_t u32TxThreshold, uint32_t u32RxThreshold);
uint32_t QSPI_GetBusClock(QSPI_T *qspi);
void QSPI_EnableInt(QSPI_T *qspi, uint32_t u32Mask);
void QSPI_DisableInt(QSPI_T *qspi, uint32_t u32Mask);
uint32_t QSPI_GetIntFlag(QSPI_T *qspi, uint32_t u32Mask);
void QSPI_ClearIntFlag(QSPI_T *qspi, uint32_t u32Mask);
uint32_t QSPI_GetStatus(QSPI_T *qspi, uint32_t u32Mask);


/*@}*/ /* end of group QSPI_EXPORTED_FUNCTIONS */

/*@}*/ /* end of group QSPI_Driver */

/*@}*/ /* end of group Standard_Driver */

#ifdef __cplusplus
}
#endif

#endif /* __QSPI_H__ */

/*** (C) COPYRIGHT 2018 Nuvoton Technology Corp. ***/
