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

#define DEBUGMODE 1							//ENABLE PRINTF OUTPUTS
#define DISABLEWATCHDOG 0					//FORCE REFRESH OF WATCHDOG
#define TRUE 1
#define FALSE 0

#define MAX_PANELS 128
#define MAX_PANEL_LEDS 256
#define MAX_EDGE_LEDS 48
#define MAXTOUCHCHANNELS 16

#define MAX_PALETTE_SIZE 256
#define MAX_GAMMA_R_SIZE 256
#define MAX_GAMMA_G_SIZE 256
#define MAX_GAMMA_B_SIZE 256


#define RX_BUFFER_SIZE 2048				//MAX DATA FEED LENGTH IN A SINGLE BATCH
#define TX_BUFFER_SIZE 320				//TRANSMISSION MAX SIZE

#define TOUCH_BUFFER_SIZE 32			//MAX STORE FOR TOUCH DATA PER FRAME
#define PERIPHERAL_SIZE 256				//MAX SIZE FOR COMBINED PANEL PERIPHERALS SUCH AS TEMP SENSORS, MIC SENSORS AND LIGHT SENSORS THAT CAN BE FED IN SINGLE BATCH
										//NOTE THAT THE FIRST BIT IS RESERVED FOR EXT1 WHICH IS A DIGITAL 1|0. REMAINING 255 BITS IS DATA STREAM.

typedef enum{							//BUNCH OF DATA STATES. HELD INSIDE "DISPLAY" STRUCT
	READY,
	AWAITING_HEADER,
	PIXEL_DATA_STREAM,
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
	HEADERSTATES headerMode;					//LAST HEADER STATE RECEIVED
	ADDRESSMODE_STATES addressSubMode;		//SUB HEADER STATE FOR ADDR MODE
	CONFIGMODE_STATES configSubMode;		//SUB HEADER STATE FOR CONF MODE
	uint8_t totalPanels;
	uint8_t currentPanelID;					//CURRENT PANEL ID COUNT
	uint16_t currentPanelSize;				//SIZE IN BYTES OF EACH SEGMENT.
	uint16_t currentPanelReturnSize;		//SIZE IN BYTES OF EACH SEGMENT.
	uint8_t lastPanelID;					//TRACK WHICH PANEL FROM THE CURRENT SEGMENT WE LAST SENT
	uint8_t rs485RXMode;					//CURRENT STATE OF RS485 CHIP
	uint8_t dataToLEDs;						//DATA TO LEDS OR PI
	uint8_t peripheralDataPoint;			//TALLY FOR ADC PERIPHERAL STREAM
	uint8_t touchRunning;
	uint8_t touchCalibrated;
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
	uint8_t value;
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




struct PanelInfLookup {
	uint16_t ledByteSize;					//w*h*colourMode
	uint16_t edgeByteSize;					//width per8ratio * height per8Ratio
	uint8_t touchByteSize;					//LENGTH OF DATA RETURNED FOR TOUCH AFTER SEND. CALC BASED ON 1: TOUCH ACTIVE, 2: TOUCH CHANNELS SET, 3 SENSETIVITY
	uint8_t periperalByteSize;				//LENDTH OF DATA RETURNED FOR PERIPHERALS AFTER SEND. CALC BASED ON 1: PERIP ACTIVE, 2: DATA RETURN SIZE
} panelInfoLookup[MAX_PANELS];


uint8_t spiBufferRX[RX_BUFFER_SIZE];
uint8_t * bufferSPI_RX;

uint8_t spiBufferTX[TX_BUFFER_SIZE];
uint8_t * bufferSPI_TX;

uint8_t panelReturnData[TOUCH_BUFFER_SIZE+PERIPHERAL_SIZE];		//COMBINED RETURN DATA FROM ALL PANELS IN CONNECTED CHAIN. FIRST 2048 RESERVED FOR TOUCH.
uint8_t * returnData;

uint8_t paletteBuffer[MAX_PALETTE_SIZE*3];	//SET ASIDE MEMORY FOR MAX PALETTE SIZE (256 COLOURS * 3)
uint8_t *bufferPalette;						//POINTER TO PALETTE ARRAY

uint8_t gammaR[MAX_GAMMA_R_SIZE];
uint8_t gammaG[MAX_GAMMA_G_SIZE];
uint8_t gammaB[MAX_GAMMA_B_SIZE];
uint8_t *gammaDataR;
uint8_t *gammaDataG;
uint8_t *gammaDataB;


void debugPrint(char *data, uint16_t *params);
void EnsureDefaults(void);

#endif /* GLOBALS_H_ */
