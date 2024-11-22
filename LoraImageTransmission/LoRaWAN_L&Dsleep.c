#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "mxc_sys.h"
#include "mxc_delay.h"
#include "rtc.h"
#include "uart.h"
#include "board.h"
#include "wdt.h"
#include "spi.h"
#include "pb.h"
#include "mxc_config.h"
#include "led.h"
#include "mxc_pins.h"
#include "spi.h"
#include "tmr.h"
#include "tmr_utils.h"
#include "nvic_table.h"
#include "lp.h"
#include "icc.h"
#include "gpio.h"
/* **** Definitions **** */
#define UART_BAUD           115200//1000000
#define BUFF_SIZE           1500//512

#define TEST_LEN           14000//(11000/2)

//#define TEST_LEN_BYTE      12000//120//(11000) //works for max 11KB
#define CLOCK_RATE         1000000 // Bit Rate 1MHz
#define bufferSize         TEST_LEN * 2 + 1

#define TEST_SLAVE SPI1A
#define TEST_SLAVE_IRQ SPI1_IRQn


#define DELAY_IN_SEC    20
#define SLEEP_DURATION_SEC  9 // Sleep duration in seconds

/* **** Globals **** */

volatile int read_flag;
volatile int write_flag;

volatile int alarmed;


volatile uint8_t cycle_count __attribute__((section(".retained_ram")));

//volatile uint8_t cycle_count __attribute__((section(".data")));

uint8_t slave_rx_data[TEST_LEN];
uint8_t slave_tx_data[TEST_LEN];
volatile int spi_slave_flag;
gpio_cfg_t toggle = {PORT_0, PIN_9, GPIO_FUNC_OUT, GPIO_PAD_NONE};

/* **** Functions **** */

/*void incrementCycleCount(void) {
    cycle_count++;  // Increment cycle count
    printf("Cycle count incremented to: %u\n", cycle_count);
}
*/
void checkCycleCount(void) {
    uint32_t reset_cause = MXC_GCR->rstr0;

    printf("Reset cause: 0x%08X\n", reset_cause);

    if (reset_cause == 0) {
        printf("Power-On Reset detected. Initializing cycle count.\n");
        cycle_count = 1; // Initialize cycle count
    } else if (reset_cause & MXC_F_GCR_RSTR0_WDT) {
        printf("Watchdog Reset detected. Resuming operation.\n");
    } else if (reset_cause & MXC_F_GCR_RSTR0_SRST) {
        printf("Software Reset detected. Resuming operation.\n");
    } else if (reset_cause & MXC_F_GCR_RSTR0_SYSTEM) {
        printf("System Reset detected. Resuming operation.\n");
    } else {
        printf("Unknown reset cause. Defaulting cycle count.\n");
        cycle_count = 1;
    }
}

void setTrigger(int waitForTrigger)
{
    alarmed = 0;
    sys_cfg_rtc_t sys_cfg;
    sys_cfg.tmr = MXC_TMR0;
    while(RTC_Init(MXC_RTC, 0, 0, &sys_cfg) == E_BUSY);
    while(RTC_SetTimeofdayAlarm(MXC_RTC, DELAY_IN_SEC) == E_BUSY);
    while(RTC_EnableRTCE(MXC_RTC) == E_BUSY);
    if(waitForTrigger)
    {
        while(!alarmed);
    }

    while(UART_PrepForSleep(MXC_UART_GET_UART(CONSOLE_UART)) != E_NO_ERROR);


}


void alarmHandler(void)
{
    int flags = RTC_GetFlags();
    alarmed = 1;

    if (flags & MXC_F_RTC_CTRL_ALSF) {
        RTC_ClearFlags(MXC_F_RTC_CTRL_ALSF);
    }

    if (flags & MXC_F_RTC_CTRL_ALDF) {
        RTC_ClearFlags(MXC_F_RTC_CTRL_ALDF);
    }
}


/* Configure RTC and enter light sleep */
void enterLightSleep(void) {
    alarmed = 0;
    sys_cfg_rtc_t sys_cfg;
    sys_cfg.tmr = MXC_TMR0;

    printf("Initializing RTC...\n");
    if (RTC_Init(MXC_RTC, 0, 0, &sys_cfg) != E_NO_ERROR) {
        printf("RTC initialization failed.\n");
        return;
    }

    printf("Setting RTC alarm for %d seconds...\n", SLEEP_DURATION_SEC);
    if (RTC_SetTimeofdayAlarm(MXC_RTC, SLEEP_DURATION_SEC) != E_NO_ERROR) {
        printf("Failed to set RTC alarm.\n");
        return;
    }

    // Enable RTC and the RTC Alarm wake-up source
    if (RTC_EnableRTCE(MXC_RTC) != E_NO_ERROR) {
        printf("Failed to enable RTC.\n");
        return;
    }

    LP_EnableRTCAlarmWakeup();
    NVIC_EnableIRQ(RTC_IRQn);  // Ensure RTC IRQ is enabled for wake-up

    printf("Entering light sleep mode...\n");
    while(!alarmed) {
        LP_EnterSleepMode();  // Enter light sleep
    }
    printf("Woke up from light sleep!\n");
}


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
    //printf("Write Callback called with error: %d\n", error);
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


	 NVIC_SetVector(RTC_IRQn, alarmHandler);
	 LP_EnableRTCAlarmWakeup();


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

	free(command);
}

void customWait(float s){
	//mxc_delay_start(MXC_DELAY_SEC(s));
	TMR_Delay(MXC_TMR0,MSEC(s*1000),NULL);
	//for(int i = 0; i< 960000*s;i++){}
	//printf("done waiting\n");
}


int main(void)
{

	/* if (cycle_count == 0) {
	        cycle_count = 1;  // Start at 1 for the first cycle
	        printf("First cycle initialized.\n");
	    }
         */

   checkCycleCount(); 
    const sys_cfg_uart_t sys_uart_cfg = {
        MAP_A,
        UART_FLOW_DISABLE,
    };

    spi_req_t slave_req;  // Initialize SPI Request struct for Slave

    int error;

    // Turn on the LED in active mode
        //LED_On(0); // Assuming LED index 0, change if necessary
      //  setTrigger(1);

        // Turn off the LED before entering low-power modes
    LED_Off(1);

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

    NVIC_SetVector(RTC_IRQn, alarmHandler);

    LP_EnableRTCAlarmWakeup();


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


    gpio_cfg_t gpio_out;

    /* Setup output pin. */
    gpio_out.port = PORT_0;//GPIO_PORT_OUT;
    gpio_out.mask = PIN_6;//GPIO_PIN_OUT;
    gpio_out.pad = GPIO_PAD_NONE;
    gpio_out.func = GPIO_FUNC_OUT;
    GPIO_Config(&gpio_out);

    GPIO_OutClr(&gpio_out);

    while(spi_slave_flag == 1) {

    }


	Console_Init();

//	printf("SPI Reception Complete.\n");
	//printf("Slave data: %s \n", slave_rx_data);
//	printf("Length of Slave data: %d \n", sizeof(slave_rx_data)/sizeof(slave_rx_data[0]));

	// Print the data
	GPIO_OutSet(&gpio_out);
	gpio_out.mask = PIN_8;//GPIO_PIN_OUT;
	GPIO_Config(&gpio_out);

	GPIO_OutClr(&gpio_out);


	char hex_string[TEST_LEN * 2 + 1]; // 2 characters per byte + null terminator

	// Convert the received data to a hex string
	hex_to_string(slave_rx_data, TEST_LEN, hex_string);

	// Print the hex string
	printf("Slave data in hex: %s\n", hex_string);

//	printf("Length of Slave data: %d \n", TEST_LEN);



    /* Initialize the UART */
    uart_cfg_t cfg;
    cfg.parity = UART_PARITY_DISABLE;
    cfg.size = UART_DATA_SIZE_8_BITS;
    cfg.stop = UART_STOP_1;
    cfg.flow = UART_FLOW_CTRL_EN;
    cfg.pol = UART_FLOW_POL_EN;
    cfg.baud = UART_BAUD;

//    printf("Enabling UART0A.\n");

    error = UART_Init(MXC_UART_GET_UART(0), &cfg, &sys_uart_cfg);

    if (error != E_NO_ERROR) {
        printf("Error initializing UART %d\n", error);
        while(1) {}
    }
//    //LORA
//
//    // 42 = roundup(TEST_LEN/240)
//    //240  = 	packet size
   int dignum=450;
   char txdata[dignum+10+2];
   char picnumdata[13];

   // Format the command string dynamically
   int length = snprintf(picnumdata, sizeof(picnumdata), "AT+SEND=3:%d\r\n", cycle_count);
   printf(picnumdata);
   // Check if snprintf succeeded and the buffer size was sufficient
   /*if (length < 0 || length >= sizeof(picnumdata)) {
       printf("Error: Command string formatting failed or buffer size is insufficient.\n");
       return -1;
   } */

   // Pass the constructed command to sendCommand
   sendCommand(length, picnumdata);
   customWait(10);

	
   //int loop= (TEST_LEN/240)+1;
   int loop=((TEST_LEN*2)/450+1);
   //int loop= ((TEST_LEN*2)/240)+1;
   //sendCommand(22,"AT+PARAMETER=7,8,1,4\r\n");
        	//customWait(3);
   for (int i = 0; i < loop; i++) {
    	    // Initialize the AT command at the beginning of txdata
    	    //const char* at_command = "AT+SEND=1,240,";
	        const char* at_command = "AT+SEND=9:";
    	    strncpy(txdata, at_command, 10); // Copy the AT command into txdata

    	    for(int j =0;j<dignum;j++){
    		if (i*dignum+j == TEST_LEN){
    			break;
    		}
    		//txdata[j+14] = slave_rx_data[i*240+j];
    		txdata[j+10] = hex_string[i*dignum+j];
    	}
    	txdata[dignum+10] = '\r';
    	txdata[dignum+11] = '\n';

    	//customWait(10);
    	sendCommand(dignum+10+2,txdata);
    	printf("Entering light sleep for 10 seconds...\n");
    	enterLightSleep();
    	printf("Woke up from light sleep!\n");


    }


    printf("Finish Sending Data");

	GPIO_OutSet(&gpio_out);

	// Check the reset cause
	checkCycleCount();


	    // Increment and display cycle count
	cycle_count++;
	printf("Cycle count incremented to: %u\n", cycle_count);

	MXC_GCR->scon |= 0x4000;  // Disable SWD

	// Increment the cycle count at the end of the task
	//incrementCycleCount();

	printf("Entering DEEPSLEEP mode.\n");
	setTrigger(0);
	LP_DisableBandGap();
	LP_DisableVCorePORSignal();
	LP_EnableRamRetReg();
	LP_DisableBlockDetect();
	LP_EnableFastWk();
	LP_ClearWakeStatus();
	LP_EnterDeepSleepMode();

	printf("Entering BACKUP mode.\n");
	setTrigger(0);
	LP_ClearWakeStatus();
	LP_EnterBackupMode();

	MXC_GCR->scon &= 0xBFFF;  // Enable SWD


    printf("Complete example");
    while (1) {}
}
