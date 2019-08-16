/*
 * globals.h
 *
 *  Created on: 15 Jul 2019
 *      Author: me
 */
#include "inttypes.h"
#include "stdlib.h"
#include "main.h"

#ifndef GLOBALS_H_
#define GLOBALS_H_

#define DEBUGMODE 1						//ENABLE PRINTF OUTPUTS
#define DISABLEWATCHDOG 0					//FORCE REFRESH OF WATCHDOG
#define TRUE 1
#define FALSE 0

#define MAX_PANELS 128
#define MAX_PANEL_LEDS 256				//LIMITED TO 16x16
#define MAX_EDGE_LEDS 48
#define MAXTOUCHCHANNELS 16

#define OUTPUTBUFFERSIZE 768			//16*16 MAX SIZE = 256. DIV BY SCAN LINES (8 WORST CASE). * BY BAM BITS (8 WORST CASE). * 3 (RGB)
										//X2 BUFFERS FOR FLIP FLOP

#define MAX_PALETTE_SIZE 256			//MAX RESERVED MEMORY FOR PALETTE DATA. n * 3
#define MAX_GAMMA_R_SIZE 256			//RESERVED MEMORY FOR GAMMA LOOKUP. EACH CHANNEL CAN BE SEPERATE LENGTHS DUE TO 5/6/5 HC
#define MAX_GAMMA_G_SIZE 256
#define MAX_GAMMA_B_SIZE 256


#define RX_BUFFER_SIZE 1024				//MAX DATA FEED LENGTH IN A SINGLE FRAME.768 RGB, 144 EDGE, 112 RESERVED
#define TOUCH_BUFFER_SIZE 32			//MAX STORE FOR TOUCH DATA PER FRAME
#define PERIPHERAL_SIZE 256				//MAX SIZE FOR COMBINED PANEL PERIPHERALS SUCH AS TEMP SENSORS, MIC SENSORS AND LIGHT SENSORS THAT CAN BE FED IN SINGLE BATCH
										//NOTE THAT THE FIRST BIT IS RESERVED FOR EXT1 WHICH IS A DIGITAL 1|0. REMAINING 255 BITS IS DATA STREAM.

typedef enum{							//BUNCH OF DATA STATES. HELD INSIDE "DISPLAY" STRUCT
	READY,
	AWAITING_HEADER,
	PANEL_DATA_STREAM,					//WAITING FOR DATA ON RX...
	AWAITING_CONF_DATA,
	AWAITING_PALETTE_DATA,
	AWAITING_GAMMA_DATA,
	AWAITING_ADDRESS_CALLS,
	SENDING_PIXEL_DATA,
	SENDING_CONF_HEADER,
	SENDING_PALETTE_HEADER,
	SENDING_GAMMA_HEADER,
	SENDING_CONF_DATA,
	SENDING_PALETTE_DATA,
	SENDING_GAMMA_DATA,
	SENDING_ADDRESS_CALL,
	SENDING_DATA_STREAM,				//WERE TRANSMITTING OUR TOUCH OR PERIPHERAL DATA
	DEBUG,
	ERR_DATA,
	ERR_DMA,
	ERR_TIM
}DATASTATES;

typedef enum{
	DATA_MODE,
	ADDRESS_MODE,
	COLOUR_MODE,
	CONFIG_MODE
}HEADERSTATES;

typedef enum{
	WORKING,
	RESTART,
	FINISH,
	TRANSMIT
}ADDRESSMODE_STATES;

typedef enum{
	PANEL_INF,
	GAMMA
}CONFIGMODE_STATES;

typedef enum{
	TRUE_COLOUR,
	HIGH_COLOUR,
	PALETTE_COLOUR
}COLOUR_MODES;


struct GlobalRunVals {
	DATASTATES dataState;					//CURRENT ROUTINE STATE
	HEADERSTATES headerMode;				//LAST HEADER STATE RECEIVED
	ADDRESSMODE_STATES addressSubMode;		//SUB HEADER STATE FOR ADDR MODE
	CONFIGMODE_STATES configSubMode;		//SUB HEADER STATE FOR CONF MODE
	uint8_t totalPanels;					//TOTAL PANELS HOOKED UP
	uint8_t currentPanelID;					//CURRENT PANEL ID COUNT
	uint16_t currentPanelSize;				//SIZE IN BYTES OF EACH SEGMENT.
	uint16_t currentPanelReturnSize;		//SIZE IN BYTES OF EACH SEGMENT.
	uint8_t bufferFlipFlopState;			//FLIP FLOP TRACKER FOR RX BUFFER.
	uint8_t rs485RXMode;					//CURRENT STATE OF RS485 CHIP
	uint8_t dataToLEDs;						//DATA TO LEDS OR PI
	uint8_t peripheralDataPoint;			//TALLY FOR ADC PERIPHERAL STREAM
	uint8_t rxBufferLocation;				//0 = RX, 1 = RX ALT (FLIP FLOP RX BUFFERS).
	uint8_t touchRunning;
	uint8_t touchCalibrated;
	uint8_t pauseOutput;
} globalVals;

typedef struct pData {
	uint8_t dataRedBufA[MAX_PANEL_LEDS];				//REDS 24BIT Primary
	uint8_t dataGreenBufA[MAX_PANEL_LEDS];				//GREENS 24BIT Primary
	uint8_t dataBlueBufA[MAX_PANEL_LEDS];				//BLUES 24BIT Primary
	uint8_t dataRedBufB[MAX_PANEL_LEDS];				//REDS 24BIT Primary
	uint8_t dataGreenBufB[MAX_PANEL_LEDS];				//GREENS 24BIT Primary
	uint8_t dataBlueBufB[MAX_PANEL_LEDS];				//BLUES 24BIT Primary
	uint8_t edgeRedBuf[MAX_EDGE_LEDS];					//REDS 24BIT Primary
	uint8_t edgeGreenBuf[MAX_EDGE_LEDS];				//GREENS 24BIT Primary
	uint8_t edgeBlueBuf[MAX_EDGE_LEDS];					//BLUES 24BIT Primary
	uint8_t * pixelDataRBufA;
	uint8_t * pixelDataGBufA;
	uint8_t * pixelDataBBufA;
	uint8_t * pixelDataRBufB;
	uint8_t * pixelDataGBufB;
	uint8_t * pixelDataBBufB;
	uint8_t * edgeDataR;
	uint8_t * edgeDataG;
	uint8_t * edgeDataB;
} panelData;

typedef struct touchCh{
	uint32_t baseReading;
} touchData;

struct Panel {
	COLOUR_MODES colourMode;
	uint8_t paletteSize;					//256 MAX.
	uint8_t bamBits;						//
	uint8_t biasHC;							//0 = 5/6/5, 1 = 6/5/5, 2 = 5/5/6, 3 = 5/5/5
	uint16_t gammaSize;
	uint16_t gammaRLength;
	uint16_t gammaGLength;
	uint16_t gammaBLength;
	uint8_t address;				//0 - 255. 256 Panels MAX.
	uint8_t addressSet;				//HAVE WE SET OUR OWN ADDRESS? TOGGLED DURING ADDRESS MODE AND BOOT VIA EEPROM
	uint8_t width;					//WIDTH IN PIXELS
	uint8_t height;					//HEIGHT IN PIXELS
	uint16_t pixelCount;			//WIDTH * HEIGHT
	uint8_t orientation;			// 0 - 3. U, D, L, R
	uint8_t scanlines;				//0 = 1:8, 1 = 1:16

	uint8_t outputEn;				//1 = ON. 0 = OFF.
	uint8_t outputThrottle;			//0 = 100%, 1 = 80%...

	uint8_t touchActive;			//1 = ON. 0 = OFF.
	uint8_t touchChannels;			//CONTAINS ACTUAL COUNT NOT THE HEADER FLAG
	uint8_t touchBits;				//0 = 4BIT, 1=8BIT

	uint8_t edgeActive;
	uint8_t edgeDensity;
	uint8_t edgeThrottle;

	uint8_t periperalActive;		//0 = NO PERIPH, 1-7 NOT DEFINED YET
	uint8_t peripheralSettings;
	uint8_t peripheralSizeFlag;

	panelData data;
	touchData touchChannel[MAXTOUCHCHANNELS];
} thisPanel;

struct Rendering {
	uint32_t framesReceived;		//SIMPLE TRACKING TALLY FOR DEBUG PURPOSES.
	uint32_t returnedFrames;		//TRACKING FOR RETURNED FRAMES. DEBUG PURPOSES.
	uint32_t framesSeen;			//TRACKING DEBUG

	uint8_t renderFlipFlopState;				//SWITCH BETWEEN RENDER AND OUTPUT BUFFERS
	volatile uint8_t storedData;				//HAVE WE STORED OUR RECEIVED DATA WHEN WE WERE THE TARGET
	volatile uint8_t parsedData;				//HAVE WE PREPPED AND PARSED OUR DATA INTO BAM FORMAT
	volatile uint8_t returnDataMode;			//1 MEANS WERE RECEIVING RETURN DATA NOT LED DATA (OR NEED TO SEND OUR RETURN DATA)
	volatile uint8_t waitingToReturn;			//1 MEANS WE NEED TO SEND BACK OUR TOUCH DATA ASAP
	volatile uint8_t waitingProcessing;			//1 MEANS WE NEED TO PROCESS SOMETHING
	uint8_t requireEdgeUpdate;
	uint8_t edgeComplete;
	uint8_t returnSent;							//1 MEANS WEVE SENT - USED IN TRACKING DMA RESPONSE
	uint8_t pendingNext;

	uint8_t drawBufferLocation;					//FLIP FLOP BETWEEN RENDERING BUFFERS AND CURRENT LIVE DRAW BUFFER
	uint8_t drawBufferSwitchPending;			//AFTER FLIP FLOP WE SET A FLAG TO SWITCH OUT BUFFERS @ ROW 0 BAM 0 TO START NEW FRAME.

	uint8_t bamTimerStarted;		//HAVE WE STARED OUR RENDER/BAM TIMER?
	uint8_t currentBamBit;			//TRACK OUR CURRENT BAM STEP
	uint8_t currentRow;				//TRACK OUR ROW STEP
	volatile uint8_t firstRender;	//IS THIS OUR FIRST RENDER AFTER A RESTART?
	uint8_t immediateJump;			//TRACKING FOR BAM 0 FOR QUICKER TRANSFER
	uint16_t rowOffset;				//USED IN RENDER TRACKING LOOP
	uint16_t bamOffset;				//USED IN RENDER TRACKING LOOP
} renderState;


struct PanelInfLookup {
	uint16_t ledByteSize;					//w*h*colourMode
	uint16_t edgeByteSize;					//width per8ratio * height per8Ratio
	uint8_t touchByteSize;					//LENGTH OF DATA RETURNED FOR TOUCH AFTER SEND. CALC BASED ON 1: TOUCH ACTIVE, 2: TOUCH CHANNELS SET, 3 SENSETIVITY
	uint8_t periperalByteSize;				//LENDTH OF DATA RETURNED FOR PERIPHERALS AFTER SEND. CALC BASED ON 1: PERIP ACTIVE, 2: DATA RETURN SIZE
} panelInfoLookup[MAX_PANELS];



uint8_t spiBufferRX[RX_BUFFER_SIZE];		//WE FLIP FLOP OUR RX BUFFERS TO ENSURE WE DONT OVERWRITE DATA BEFORE WEVE PROCESSED IT
uint8_t spiBufferRXAlt[RX_BUFFER_SIZE];
uint8_t * bufferSPI_RX;

uint8_t spiBufferTX[TOUCH_BUFFER_SIZE+PERIPHERAL_SIZE];
uint8_t * bufferSPI_TX;


void debugPrint(char *data, uint16_t *params);
void EnsureDefaults(void);
void Initialise(void);


#endif /* GLOBALS_H_ */
