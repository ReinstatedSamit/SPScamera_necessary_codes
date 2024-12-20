/**
 * @file    main.c
 * @brief   SPI Master Demo
 * @details Shows Master loopback demo for SPI0 (AKA: SPI17Y) and SPI1 (AKA: SPIMSS)
 *          Read the printf() for instructions
 */

/*******************************************************************************
 * Copyright (C) 2017 Maxim Integrated Products, Inc., All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL MAXIM INTEGRATED BE LIABLE FOR ANY CLAIM, DAMAGES
 * OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of Maxim Integrated
 * Products, Inc. shall not be used except as stated in the Maxim Integrated
 * Products, Inc. Branding Policy.
 *
 * The mere transfer of this software does not imply any licenses
 * of trade secrets, proprietary technology, copyrights, patents,
 * trademarks, maskwork rights, or any other form of intellectual
 * property whatsoever. Maxim Integrated Products, Inc. retains all
 * ownership rights.
 *
 * $Date: 2018-08-08 11:43:44 -0500 (Wed, 08 Aug 2018) $
 * $Revision: 36747 $
 *
 ******************************************************************************/

/***** Includes *****/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "pb.h"
#include "mxc_config.h"
#include "led.h"
#include "mxc_pins.h"
#include "mxc_delay.h"
#include "uart.h"
#include "spi.h"

/***** Definitions *****/
#define TEST_LEN           12000//(11000/2)
#define TEST_LEN_BYTE      12000//120//(11000) //works for max 11KB
#define CLOCK_RATE         10000 // Bit Rate

//#define TEST_MASTER SPI1A
//#define TEST_MASTER_IRQ SPI1_IRQn

#define TEST_SLAVE SPI0A
#define TEST_SLAVE_IRQ SPI0_IRQn

/***** Globals *****/

uint8_t slave_rx_data[TEST_LEN];
uint8_t slave_tx_data[TEST_LEN];
volatile int spi_slave_flag;
gpio_cfg_t toggle = {PORT_0, PIN_9, GPIO_FUNC_OUT, GPIO_PAD_NONE};

/***** Functions *****/

/*void spi_master_cb(void *req, int error)
{
    spi_master_flag = error;
}*/


void spi_slave_cb(void *req, int error)
{
    spi_slave_flag = error;
}
void SPI0_IRQHandler(void)
{
    SPI_Handler(SPI0A);
}

/*void SPI1_IRQHandler(void)
{
    SPI_Handler(SPI1A);

}*/

void hex_to_string(uint8_t *data, size_t length, char *output) {
    for (size_t i = 0; i < length; ++i) {
        sprintf(output + i * 2, "%02X", data[i]); // Convert byte to hex string
    }
}


/*void printHex(uint8_t *buffer, size_t length) {
    for (size_t i = 0; i < length; ++i) {
        printf("%02X", buffer[i]); // Print each byte in hex format with leading zero if needed
    }
    printf("\n"); // Add a newline after printing the buffer
}
*/
int main(void)
{

    
    spi_req_t slave_req;  // Initialize SPI Request struct for Slave


   // NVIC_EnableIRQ(TEST_MASTER_IRQ);
    NVIC_EnableIRQ(TEST_SLAVE_IRQ);

    printf("\n************** SPI Master and Slave Demo ****************\n");
  //  printf("This example configures the SPI to send data between the SPI0 and SPI1. Connect SCK (P0.6) to SCK (P0.12)\n");
    //printf("MOSI (P0.5) to MOSI (P0.11), MISO (P0.4) to MISO (P0.10) and SSEL (P0.7) to SSEL(P0.13)  Connect these two pins together.\n");
    printf("This demo shows SPI receiving different images tjrough SPI on each run through. \n");

    // delay before UART shutdown
    while (UART_Busy(MXC_UART_GET_UART(CONSOLE_UART)));
    Console_Shutdown();


    if(SPI_Init(TEST_SLAVE , 0, CLOCK_RATE ) != E_NO_ERROR) {
        Console_Init();
        printf("Error configuring SPI\n");
        while(1) {}
    }

    // Initialize RX buffer to store data
  
    memset(slave_rx_data,  0x0, TEST_LEN_BYTE);
    GPIO_Config(&toggle);


    // Initialize spi_request struct for slave
    slave_req.ssel = 0;
    slave_req.ssel_pol = SPI_POL_LOW;
    slave_req.tx_data = NULL;//slave_tx_data;
    slave_req.rx_data = slave_rx_data;
    slave_req.len = TEST_LEN;
    slave_req.width = SPI0_WIDTH_1;
    slave_req.bits = 8;
    slave_req.tx_num = 0;
    slave_req.rx_num = 0;
    slave_req.callback = spi_slave_cb;

   // spi_master_flag =1;
    spi_slave_flag =1;

    // Initiate the Slave Async first before Master Sync to make sure
    // data can be received and no data lost when master initiates communication


    SPI_SlaveTransAsync(TEST_SLAVE, &slave_req);
  //  SPI_MasterTrans(TEST_MASTER, &master_req);

    while(spi_slave_flag == 1) {
    }

    Console_Init();


     //printf("Slave data: %s \n", slave_rx_data);
     printf("Length of Slave data: %d \n", sizeof(slave_rx_data)/sizeof(slave_rx_data[0]));

     // Print the data

     char hex_string[TEST_LEN * 2 + 1]; // 2 characters per byte + null terminator

     // Convert the received data to a hex string
     hex_to_string(slave_rx_data, TEST_LEN, hex_string);

     // Print the hex string
     printf("Slave data in hex: %s\n", hex_string);

     printf("Length of Slave data: %d \n", TEST_LEN);

    // printHex(slave_rx_data, TEST_LEN);


    Console_Init();
    printf("\n Done testing \n");


    return 0;
}
