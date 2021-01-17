/* NOTES
 * -add missing mandatory argument exception--Very important MUST HAVE*/





//-------------------| LL Libs |------------------------------------
#include <stm32f4xx_ll_bus.h>
#include <stm32f4xx_ll_gpio.h>
#include <stm32f4xx_ll_utils.h>
#include <stm32f4xx_ll_usart.h>
#include <stm32f4xx_ll_rcc.h>
#include <stdlib.h>
#include "string.h"
#include "stdbool.h"

//-------------------| CL Libs |------------------------------------
#include "CL_CONFIG.h"
#include "CL_delay.h"
#include "CL_systemClockUpdate.h"
#include "CL_printMsg.h"
//#include "CL_bfp.h"
#include "cli_python_host.h"
//-------------------| Defines |------------------------------------


//-------------------| Global variables |---------------------------
//char cli.cliMsg[MESSAGE_MAX]; //cli commands will be stored here

//uint8_t cli.msgPtr = 0;  //counter to make sure pointer doesnt exceed cli.cliMsg max
//bool cli.parsePending  = false; 
//-------------------| Prototypes |---------------------------------
void initLed(void);
void blink(uint8_t times, uint16_t delay);


//-------------------| cli stuff  |---------------------------------


CL_cli_type cli;
void uart_init_full_duplex(void);


void cmd_ok_handler(uint8_t num, char *values[]);
void cmd_add_handler(uint8_t num, char *values[]);
void cmd_sub_handler(uint8_t num, char *values[]);
void cmd_test_var_handler(uint8_t num, char *values[]);
void cmd_ledOn_handler(uint8_t num, char *values[]);
void cmd_ledOff_handler(uint8_t num, char *values[]);
void cmd_ledBlink_handler(uint8_t num, char *values[]);
void cmd_getreg_handler(uint8_t num, char *values[]);
//this array of function pointers will hold pointers to all the handler functions


//------------------------------------------------------------------

int main(void)
{
	//LL_InitTick(180000000, 1000);
	setClockTo180();
	CL_delay_init();
	//CL_printMsg_init_Default(false);
	initLed();
	uart_init_full_duplex();
	CL_printMsg("------Cli init-----\r\n");

	
	
	CL_cli_init(&cli);
	cli.prompt = "eddie>";
	cli.delimeter = '\n';



	cli.registerCommand("ok", ' ', cmd_ok_handler, "Prints \"ok\" if cli is ok");
	cli.registerCommand("add", '+', cmd_add_handler, "Add numbers with a + delimeter");
	cli.registerCommand("sub", '-', cmd_sub_handler, "Subtracts numbers with a - delimeter");
	cli.registerCommand("modx", ' ', cmd_test_var_handler, "Under construction");
	cli.registerCommand("ledOn", ' ', cmd_ledOn_handler, "Turns on user LED");
	cli.registerCommand("ledOff", ' ', cmd_ledOff_handler, "Turns of user LED");
	cli.registerCommand("ledBlink", ';', cmd_ledBlink_handler, "Blinks user led x times with y delay\r\nusing the following format : ledBlink x;y");
	cli.registerCommand("getReg", ';', cmd_getreg_handler, "Returns the value of a given register\n\r[Format] getreg GPIOA;hex or (bin)(dec)");
	

	while (1)
	{
	
		
		if (cli.parsePending == true)
		{
		
			parseCMD(&cli);
		//	CL_printMsg("%s", cli.prompt);
		}

       
	}
	
	

}//------------------------------------------------------------------
void initLed(void)
{	
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOG);
	LL_GPIO_InitTypeDef gpio;
	LL_GPIO_StructInit(&gpio);
	gpio.Mode = LL_GPIO_MODE_OUTPUT;
	gpio.Pin = LL_GPIO_PIN_14 | LL_GPIO_PIN_13;
	LL_GPIO_Init(GPIOG, &gpio);


}//------------------------------------------------------------------
void blink(uint8_t times, uint16_t delay)
{
	for (int i = 0; i < times; i++)
	{
		LL_GPIO_SetOutputPin(GPIOG, LL_GPIO_PIN_14);
		delayMS(delay);		
		LL_GPIO_ResetOutputPin(GPIOG, LL_GPIO_PIN_14);
		delayMS(delay);	
	
	}
	
	LL_GPIO_ResetOutputPin(GPIOG, LL_GPIO_PIN_14);
	
	
	
}//------------------------------------------------------------------
void uart_init_full_duplex(void)
{
	//enable GPIOA clock and configure  for USART1 : [PA-9 TX] [PA-10 RX]
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
	
	LL_GPIO_InitTypeDef usartGpio;
	LL_GPIO_StructInit(&usartGpio);
	usartGpio.Pin  = LL_GPIO_PIN_9 | LL_GPIO_PIN_10;
	usartGpio.Mode = LL_GPIO_MODE_ALTERNATE;
	usartGpio.Alternate = LL_GPIO_AF_7;
	LL_GPIO_Init(GPIOA, &usartGpio);
	
	
	//enable USART1 clock
	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1);
	
	//init usart peripheral
	LL_USART_InitTypeDef cli_uart_1;
	LL_USART_StructInit(&cli_uart_1);
	cli_uart_1.BaudRate = 115200;
	LL_USART_Init(USART1, &cli_uart_1);
	//LL_USART_ConfigAsyncMode(USART1);
	USART1->BRR = 0x30D;
	LL_USART_Enable(USART1);
	
	//enable interrupts
	LL_USART_EnableIT_RXNE(USART1);
	NVIC_EnableIRQ(USART1_IRQn);
	
		
}//-----------------------------------------------------------------
void USART1_IRQHandler(void)
{
	if ((USART1->SR & USART_SR_RXNE)) //if data has arrived on the uart
	{
		USART1->SR &= ~(USART_SR_RXNE);//clear interrupt
		
		//fetch data
		//python version does not need to relay the received char
		cli.charReceived = USART1->DR; 

		//if the character receieved is not the delimeter then echo the character
		//NOT NEEDED IN PYTHON VERSION


//		if(cli.charReceived != cli.delimeter)
//			USART1->DR = cli.charReceived; 
		parseChar(&cli);				
	}
	
	
}//----------------------------------------------------------------
void cmd_ok_handler(uint8_t num, char *values[])
{
//	******figure out how to handle help messages
//	if ( !(strcmp(values[0], "?")) )
//	{
//		CL_printMsg("\r\nThis is the help messg\r\n");
//	}
//	else
//	{
		CL_printMsg("System ok!");
	//}
}//--------------------------------------------------
void cmd_add_handler(uint8_t num, char *values[])
{
	/*
	 num will have the number of arguments passed
	 and values is an array of strings with those arguments
	 in the order they were passed
	 */
	uint32_t sum = 0;

	for (int i = 0; i < num; i++)
	{
		sum += atoi(values[i]);

	}
	CL_printMsg("Sum: %d\r\n", sum); 


   
}//--------------------------------------------------
void cmd_sub_handler(uint8_t num, char *values[])
{
    
	//printf("%d\n",num);
	if(num >= 2)
	{

        
		int initial = atoi(values[0]);
		int i = 0;
		for (i = 1; i < num; i++)
		{
			initial -= atoi(values[i]);
		}
		CL_printMsg("Difference = %d\r\n", initial);
	}
	else
	{

		CL_printMsg("Error! Two numbers needed to subtract\r\n");
	}
}//--------------------------------------------------
void cmd_test_var_handler(uint8_t num, char *values[])
{
	for (int i = 0; i < num; i++)
		CL_printMsg("%s\r", values[i]); 
        
	//TEST_VAR = atoi(values[0]);
	//CL_printMsg("Var : %d\n", TEST_VAR);


}
void cmd_ledOn_handler(uint8_t num, char *values[])
{
	LL_GPIO_SetOutputPin(GPIOG, LL_GPIO_PIN_14);
//	CL_printMsg("\r\n");

}
void cmd_ledOff_handler(uint8_t num, char *values[])
{
	LL_GPIO_ResetOutputPin(GPIOG, LL_GPIO_PIN_14);
	//CL_printMsg("\r\n");


}
void cmd_ledBlink_handler(uint8_t num, char *values[])
{
	blink(atoi(values[0]) , atoi(values[1]) );
	//CL_printMsg("\r\n");


}
void cmd_getreg_handler(uint8_t num, char *values[])// make this actually print registers 
{
	//getreg GPIOA:moder
	//CL_printMsg("%s : %d\r\n", values[0], values[1]); 
	printRegister(33);
}