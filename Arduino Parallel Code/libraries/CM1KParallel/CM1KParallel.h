/*
 ****************************************************************************************
 *     
 *     COMPANY: Purdue University Calumet Department of Computer Information Technology 
 *
 * DESCRIPTION: CM1K Parallel Interfacing with Arduino-Due Micro controller 
 *
 * 	   VERSION: 1.0 (05/7/2015) 
 *
 ***************************************************************************************
 */

#ifndef CM1KPARALLEL_H
#define CM1KPARALLEL_H

#include "Arduino.h"

// Arduino PIN Mapping, Refer to CM1KPinMap.xlsx
#define CM_DATA_0        33
#define CM_DATA_1        34
#define CM_DATA_2        35
#define CM_DATA_3        36
#define CM_DATA_4        37
#define CM_DATA_5        38
#define CM_DATA_6        39
#define CM_DATA_7        40
#define CM_DATA_8        41

#define CM_DATA_9        51
#define CM_DATA_10        50
#define CM_DATA_11        49
#define CM_DATA_12        48
#define CM_DATA_13        47
#define CM_DATA_14        46
#define CM_DATA_15        45

#define CM_R_W_			26
#define CM_DS_          25

#define CM_ID           11
#define CM_UNC          12

#define CM_RDY			23
#define CM_G_CLK        24
#define CM_CS			27
#define CM_G_RESET      28
#define CM_DCI			29  // Not DC0
#define CM_S_CHIP       30
#define CM_I2C_EN       31

#define CM_REG_0        9
#define CM_REG_1        8
#define CM_REG_2        7
#define CM_REG_3        6
#define CM_REG_4        5

#define NOP __asm__ __volatile__ ("nop\n\t")
//uint32_t dwMask;



class ParallelClass {
public:
  ParallelClass() { };
  
  // CM1K Parallel Configuration Function 
  void begin();
  
  // CM1K Reset Function 
  void reset();

  // CM1K Parallel Write Function 
  void write(uint8_t RegisterID, uint16_t data) ;
  
  // CM1K Parallel Write  Function by Specific Write Type
  void write(uint8_t RegisterID, uint16_t data, uint8_t WriteType) ;
  
  // CM1K Parallel Read  Function 
  uint16_t read(uint8_t RegisterID);  
  
  // CM1K Parallel Read Function by Specific Read type
  uint16_t read(uint8_t RegisterID, uint8_t ReadType);  

private:

    // Read 2 Byte from 16 Arduino DATA pins 
	unsigned long read2Byte() ;

	// Write 2 Byte data to 16 Arduino DATA pins 
	void write2Byte(unsigned long bt);

	// Write Desired Register Value to 5 Arduino REGISTER pins
	void SetRegister(uint8_t cmRegister);
	
	// Direct Digital Read from a given Arduino pin 
	uint16_t DDRead(uint32_t ulPin);

	// Change 16 Arduino DATA Pins Mode to INPUT/OUTPUT
	void SetDataDir(uint8_t DIR);
	
	// Direct Digital Write to a given Arduino pin 
	void DDWrite(uint32_t ulPin, uint32_t Output);

	// Set the DATA STROBE PIN to HIGH 
	void DS_high();
	
	// Set the DATA STROBE PIN to LOW 
	void DS_low();
	
	// Set the CLOCK PIN to HIGH (TIC) 
	void CLK_high();

	// Set the CLOCK PIN to LOW (TOC)
	void CLK_low();

	// Sleep count (NOP for loop)
	void SleepCLK(long time);
	
};

extern ParallelClass cm1k;

#endif
