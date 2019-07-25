/*
 * addresses.c
 *
 *  Created on: 24 Jul 2019
 *      Author: me
 */
#include "addresses.h"

extern SPI_HandleTypeDef hspi1;
extern I2C_HandleTypeDef hi2c1;
uint8_t lastSeenAddress=0;

void AddressHeader(){
	globalVals.addressSubMode = (*bufferSPI_RX>>4) & 0x3;
	debugPrint("GOT ADDRESS MODE \n",(uint16_t*)globalVals.addressSubMode);

	if(globalVals.addressSubMode==WORKING || globalVals.addressSubMode==RESET){
		thisPanel.addressSet = FALSE;
		lastSeenAddress = 0;
		globalVals.dataMode = AWAITING_ADDRESS_CALLS;
		HeaderMode(FALSE);
	}
	else if(globalVals.addressSubMode==TRANSMIT){
		lastSeenAddress++;
		HeaderMode(FALSE);
	}
	else if(globalVals.addressSubMode==FINISH){
		FinishAddressMode();
	}
}

void SetAndSendAddress(){
	thisPanel.addressSet = TRUE;
	thisPanel.address = lastSeenAddress;
	lastSeenAddress++;
	//CREATE TRANSMIT ADDRESS HEADER TO INC ALL OTHER PANELS LASTSEENADDRESS TALLY
	*bufferSPI_TX = 64 | 48; //01110000
	*(bufferSPI_TX+1) = 0;
	EnableRS485TX();
	DataToEXT();
	globalVals.dataMode = SENDING_ADDRESS_CALL;
	HAL_SPI_Transmit_DMA(&hspi1, bufferSPI_TX, 2);
}

/*
 * EEPROM STRUCTURE IS AS SIMPLE AS IT GETS...1ST BYTE IS 1 OR 0 FOR WHETHER THIS PANEL HAS AN ADDRESS STORED.
 * IF IT DOES IT CAN LOAD IT FROM BYTE2.
 */

void FinishAddressMode(){
	//WRITE TO EEPROM...
	//INIT BYTE = DEVICE SELECT, 0 = START ADDRESS, 1 = ADDRESS SET, 2 = ADDRESS
	uint8_t initByte = 160;//10100000 - LSB OF 0 INDICATES WRITE MODE.
	uint8_t data[3] = {0,1,thisPanel.address};
	HAL_I2C_Master_Transmit(&hi2c1,initByte,data,3,1000);
	LoadAddress();
	HeaderMode(TRUE);
}

void LoadAddress(){
	//READ EEPROM...
	//TRANSMIT OUR DEVICE SELECT INIT BYTE AND OUR TARGET ADDRESS START POS.
	//RECEIVE 2 BYTES OF DATA FROM EEPROM
	uint8_t initByte = 161;//10100001 - LSB OF 1 INDICATES READ MODE.
	uint8_t addressByte = 0;
	uint8_t returnData[2];
	uint8_t resp = HAL_I2C_Master_Transmit(&hi2c1,initByte,addressByte,1,1000);
	if(resp == 0){
		resp = HAL_I2C_Master_Receive(&hi2c1,initByte,returnData,2,1000);
		if(resp == 0){
			if(returnData[0] == 0 || returnData[0] ==255){
				thisPanel.addressSet = FALSE;
			}
			else{
				thisPanel.addressSet = TRUE;
			}
			thisPanel.address = returnData[1];
		}
	}
}
