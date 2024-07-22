/*******************************************************************************
 * Copyright (C) 2016 Maxim Integrated Products, Inc., All Rights Reserved.
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
 *******************************************************************************
 */

/**
 * @file    main.c
 * @brief   Main for UART example.
 * @details This example loops back the TX to the RX on UART0. For this example
 *          you must connect a jumper across P0.4 to P0.5. UART_BAUD and the BUFF_SIZE
 *          can be changed in this example.
 */
/* **** Includes **** */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "mxc_sys.h"
#include "mxc_delay.h"
#include "uart.h"
#include "board.h"
#include "wdt.h"
#include "spi.h"
#include "pb.h"
#include "mxc_config.h"
#include "led.h"
#include "mxc_pins.h"
#include "spi.h"

/* **** Definitions **** */
#define UART_BAUD           115200//1000000
#define BUFF_SIZE           1500//512

#define TEST_LEN           12000//(11000/2)
//#define TEST_LEN_BYTE      12000//120//(11000) //works for max 11KB
#define CLOCK_RATE         100000 // Bit Rate
#define bufferSize         TEST_LEN * 2 + 1

#define TEST_SLAVE SPI1A
#define TEST_SLAVE_IRQ SPI1_IRQn

/* **** Globals **** */
volatile int read_flag;
volatile int write_flag;

uint8_t slave_rx_data[TEST_LEN];
uint8_t slave_tx_data[TEST_LEN];
volatile int spi_slave_flag;
gpio_cfg_t toggle = {PORT_0, PIN_9, GPIO_FUNC_OUT, GPIO_PAD_NONE};

/* **** Functions **** */

void spi_slave_cb(void *req, int error)
{
    spi_slave_flag = error;
}

void SPI1_IRQHandler(void)
{
    SPI_Handler(SPI1A);

}

void hex_to_string(uint8_t *data, size_t length, char *output) {
    for (size_t i = 0; i < length; ++i) {
        sprintf(output + i * 2, "%02X", data[i]); // Convert byte to hex string
    }
}

/* **************************************************************************** */
void read_cb(uart_req_t* req, int error)
{
    read_flag = error;
    printf("Read Callback called with error: %d\n", error);
}

/* **************************************************************************** */
void write_cb(uart_req_t* req, int error)
{
    write_flag = error;
    printf("Write Callback called with error: %d\n", error);
}

/* **************************************************************************** */
void UART0_IRQHandler(void)
{
    UART_Handler(MXC_UART0);
}

/* **************************************************************************** */

void sendCommand(int length,const char* cmd){

	uint8_t* command = (uint8_t*)malloc(length * sizeof(uint8_t));
	for (int i = 0; i < length; i++) {
		command[i] = (uint8_t)cmd[i];
	}
//	uint8_t* response = (uint8_t*)malloc(128 * sizeof(uint8_t));
	int error;
	uart_req_t write_req;
	write_req.data = command;
	write_req.len = length;
	write_req.callback = write_cb;

//	uart_req_t read_req;
//	read_req.data = response;
//	read_req.len = 128;
//	read_req.callback = read_cb;


	printf("%s\n",command);
	write_flag = 1;
	read_flag = 1;
	error = UART_WriteAsync(MXC_UART_GET_UART(0), &write_req);
	printf("ERROR:%d\n",error);
	if (error != E_NO_ERROR) {
		printf("Error starting async write %d\n", error);
		while(1) {}
	}
	else {
		printf("Write Transaction has been set up.\n");
	}

//	error = UART_ReadAsync(MXC_UART_GET_UART(0), &read_req);
//	if(error != 0){
//		printf("Error Reading\n");
//		while(1){}
//	}
//	else{
//		printf("Read Set up\n");
//	}

	/* Wait for the write to complete. */
	while (write_flag == 1) {}
	if (write_flag != 0) {
		printf("Error with UART_WriteAsync callback\n");
	} else {
		printf("Write has been completed.\n");
	}
//	while(read_flag == 1){
//		if (count == 9600000){
//			printf("IM STUCK/n");
//			break;
//		}
//		count++;
//
//
//	}
//	if (read_flag != E_NO_ERROR) {
//		printf("Error with reading\n");
//	}
//	else{
//		printf("Response: %s\n",response);
//	}
	free(command);
//	free(response);
}

void customWait(int s){
	//mxc_delay_start(MXC_DELAY_SEC(s));

	for(int i = 0; i< 9600000*s;i++){}
	printf("done waiting\n");
}


int main(void)
{
    const sys_cfg_uart_t sys_uart_cfg = {
        MAP_A,
        UART_FLOW_DISABLE,
    };

    spi_req_t slave_req;  // Initialize SPI Request struct for Slave

    int error;

//    uint8_t txdata[1460];
    //uint8_t txdata[256] = "AT+SEND=0,240,";

    // image is 3412 bytes
    //char image[3412] = "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";


    printf("\n\n***** UART Example *****\n");
    printf("\nConnect UART0A TX (P0.4) to UART0A RX (P0.5) for this example.\n\n");
    printf(" System freq \t: %d Hz\n", SystemCoreClock);
    printf(" UART freq \t: %d Hz\n", UART_BAUD);
    printf(" Loop back \t: %d bytes\n\n", BUFF_SIZE);


    /* Setup the interrupt */
    NVIC_ClearPendingIRQ(MXC_UART_GET_IRQ(0));
    NVIC_DisableIRQ(MXC_UART_GET_IRQ(0));
    NVIC_SetPriority(MXC_UART_GET_IRQ(0), 1);
    NVIC_EnableIRQ(MXC_UART_GET_IRQ(0));

    NVIC_EnableIRQ(TEST_SLAVE_IRQ);

    while (UART_Busy(MXC_UART_GET_UART(CONSOLE_UART)));
    Console_Shutdown();

    if(SPI_Init(TEST_SLAVE , 0, CLOCK_RATE ) != E_NO_ERROR) {
    	//  Console_Init();
		printf("Error configuring SPI\n");
		while(1) {}
	}

    memset(slave_rx_data,  0x0, TEST_LEN*2);
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
	slave_req.rx_num = 0;//TEST_LEN;//0;
	slave_req.callback = spi_slave_cb;

   // spi_master_flag =1;
	spi_slave_flag =1;

    SPI_SlaveTransAsync(TEST_SLAVE, &slave_req);
  //  SPI_MasterTrans(TEST_MASTER, &master_req);

    while(spi_slave_flag == 1) {

    }

	Console_Init();

	printf("SPI Reception Complete.\n");
	//printf("Slave data: %s \n", slave_rx_data);
	printf("Length of Slave data: %d \n", sizeof(slave_rx_data)/sizeof(slave_rx_data[0]));

	// Print the data

	char hex_string[TEST_LEN * 2 + 1]; // 2 characters per byte + null terminator

	// Convert the received data to a hex string
	hex_to_string(slave_rx_data, TEST_LEN, hex_string);

	// Print the hex string
	printf("Slave data in hex: %s\n", hex_string);

	printf("Length of Slave data: %d \n", TEST_LEN);



    /* Initialize the UART */
    uart_cfg_t cfg;
    cfg.parity = UART_PARITY_DISABLE;
    cfg.size = UART_DATA_SIZE_8_BITS;
    cfg.stop = UART_STOP_1;
    cfg.flow = UART_FLOW_CTRL_EN;
    cfg.pol = UART_FLOW_POL_EN;
    cfg.baud = UART_BAUD;

    printf("Enabling UART0A.\n");

    error = UART_Init(MXC_UART_GET_UART(0), &cfg, &sys_uart_cfg);

    if (error != E_NO_ERROR) {
        printf("Error initializing UART %d\n", error);
        while(1) {}
    }
//    //LORA
//
//    // 42 = roundup(TEST_LEN/240)
//    //240  = 	packet size
//    for(int i = 0; i < 42;i++){
//    	for(int j =0;j<240;j++){
//    		if (i*240+j == TEST_LEN){
//    			break;
//    		}
//    		txdata[j+14] = slave_rx_data[i*240+j];
//    	}
//    	txdata[254] = '\r';
//    	txdata[255] = '\n';
//
//    	sendCommand(256,txdata);
//    	wait(3);
//
//    }
//
    char txdata[1460];
    //NBIOT
	//NBIOT
	    sendCommand(13,"AT+CMNB=2\r\n");
	    //Shuts
	    sendCommand(12,"AT+CIPSHUT\r\n");
		customWait(2);
	    //PDP Attach
		sendCommand(12,"AT+CGATT=1\r\n");
		customWait(2);
		//Configure APN
		sendCommand(20,"AT+CSTT=\"hologram\"\r\n");
		customWait(2);
		//CCR Connect
		sendCommand(10,"AT+CIICR\r\n");
		customWait(2);
		//Initialize IP
		sendCommand(10,"AT+CIFSR\r\n");
		customWait(2);
		//Sending To Google Cloud Server(8.8.8.8) remember to change IP and length of command
		sendCommand(41,"AT+CIPSTART=\"UDP\",\"71.63.125.82\",\"8080\"\r\n");
		customWait(5);
		//Send image via UDP
		int chunks = bufferSize / 1460;
		//int remainingBytes = bufferSize % 1460;

		for (int chunk = 0; chunk < chunks; chunk++) {
		     sendCommand(17, "AT+CIPSEND=1460\r\n");

		     for (int i = 0; i < 1460; i++) {
		      txdata[i] = hex_string[chunk * 1460 + i];
		        }

		        sendCommand(1460, txdata);
		        customWait(15);
		    }
//		sendCommand(17,"AT+CIPSEND=1460\r\n");
//        for(int i =0;i<1460;i++){
//			txdata[i] = hex_string[i];
//		}
//		sendCommand(1460,txdata);


    printf("Complete example");
    while (1) {}
}
