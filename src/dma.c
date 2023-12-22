/*******************************************************************************
* File Name:   dma.c

* Description: Provides initialization code for DMA.
*
* Related Document: See README.md
*
*
********************************************************************************
* Copyright 2022-2023, Cypress Semiconductor Corporation (an Infineon company)
* or an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
*
* This software, including source code, documentation and related
* materials ("Software") is owned by Cypress Semiconductor Corporation
* or one of its affiliates ("Cypress") and is protected by and subject to
* worldwide patent protection (United States and foreign),
* United States copyright laws and international treaty provisions.
* Therefore, you may use this Software only as provided in the license
* agreement accompanying the software package from which you
* obtained this Software ("EULA").
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software
* source code solely for use in connection with Cypress's
* integrated circuit products.  Any reproduction, modification, translation,
* compilation, or representation of this Software except as specified
* above is prohibited without the express written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer
* of such system or application assumes all risk of such use and in doing
* so agrees to indemnify Cypress against all liability.
*******************************************************************************/


/*******************************************************************************
* Header Files
*******************************************************************************/
#include "cyhal.h"
#include "spi_slave.h"
#include "dma.h"


/*******************************************************************************
* Macros
*******************************************************************************/


/*******************************************************************************
* Global Variables
*******************************************************************************/
volatile bool rx_dma_done;

/* Buffer to save the received data by the master */
uint8_t  dma_buffer[NUMBER_OF_ELEMENTS];

/*Initialization configuration structure for interrupt channel*/
const cy_stc_sysint_t intRxDma_cfg =
{
    .intrSrc      = rxDma_IRQ,
    .intrPriority = RXDMA_INTERRUPT_PRIORITY
};

/*******************************************************************************
* Function Prototypes
*******************************************************************************/
void rx_dma_complete_isr(void);


/*******************************************************************************
* Function Definitions
*******************************************************************************/

/*******************************************************************************
* Function Name: rx_dma_configure
********************************************************************************
* Summary:
*  This function configure the receive DMA block
*
* Parameters:
*  rx_buffer : Receive data buffer
*
* Return:
*  (uint32_t) INIT_SUCCESS or INIT_FAILURE
*
*******************************************************************************/
uint32_t rx_dma_configure()
 {
     cy_en_dma_status_t dma_init_status;

     /* Initialize descriptor */
     dma_init_status = Cy_DMA_Descriptor_Init(&rxDma_Descriptor_0,
                                                  &rxDma_Descriptor_0_config);
     if (dma_init_status!=CY_DMA_SUCCESS)
     {
         return INIT_FAILURE;
     }
     /* Initialize DMA channel */
     dma_init_status = Cy_DMA_Channel_Init(rxDma_HW, rxDma_CHANNEL,
                                              &rxDma_channelConfig);
     if (dma_init_status!=CY_DMA_SUCCESS)
     {
         return INIT_FAILURE;
     }

     /* Set source and destination for descriptor 1 */
     Cy_DMA_Descriptor_SetSrcAddress(&rxDma_Descriptor_0,
                                     (void *)&sSPI_HW->RX_FIFO_RD);
     Cy_DMA_Descriptor_SetDstAddress(&rxDma_Descriptor_0,
                                      (uint8_t *)dma_buffer);

      /* Initialize and enable the interrupt from TxDma */
     if(CY_SYSINT_SUCCESS != Cy_SysInt_Init(&intRxDma_cfg, &rx_dma_complete_isr))
     {
         return INIT_FAILURE;
     }

     NVIC_ClearPendingIRQ(intRxDma_cfg.intrSrc);
     NVIC_EnableIRQ((IRQn_Type)intRxDma_cfg.intrSrc);

      /* Enable DMA interrupt source. */
     Cy_DMA_Channel_SetInterruptMask(rxDma_HW, rxDma_CHANNEL, CY_DMA_INTR_MASK);
     /* Enable channel and DMA block to start descriptor execution process */
     Cy_DMA_Channel_Enable(rxDma_HW, rxDma_CHANNEL);
     Cy_DMA_Enable(rxDma_HW);
     return INIT_SUCCESS;
 }


/*******************************************************************************
* Function Name: rx_dma_complete_isr
********************************************************************************
* Summary:
*   Receive complete callback .
*
* Parameters:
*  None
*
*******************************************************************************/
void rx_dma_complete_isr(void)
 {

    /* Clear the interrupt */
    NVIC_ClearPendingIRQ(intRxDma_cfg.intrSrc);
    Cy_DMA_Channel_ClearInterrupt(rxDma_HW, rxDma_CHANNEL);

    rx_dma_done = true;

 }


/*******************************************************************************
* Function Name: handle_error
********************************************************************************
* Summary:
* User defined error handling function
*
* Parameters:
*    None
* Return:
*  void
*
*******************************************************************************/
void handle_error(void)
{
     /* Disable all interrupts. */
    __disable_irq();

    /* Infinite loop. */
    while(1u) {}

}

