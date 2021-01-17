#pragma once

#include "CL_CONFIG.h"
#include "stdint.h"
#include "stdbool.h"
#include "string.h"
//---------------------| variables |-------------------

 
//function pointer to go inside struct 
//and point to cmd handler function
typedef  void(*cmd_handler)(uint8_t num, char *values[]);
typedef void(*registerCmd)(char *cmd, char delimeter, cmd_handler handler, char *help);


//this used to create a command, not sure of a more consice syntax to allow a sturct to point to iself as an element
 typedef struct _newCMD 
{
	//struct _commands *cmd;
	char            *command;
	char            delimeter;
	cmd_handler     cmdHandler;
	const char      *help;
	uint8_t			index;

}newCommand_type;
  
//will hold all commands and later be searched for a match
//when match is found the function in struct will be called
newCommand_type cmd_list[MAX_NUM_OF_COMMANDS];  //just initializing it to 3 for now



//control struct for the cli
typedef struct  
{
	char *prompt; 					//user define cli prompt
	char delimeter; 					//user defined delimeter for end of command usually \r (enter key)
	bool parsePending; 				//used to let main app know a parse is pending
	char charReceived; 				//stores received characters
	char cliMsg[MESSAGE_MAX]; 		//stores the complete received message
	uint8_t msgPtr; 					//this keeps track of how much we have incremtned the cli.cliMsg index
	registerCmd registerCommand; 	//function for user to register commands and handlers
}CL_cli_type;




//---------------------| Prototypes |-----------------------------------
void registerCommand(char *cmd, char delimeter, cmd_handler handler, char *help);
void charReceived(char data);
void parseCMD(CL_cli_type *cli);
void parseChar(CL_cli_type *cli);
void CL_cli_init(CL_cli_type *cli);
void printRegister(uint32_t regVal);
void updatePython(void);



