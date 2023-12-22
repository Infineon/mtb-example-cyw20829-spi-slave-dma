/*******************************************************************************
* File Name:   main.c
*
* Description: This example demonstrates the use of the
*              SPI Serial Communication Block (SCB) resource for CYW920829
*              MCU in slave mode using DMA
*
* Related Document: See README.md
*
*
********************************************************************************
* Copyright 2021-2022, Cypress Semiconductor Corporation (an Infineon company) or
* an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
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
#include "cybsp.h"
#include "interface.h"
#include "spi_slave.h"
#include "dma.h"
#include "cy_pdl.h"
#include "cycfg.h"
#include "cybsp_types.h"
#include "cycfg_dmas.h"
#include "cy_retarget_io.h"
/*******************************************************************************
* Macros
*******************************************************************************/


/*******************************************************************************
* Global Variables
*******************************************************************************/


/*******************************************************************************
* Function Prototypes
*******************************************************************************/
static void led_update(uint8_t led_cmd);


/*******************************************************************************
* Function Definitions
*******************************************************************************/

/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
*    1. System entrance point, This function configures and initializes the SPI
*    2. SPI slave receives the data from master and LED is turned ON or OFF
*       based on the received command.
*
* Parameters:
*  void
*
* Return:
*  int
*
*******************************************************************************/
int main(void)
{

    uint32_t status = 0;
    cy_rslt_t result;

#if defined (CY_DEVICE_SECURE)
    cyhal_wdt_t wdt_obj;

    /* Clear watchdog timer so that it doesn't trigger a reset */
    result = cyhal_wdt_init(&wdt_obj, cyhal_wdt_get_max_timeout_ms());
    CY_ASSERT(CY_RSLT_SUCCESS == result);
    cyhal_wdt_free(&wdt_obj);
#endif

    /* Initialize the device and board peripherals */
    result = cybsp_init();

    /* Board init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Enable global interrupts */
    __enable_irq();

    /* Initialize the SPI Slave */
    status = slave_init();

    cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX,
                        CY_RETARGET_IO_BAUDRATE);

    /* Configure the receive dma */
    status = rx_dma_configure();

    if (INIT_FAILURE == status)
    {
        /* NOTE: This function will block the CPU forever */
        handle_error();
    }

    if (INIT_FAILURE == status)
    {
        /* NOTE: This function will block the CPU forever */
        handle_error();
    }

    printf("\x1b[2J\x1b[;H");
    printf("================================================\r\n");
    printf("================= DMA SPI SLAVE ================\r\n");
    printf("================================================\r\n");

    status = cyhal_gpio_init(CYBSP_USER_LED2, CYHAL_GPIO_DIR_OUTPUT,
                                  CYHAL_GPIO_DRIVE_STRONG, CYBSP_LED_STATE_OFF);

    if (INIT_FAILURE == status)
    {
        /* NOTE: This function will block the CPU forever */
        handle_error();
    }

    for (;;)
    {
        if(rx_dma_done)
         {
         /* Check start and end of packet markers */
            if ((dma_buffer[PACKET_SOP_POS] == PACKET_SOP) &&
                   (dma_buffer[PACKET_EOP_POS] == PACKET_EOP))
            {
                /* Communication succeeded. Update the LED. */
                led_update(dma_buffer[PACKET_CMD_POS]);
                printf("Data received from the SPI master and LED status is "
                        "%s\r\n", dma_buffer[PACKET_CMD_POS] ? "OFF" : "ON");
            }
             rx_dma_done = false;
             /* Get the bytes received by the slave */
         }
    }
}


/******************************************************************************
* Function Name: led_update
*******************************************************************************
*
* Summary:      This function updates the LED based on the command received by
*               the SPI Slave from Master.
*
* Parameters:   (uint8_t) led_cmd - command to turn LED ON or OFF
*
* Return:       None
*
******************************************************************************/
static void led_update(uint8_t led_cmd)
{
    /* Control the LED. Note that the LED on the supported kits is in active low
       connection. */
    if (CYBSP_LED_STATE_ON == led_cmd)
    {
        /* Turn ON the LED */
        cyhal_gpio_write(CYBSP_USER_LED2, CYBSP_LED_STATE_ON);
    }
    if (CYBSP_LED_STATE_OFF == led_cmd)
    {
        /* Turn OFF the LED */
        cyhal_gpio_write(CYBSP_USER_LED2, CYBSP_LED_STATE_OFF);
    }
}

/* [] END OF FILE */
