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

#include "CM1KParallel.h"


void ParallelClass::DS_high(){   
	// Set the DATA STROBE PIN to HIGH (PIO_SODR Sets High)
     g_APinDescription[CM_DS_].pPort->PIO_SODR = g_APinDescription[CM_DS_].ulPin;  
}

void ParallelClass::DS_low(){
	// Set the DATA STROBE PIN to LOW (PIO_CODR Clear High)
    g_APinDescription[CM_DS_].pPort->PIO_CODR = g_APinDescription[CM_DS_].ulPin;	
}

void ParallelClass::CLK_high(){
	// Set the CLOCK PIN to HIGH (TIC) (PIO_SODR Sets High)
	// PIO_SODR Direct Pin access is 24.4 nano second long.. Period is 48.8 ns( Highest Frequency = 1/(48.8 e-6). = 20.49 Mhz) 
	// To slow down the Clock Add more NOP function below (both for TIC and TOC).  Each NOP is 11.9 nano second 
	// When adding single NOP, period is = (24.4 + 11.9)*2 =72.6 ns (Highest Frequency = 1/(72.6 e-6). = 13.77 Mhz)
    g_APinDescription[CM_G_CLK].pPort->PIO_SODR = g_APinDescription[CM_G_CLK].ulPin;
	NOP;
	NOP;
	//NOP;
	//NOP;
}

void ParallelClass::CLK_low(){
	// Set the CLOCK PIN to LOW (TOC) (PIO_CODR Clears High)
	// PIO_CODR Direct Pin access is also 24.4 nano second long.. Period is (24.4+24.4) 48.8 ns( Highest Frequency = 1/(48.8 e-6). = 20.49 Mhz) 
	// To slow down the Clock Add more NOP function below (both for TIC and TOC).  Each NOP is 11.9 nano second 
	// When adding single NOP, period is = (24.4 + 11.9)*2(toc and toc) = 72.6 ns (Highest Frequency = 1/(72.6 e-6). = 13.77 Mhz)
    g_APinDescription[CM_G_CLK].pPort->PIO_CODR = g_APinDescription[CM_G_CLK].ulPin;
	NOP;
	NOP;
	//NOP;
	//NOP;
}

void ParallelClass::SleepCLK(long time){
	// Sleep Function Using NOP and for loop to archive pauses is nano seconds ( each NOP is 11.9 nano second long)
    for (int x = 0; x < time; x++){
      NOP;
    }
}

unsigned long ParallelClass::read2Byte() {
	// Read 2 Byte data from 16 Arduino DATA pins 

	// Read the status/values of the 16 DATA PINS in PIOC("0000 0000 0111 1111 0011 1111 1110") 
	// 16 selected pins location in PIOC -- > 0000 0000 0111 1111 0011 1111 1110
	// PIO_PDSR (Pin Data Status Register) read the PIN Values ("bt") on PIOC ("0000 0000 0111 1111 0011 1111 1110")
	unsigned long bt = PIOC->PIO_PDSR;
	
	// 28-bit "bt" is broken down to 16-bit "bt" using appropriate DATA pin location in PIOC.
	unsigned long bt2 =((((bt >> 12 ) & 0x7F ) << 9 ) | (( bt >> 1 ) & 0x1FF ) );
	return bt2;
}

void ParallelClass::write2Byte(unsigned long bt) {
	// Write 2 Byte data to 16 Arduino DATA pins 
	// 16-bit data "bt" is broken down to appropriate DATA pin location in PIOC using bitwise operations 
	// 16 selected pins location in PIOC -- > 0000 0000 0111 1111 0011 1111 1110
	// PIO_ODSR (Output Data Status Register) sets the PIN Values ("bt") on PIOC ("0000 0000 0111 1111 0011 1111 1110")
	PIOC->PIO_ODSR =  (((((bt >> 7) << 9 ) | (bt & 0x00FF) )) << 1 );
}

uint16_t ParallelClass::DDRead(uint32_t ulPin){
		// Direct Digital Read from a given Arduino pin using PIO_PDSR, Very fast !!
		// PIO_PDSR (Pin Data Status Register)
		
        Pio* pPio =g_APinDescription[ulPin].pPort;
        return pPio->PIO_PDSR & g_APinDescription[ulPin].ulPin; 
}

void ParallelClass::DDWrite(uint32_t ulPin, uint32_t Output){
    // Direct Digital Write to a given Arduino pin using PIO_SODR/PIO_CODR, Very fast !!
	
	//Select the PIN
    Pio* pPio =g_APinDescription[ulPin].pPort;
	
	// Clear Output Data Register using PIO_CODR (Clear Output Data Register)
	if (Output == LOW){
		pPio->PIO_CODR = g_APinDescription[ulPin].ulPin;  // PIN Needs to be in HEX format
	}
	
	// Set Output Data Register using PIO_SODR (Set Output Data Register)
	else if (Output == HIGH){
		pPio->PIO_SODR = g_APinDescription[ulPin].ulPin;   // PIN Needs to be in HEX format
	}
	
}

void ParallelClass::SetRegister(uint8_t cmRegister){	
		// Write Desired Register Value to 5 Arduino REGISTER pins
		// 5-bit cmRegister value is broken down to appropriate pins using bitwise operation
		
        DDWrite(CM_REG_0, cmRegister & 0x0001 );
        DDWrite(CM_REG_1, (cmRegister & 0x0002 ) >> 0x01 );
        DDWrite(CM_REG_2, (cmRegister & 0x0004 ) >> 0x02 );
        DDWrite(CM_REG_3, (cmRegister & 0x0008 ) >> 0x03  );
        DDWrite(CM_REG_4, (cmRegister & 0x00010 ) >> 0x04  );

}

void ParallelClass::SetDataDir(uint8_t DIR){
	// Change 16 Arduino DATA Pin Mode to INPUT/OUTPUT using low level codes ( Very Fast !!)
	
	// Set to INPUT using PIO_ODR (Output Disable Register)
	// Enable PIO control on the PINS using PIO_PER to ( PIO Enable Register)
	if (DIR == INPUT ){
          for (int i=33; i < 42; i++) {
                PIOC->PIO_ODR = g_APinDescription[i].ulPin;
                PIOC->PIO_PER = g_APinDescription[i].ulPin;
          }
          
          for (int i=45; i < 52; i++) {	
                PIOC->PIO_ODR = g_APinDescription[i].ulPin;
                PIOC->PIO_PER = g_APinDescription[i].ulPin;
          }
	}
	
	// Set to OUTPUT using PIO_OER (Output Enable Register)
	// Enable PIO control on the PINS using PIO_PER to ( PIO Enable Register)
	// Clear PIN Status using PIO_CODR ( Clear Output Data Register)
	else if (DIR == OUTPUT ){
           for (int i=33; i < 42; i++) {
                PIOC->PIO_OER = g_APinDescription[i].ulPin;
                PIOC->PIO_PER = g_APinDescription[i].ulPin;
                PIOC->PIO_CODR = g_APinDescription[i].ulPin;
           }
           
          for (int i=45; i < 52; i++) {
                PIOC->PIO_OER = g_APinDescription[i].ulPin;
                PIOC->PIO_PER = g_APinDescription[i].ulPin;
                PIOC->PIO_CODR = g_APinDescription[i].ulPin;
          }
	}
}

void ParallelClass::begin(void){	
	// CM1K Specific Parallel Configuration Function 
    
    // Data Stobe PIN to OUTPUT LOW 
    pinMode(CM_DS_,OUTPUT);  	
    digitalWrite(CM_DS_,LOW); 
    
	// RW PIN Default Read mode 
    pinMode(CM_R_W_,OUTPUT); 	
    digitalWrite(CM_R_W_,HIGH); 
    
	// CLOCK LOW at start
    pinMode(CM_G_CLK,OUTPUT);
    digitalWrite(CM_G_CLK,LOW);  
    
	// Always Disable S_CHIP
    pinMode(CM_S_CHIP,OUTPUT); 
    digitalWrite(CM_S_CHIP,LOW);  
    
	// Disable I2C 
    pinMode(CM_I2C_EN,OUTPUT);
    digitalWrite(CM_I2C_EN,LOW);   
	
	// Always Enable 1st CM1K
	pinMode(CM_DCI,OUTPUT);
    digitalWrite(CM_DCI,HIGH);  
    	
	// Default G_RESET HIGH ( LOW will reset the CM1K)	
    pinMode(CM_G_RESET,OUTPUT);  
    digitalWrite(CM_G_RESET,HIGH);
    
	// RDY, ID, UNC are to be read from CM1K
    pinMode(CM_RDY,INPUT);
    pinMode(CM_ID,INPUT);	
    pinMode(CM_UNC,INPUT);	

	// All DATA PINS are INPUT during configurations
    pinMode(CM_DATA_0,INPUT);        // DP33  PC1
    pinMode(CM_DATA_1,INPUT);         // DP34  PC2
    pinMode(CM_DATA_2,INPUT);         // DP35  PC3
    pinMode(CM_DATA_3,INPUT);         // DP36  PC4
    pinMode(CM_DATA_4,INPUT);         // DP37  PC5
    pinMode(CM_DATA_5,INPUT);         // DP38  PC6
    pinMode(CM_DATA_6,INPUT);         // DP39  PC7
    pinMode(CM_DATA_7,INPUT);         // DP40  PC8
    pinMode(CM_DATA_8,INPUT);         // DP41  PC9
    
    pinMode(CM_DATA_9,INPUT);      // DP51  PC12
    pinMode(CM_DATA_10,INPUT);     // DP50  PC13
    pinMode(CM_DATA_11,INPUT);     // DP49  PC14
    pinMode(CM_DATA_12,INPUT);     // DP48  PC15
    pinMode(CM_DATA_13,INPUT);     // DP47  PC16
    pinMode(CM_DATA_14,INPUT);     // DP46  PC17
    pinMode(CM_DATA_15,INPUT);     // DP45  PC18
    
	// All REGISTER PINS are OUTPUT as DEFAULT
    pinMode(CM_REG_0,OUTPUT);     // DP49  PC14
    digitalWrite(CM_REG_0,LOW); 
	
    pinMode(CM_REG_1,OUTPUT);     // DP48  PC15
    digitalWrite(CM_REG_1,LOW); 
	
    pinMode(CM_REG_2,OUTPUT);     // DP47  PC16
    digitalWrite(CM_REG_2,LOW); 
	
    pinMode(CM_REG_3,OUTPUT);     // DP46  PC17
    digitalWrite(CM_REG_3,LOW); 
	
    pinMode(CM_REG_4,OUTPUT);     // DP45  PC18
    digitalWrite(CM_REG_4,LOW); 
    
	// Clear all the PINS in PIOC ( All the DATA Pins are in PIOC)
    PIOC->PIO_OWDR = 0xFFFFFFFF;    // Clear out
	
	// Only Assign specific 16 DATA PINS to PIOC for Parallel READ and WRITE Access commands.
	// PIOC->PIO_ODSR to WRITE to selected 16  PINS at once
	// PIOC->PIO_PDSR to READ From selected 16  PINS at once
	// 0x7F3FE in Binary -> 0000 0000 0111 1111 0011 1111 1110 (appropriate 16 pins are selected)
	
    PIOC->PIO_OWER = 0x7F3FE;      // 7f3FE ( 16 bit)

	// change the DATA PINS directions to OUTPUT at start
	SetDataDir(OUTPUT);
}

void ParallelClass::reset(void){	
	// RESET CM1K be pulling down G_RESET PIN for more than 5 clock cycle
	DDWrite(CM_DS_,LOW); // DATA Strobe to normal

    for (int x = 0; x < 1024; x++)
	{
		// Give the CM1K time to start-up
		CLK_high();
		CLK_low();
	}


	DDWrite(CM_G_RESET,LOW); // Reset Mode 
    for (int x = 0; x < 32; x++)  // 32 CLOCK cycle > 5 clock required
	{
		CLK_high();
		CLK_low();
	}

	DDWrite(CM_G_RESET,HIGH); // Release Reset Mode
    for (int x = 0; x < 1024; x++)
	{
		// Give the CM1K time to start-up
		CLK_high();
		CLK_low();
	}
}

void ParallelClass::write(uint8_t RegisterID, uint16_t data){
	// Parallel Write Function to WRITE data to given Register in CM1K
	
	// Set CLK PIN to HIGH 
	CLK_high(); 
	
	// Set Desired Register value to REGISTER PINS
	SetRegister(RegisterID); 
	
	// Set the data values to DATA PINS in Arduino 
	write2Byte(data);  
	
	// Set CM1K to Write mode
	DDWrite(CM_R_W_,LOW); 
	
	// Assert the data-strobe line
	DS_high();
	
	// One clock is needed by CM1K to listen the DS_high pulse
    CLK_low();
    CLK_high();
	
	// Clear the PINS ? Needs investigation. This is important. 
	SetDataDir(INPUT);
	
    // Release the data-strobe line, and trigger a negative clock edge
	// As soon as negative clock edge is triggered data is latched by CM1K
	DS_low();  	
	CLK_low();
	
    // Set CM1K to Normal Read mode     
    DDWrite(CM_R_W_,HIGH); 
	
	// Provide Clock pulses to CM1K to process the data (Wait till CM1K is done)
	while ( DDRead(CM_RDY) == LOW) { 
        CLK_high();
        CLK_low();
	}
	
	
	// Set the DATA PIN directions to OUTPUT Default, so the Write function will be faster
	SetDataDir(OUTPUT); 
}

void ParallelClass::write(uint8_t RegisterID, uint16_t data, uint8_t WriteType){
	// Parallel Write Function by WriteType to WRITE data to given Register in CM1K
	// This function eliminate the while loop clock and 
	// provide specific number of clock needed by each write type
	
	// Set CLK PIN to HIGH 
	CLK_high(); 
	
	// Set Desired Register value to REGISTER PINS
	SetRegister(RegisterID); 
	
	// Set the data values to DATA PINS in Arduino 
	write2Byte(data);  
	
	// Set CM1K to Write mode
	DDWrite(CM_R_W_,LOW); 
	
	// Assert the data-strobe line
	DS_high();
	
	// One clock is needed by CM1K to listen the DS_high pulse
    CLK_low();
    CLK_high();
	
	// Clear the PINS ? Needs investigation. This is important. 
	SetDataDir(INPUT);
	
    // Release the data-strobe line, and trigger a negative clock edge
	// As soon as negative clock edge is triggered data is latched by CM1K
	DS_low();  	
	CLK_low();
	
    // Set CM1K to Normal Read mode     
    DDWrite(CM_R_W_,HIGH); 
	
	// Provide Clock pulses to CM1K to process the data (Wait till CM1K is done)
	// Eg: "writing CAT needs 19 cycles. 
	
    for (int i=0; i<WriteType; i++)
    {
        CLK_high();   
        CLK_low();
    }
	
	// Set the DATA PIN directions to OUTPUT Default, so the Write function will be faster
	SetDataDir(OUTPUT);
}

uint16_t ParallelClass::read(uint8_t RegisterID){
	// Parallel Read Function by ReadType to READ data from given Register in CM1K
	
	// Set CLK PIN to HIGH 
    CLK_high();
	
    // Set Arduino DATA BUS to Read mode    
	SetDataDir(INPUT); 
	
	// Set Desired Register value to REGISTER PINS
	SetRegister(RegisterID);
	
	//Set CM1K to Read mode
	DDWrite(CM_R_W_,HIGH); 
	
    // Assert the data-strobe line
	DS_high();

	// One clock is needed by CM1K to listen the DS_high pulse	
    CLK_low();
    CLK_high();
	
	// Trigger a negative clock edge and Release the data-strobe line
    CLK_low();
    DS_low();

	// Provide Clock pulses to CM1K to process the data (Wait till CM1K is done)
	while ( DDRead(CM_RDY) == LOW) {
        CLK_high();
        CLK_low();
	}

	// Set CLK PIN to HIGH 
    CLK_high();
	
	// Read the data, get the actual value of the Parallel Data Bus
	unsigned long data = read2Byte(); 
	
	// Once data read is complete, Set CLK PIN to LOW
    CLK_low();
	
	// Set the DATA PIN directions to OUTPUT Default, so the Write function will be faster
    SetDataDir(OUTPUT); 
	
    return data;
}

uint16_t ParallelClass::read(uint8_t RegisterID, uint8_t ReadType){
	// Parallel Read Function by ReadType to READ data from given Register in CM1K
	// This function eliminate the while loop clock and 
	// provide specific number of clock needed by each read type
	
	// Set CLK PIN to HIGH 
    CLK_high();
	
    // Set Arduino DATA BUS to Read mode    
	SetDataDir(INPUT); 
	
	// Set Desired Register value to REGISTER PINS
	SetRegister(RegisterID);
	
	//Set CM1K to Read mode
	DDWrite(CM_R_W_,HIGH); 
	
    // Assert the data-strobe line
	DS_high();

	// One clock is needed by CM1K to listen the DS_high pulse	
    CLK_low();
    CLK_high();
	
	// Trigger a negative clock edge and Release the data-strobe line
    CLK_low();
    DS_low();

	// Provide Clock pulses to CM1K to process the data (Wait till CM1K is done)
	// Eg: "Reading CAT needs 3 cycles. 
    for (int i=0; i<ReadType; i++)
    {
        CLK_high(); 
        CLK_low();
    }

	// Set CLK PIN to HIGH 
    CLK_high();
	
	// Read the data, get the actual value of the Parallel Data Bus
	unsigned long data = read2Byte(); 
	
	// Once data read is complete, Set CLK PIN to LOW
    CLK_low();
	
	// Set the DATA PIN directions to OUTPUT Default, so the Write function will be faster
    SetDataDir(OUTPUT); 
	
    return data;
}

// Create our object
ParallelClass cm1k = ParallelClass();
