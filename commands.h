#ifndef COMMANDS_H
#define COMMANDS_H
#include "mbed.h"

// #define ASYNCHOUT 1 // sending output data asynchronously - TO DEBUG!! 

// communication data depth (max file size during LOAD)
#if defined TARGET_NUCLEO_L432KC
#define OUT_BUF_SIZE 20000
#define IN_BUF_SIZE 1000
#endif
#if defined TARGET_NUCLEO_L053R8
#define OUT_BUF_SIZE 1000
#define IN_BUF_SIZE 1000
#endif

#define SD_HOME "/sd/"

extern volatile uint8_t     inDataBuf[];
extern volatile uint8_t     outDataBuf[];
extern volatile uint16_t    inBufPosition;
extern volatile uint16_t    outDataPutPosition;

void ProcessCommand ( void ) ;

#endif
