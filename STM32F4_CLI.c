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
#include "CL_bfp.h"

//-------------------| Defines |------------------------------------
char DELIMETER = '\r';
char DELIMETER1 = '\n';
char DELIMETER2 = 10;
#define MESSAGE_MAX 50

//-------------------| Global variables |---------------------------
char cliMsg[MESSAGE_MAX]; //cli commands will be stored here
char *cliMsgPtr = &cliMsg[0]; //used as a pointer to cli commands 
uint8_t ptrCount = 0;  //counter to make sure pointer doesnt exceed cliMsg max
bool parsingPending  = false; 
//-------------------| Prototypes |---------------------------------
void initLed(void);
void blink(uint8_t times, uint16_t delay);


//-------------------| cli stuff  |---------------------------------
#define NUM_OF_COMMANDS 2

typedef struct  {
	const char *command;
	int(*handler)(int argc, char *argv[]);
	const char *help;
} cliCMD_init;


void uart_init_full_duplex(void);
//void parseMsg(void);
//int cliCMD_ok_handler(int argc, char *argv[]);
//int cliCMD_stas_handler(int argc, char *argv[]);
//void registerCommnds(uint8_t num_commands, cliCMD_init *command_struct);
void cmd_exit_handler(uint8_t num, char *values[]);
void cmd_ok_handler(uint8_t num, char *values[]);
void cmd_add_handler(uint8_t num, char *values[]);
void cmd_sub_handler(uint8_t num, char *values[]);
void cmd_test_var_handler(uint8_t num, char *values[]);
void cmd_ledOn_handler(uint8_t num, char *values[]);
void cmd_ledOff_handler(uint8_t num, char *values[]);
void cmd_ledBlink_handler(uint8_t num, char *values[]);

//this array of function pointers will hold pointers to all the handler functions
int(*command_Handlers[NUM_OF_COMMANDS])(int argc, char *argv[]);


//------------------------------------------------------------------

int main(void)
{
	//LL_InitTick(180000000, 1000);
	setClockTo180();
	CL_delay_init();
	//CL_printMsg_init_Default(false);
	initLed();
	uart_init_full_duplex();
	CL_printMsg("hello world");
	registerCommand("exit", ' ', cmd_exit_handler);
	registerCommand("ok", ' ', cmd_ok_handler);
	registerCommand("add", '+', cmd_add_handler);
	registerCommand("sub", '-', cmd_sub_handler);
	registerCommand("modx", ' ', cmd_test_var_handler);
	registerCommand("ledOn", ' ', cmd_ledOn_handler);
	registerCommand("ledOff", ' ', cmd_ledOff_handler);
	registerCommand("ledBlink", ';', cmd_ledBlink_handler);
	
	
	
	
	
	
	
	
	
	//blink(3, 100);
	DWT->CYCCNT = 0;
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
	while (1)
	{
		
		if (parsingPending == true)
		{
		
			parseCMD(cliMsg);
			parsingPending = false;
			CL_printMsg("----------\r\neddie@home[%ds]> ", DWT->CYCCNT / 180000000);
		}

       
	}
	
	
	//an array of command structures to declar all supporting commands
//	cliCMD_init cmd[] = 
//	{
//		{ //this declares a command, handler function and help message
//			cmd->command = "ruok" ,
//			cmd->handler = cliCMD_ok_handler ,
//			cmd->help	   = "Prints ok if cli is ok\n" ,
//		},
//		
//		{
//			cmd->command = "stats",
//			cmd->handler = cliCMD_stas_handler,
//			cmd->help	   = "Prints stats of serial connection\n"
//		}
//	
//	};
//	registerCommnds(NUM_OF_COMMANDS, &cmd[0]);
//
//	for (;;)
//	{
//		if (parsingPending == true)
//		{
//			parseMsg();
//		} 
//		
//		
//		
//
//	}
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
		char received = USART1->DR;
		
		
		//*******optional**********************/
		//send the char back to look like a real terminal
		if(received != DELIMETER || received !=  DELIMETER1 ) 
			USART1->DR = received;
		
		
		/*	If parsingPending is already true then it means a message is still
		 *	parsing , this current data will be ginored */
		if(parsingPending == false) 
		{
			/*
				 *	if the delimeter has been reached: 
				 *		--stop assembling the message 
				 *		--reset the cli pointer to point back to 
				 *		  beggining of cliMsg 
				 *		--reset the pointer counter 
				 *		--set message received to true 
				 *	
				 */
			//if (*cliMsgPtr == DELIMETER ) 
			if (received == DELIMETER ) 
			{	
				
			
				
			
				//cliMsgPtr = &cliMsg[0] ;   //point the temp pointer  back to start
				ptrCount = 0;  //reset temp pointer countr
				
				//this flag is used to let the application know we have a command to parse
				//do not parse anything in ISR
				parsingPending = true; 
				
				
				
			}	
			
			/*	if we have NOT reached the delimiter then increment the pointer and pointer counter
			 *	so the next byte is stored at the next location hence assembling the message\
			 *	*/
			else 
			{
				cliMsg[ptrCount] = received; 
				
				
				if (ptrCount < MESSAGE_MAX)
				{
				 
					ptrCount++; //this keeps track of how much we have incremtned the cliMsg index
				}			
			}
		}
	}
	
	
}//----------------------------------------------------------------
//void parseMsg(void)
//{
//	
//	
////	//lets parse the message
////	char test[] = "ok";
////	
////	char *temp = strtok(cliMsg, (const char)(DELIMETER));
////			
////	//CL_printMsg("\n temp = %s : test = %s\n", temp , test);
////	if(strcmp(*temp, *test) == 0)
////	{
////		CL_printMsg("\n::>System Ok!");
////	}
//		
//		
//	
//			
//	parsingPending = false; 
//}//----------------------------------------------------------------
void cmd_exit_handler(uint8_t num, char *values[])
{
	CL_printMsg("..................| End CLI |..................\n\n");
	//CLI_ACTIVE = false;
}//--------------------------------------------------
void cmd_ok_handler(uint8_t num, char *values[])
{
//	******figure out how to handle help messages
//	if ( !(strcmp(values[0], "?")) )
//	{
//		CL_printMsg("\r\nThis is the help messg\r\n");
//	}
//	else
//	{
		CL_printMsg("\r\nSystem ok! \r\n");
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
	CL_printMsg("\r\nSum: %d\r\n", sum); 


   
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
		CL_printMsg("\r\nDifference = %d\r\n", initial);
	}
	else
	{

		CL_printMsg("\r\nError! Two numbers needed to subtract\r\n");
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
	CL_printMsg("\r\n");

}
void cmd_ledOff_handler(uint8_t num, char *values[])
{
	LL_GPIO_ResetOutputPin(GPIOG, LL_GPIO_PIN_14);
	CL_printMsg("\r\n");


}
void cmd_ledBlink_handler(uint8_t num, char *values[])
{
	blink(atoi(values[0]) , atoi(values[1]) );
	CL_printMsg("\r\n");


}