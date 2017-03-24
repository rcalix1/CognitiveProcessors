/**
 ****************************************************************************************
 * 
 *     COMPANY: CogniMem Technologies Incorporated
 *
 *   COPYRIGHT: Copyrighted 2013 CogniMem Technologies Incorporated, 
 *              all rights reserved.
 *
 *     LICENSE: CogniMem permits you to use, modify, and distribute this 
 *              file in accordance with the terms of the license agreement 
 *              accompanying it.
 *
 * DESCRIPTION: CogniSoft demonstration of interfacing with CogniMem 
 *              hardware using Arduino libraries 
 *
 ***************************************************************************************
 **/
 
 // include the necessary arduino libraries
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <math.h>

long startTime ;
long elapsedTime ;
long startTime2 ;
long elapsedTime2 ;
/////////////////////////////////////////////////////////////////////////////////
// ricardo read write
/*
File myFile;
int NumColsGlobal=5; //4 features + 1 class
int FeaturesGlobal=4;
*/
/////////////////////////////////////////////////////////////////////////////////
// darpa data

File myFile;
int NumRowsGlobalTrain=1450;
int NumRowsGlobalTest = 100;
int NumColsGlobal=41; //4 features + 1 class
int FeaturesGlobal=40;

////////////////////////////////////////////////////////////////////////////////


// ***************************************************************************************
//                           Start CogniSoft - Arduino API  (do not edit this code)
// ***************************************************************************************
  
// Define the constants needed for CogniMem hardware APIs
#define CM_REG_NEURON_CONTEXT		0x00
#define CM_REG_COMPONENT	        0x01
#define CM_REG_LAST_COMPONENT		0x02
#define CM_REG_DISTANCE			0x03 
#define CM_REG_COMPONENT_INDEX		0x03 
#define CM_REG_CATEGORY			0x04
#define CM_REG_ACTIVE_INF		0x05
#define CM_REG_MINIMUM_INF		0x06
#define CM_REG_MAXIMUM_INF		0x07
#define CM_REG_TEST_COMPONENT		0x08
#define CM_REG_TEST_CATEGORY		0x09
#define CM_REG_NEURON_ID		0x0A
#define CM_REG_GLOBAL_CONTEXT		0x0B
#define CM_REG_RESET_CHAIN		0x0C
#define CM_REG_NETWORK_STATUS		0x0D
#define CM_REG_POWER_SAVE		0x0E	
#define CM_REG_NEURON_COUNT		0x0F	
#define CM_REG_FORGET			0x0F


// Generic SDK constants
#define CM_MAX_VECTOR_LENGTH		256
#define CM_NEURON_BYTE_LENGTH		264

// Board specific constants 
#define PIN_BUS_BUSY                    2
#define PIN_LED                         13

// the address has to be shifted to the right one bit to make room for the Read/Write bit
#define CM_I2C_ADDRESS			(0x94 >> 1)

// Enable KNN: 1-Yes, 0-No.
int EN_KNN = 0;

/////////////////////////////////////////////////////////


volatile int vBusBusy = LOW;


// here is a list of the CogniMem SDK methods

// Retrieve the contents of the network and write them to the screen
// We are asking for the vectorLength so that we only display the interesting memory bytes 
void cognimem_displayNeurons(int vectorLength)
{
	int ncount = cognimem_getCommittedNeuronCount();
	unsigned char *neurons = (unsigned char*)malloc(ncount * CM_NEURON_BYTE_LENGTH);
	int i = 0;
	int i2 = 0;


	cognimem_readNeurons(neurons, ncount);
	
	printf("\n------------------- Begin Network Details -------------------");
	printf("\nCommitted Neuron Count: %u", ncount);

	for (i = 0; i < ncount; i++)
	{
		printf("\n\n NCR: %02X", neurons[(i * 264) + 1]);
		printf("  \n NID: %02X", i); 
		printf("\nCOMP:");
		
		for (i2 = 0; i2 < vectorLength; i2++) 
		{
			printf(" %02X", neurons[(i * 264) + 2 + i2]);
		}

		printf("\n AIF: %u", (neurons[(i * 264) + 258] << 8) + neurons[(i * 264) + 259]);
		printf("\n MIF: %u", (neurons[(i * 264) + 260] << 8) + neurons[(i * 264) + 261]); 
		printf("\n CAT: %u", (neurons[(i * 264) + 262] << 8) + neurons[(i * 264) + 263]);
	}
	
	printf("\n------------------- End Network Details -------------------\n\n");

	delete(neurons);
}

// Read a value from the specified address/register combination
short cognimem_read(const unsigned char cmRegister)
{    
    short result = 0;
    
    // make sure the system is not busy processing a previous command
    while(vBusBusy == HIGH) {  };    
    
    // send the read command
    Wire.beginTransmission(CM_I2C_ADDRESS);
  
    Wire.write(cmRegister);
    
    Wire.endTransmission();
    
    // read the response from the CM1K module
    Wire.requestFrom(CM_I2C_ADDRESS, 2);
    
    // keep waiting until we clock in all the bytes
    while(Wire.available() < 2) {  };
   
    return (Wire.read() << 8) + Wire.read();
}

// Write a value to the specified address/register combination
void cognimem_write(const unsigned char cmRegister, const short value)
{
    // make sure the system is not busy processing a previous command
    while(vBusBusy == HIGH) {  };
   
    byte data[] = { cmRegister, (value >> 8), (value & 0x00FF) };
    
    // send the write command 
    Wire.beginTransmission(CM_I2C_ADDRESS);
    
    Wire.write(data, 3);
    
    Wire.endTransmission();
}

// Reset the chip to its defaults, including clearing all committed neurons, and resetting the global MINIF and MAXIF
void cognimem_clear()
{
    cognimem_write(CM_REG_FORGET, 0x0000);
}

// Broadcast the passed in vector pattern to the current neuron context
int cognimem_broadcast(unsigned char vector[], int length)
{
    // write all but the last byte to the COMP register    
    for (int i = 0; i < length - 1; i++) 
    { 
        cognimem_write(CM_REG_COMPONENT, vector[i]);
    }

    // now write the last byte to the LCOMP register (this triggers the search-and-sort function in the chip)
    cognimem_write(CM_REG_LAST_COMPONENT, vector[length - 1]);

    return 0;
}

// Learn a vector using the current context value
int cognimem_learn(unsigned char vector[], int length, int category)
{
    // first broadcast the vector to the network
    cognimem_broadcast(vector, length);

    // now write to the CAT register, this will trigger a learn if the model is not represented in the network
    cognimem_write(CM_REG_CATEGORY, category);

    return 0;
}

// Recognize a vector, assume the current context
int cognimem_recognize(unsigned char vector[], int length)
{
    // first broadcast the vector to the network
    cognimem_broadcast(vector, length);

    // finally, check the network status register, to see if it was IDENTIFIED, UNCERTAIN, or UNKNOWN
    return cognimem_read(CM_REG_NETWORK_STATUS);
}

// Recognize a vector, in a specific context
int cognimem_recognize(unsigned char vector[], int length, int context)
{
    // first switch to the desired network context
    cognimem_write(CM_REG_GLOBAL_CONTEXT, context);
    
    // second broadcast the vector to the network
    cognimem_broadcast(vector, length);

    // finally, check the network status register, to see if it was IDENTIFIED, UNCERTAIN, or UNKNOWN
    return cognimem_read(CM_REG_NETWORK_STATUS);
}

// Retrieve the number of neurons that are being used from the CM1K
int cognimem_getCommittedNeuronCount()
{
    return cognimem_read(CM_REG_NEURON_COUNT);
}

// Change the Network Mode to Save-and-Restore and read out the contents of the network (committed neurons)
int cognimem_readNeurons(unsigned char neurons[], int ncount)
{
	int category = 0x00;
	int result = 0;
	int p = 0;
	int tempNsr = cognimem_read(CM_REG_NETWORK_STATUS);
	

	cognimem_write(CM_REG_NETWORK_STATUS, 0x0010);
	cognimem_write(CM_REG_RESET_CHAIN, 0x00);
	
	
	unsigned char neuron[CM_NEURON_BYTE_LENGTH];
	
	do
	{
		cognimem_readNeuron(neuron);

		category = ((neuron[262] << 8) + neuron[263]);

		if  (category == 0 || category == 0xFFFF)
		{
			break;
		}			
		else
		{
			ncount++;

			for (int i = 0; i < CM_NEURON_BYTE_LENGTH; i++)
			{
				neurons[p++] = neuron[i];
			}
		}
	}
	while (category != 0 && category !=0xFFFF);
	
	cognimem_write(CM_REG_NETWORK_STATUS, tempNsr);

	return (result);
}

// Read the content of the currently pointed at neuron and store in a 264 unsigned char array
// It is assumed that the network is already in SR mode and positioned on the proper neuron
void cognimem_readNeuron(unsigned char neuron[])
{
	int temp = cognimem_read(CM_REG_NEURON_CONTEXT); 

	neuron[0] = 0;
	neuron[1] = (unsigned char)(temp & 0x00FF);

	for (int i = 0; i < 265; i++) 
	{
		neuron[i + 2] = cognimem_read(CM_REG_COMPONENT) & 0x00FF;
	}

	temp = cognimem_read(CM_REG_ACTIVE_INF);
	neuron[258]=(unsigned char)((temp & 0xFF00) >> 8);
	neuron[259]=(unsigned char)(temp & 0x00FF);
	
	temp = cognimem_read(CM_REG_MINIMUM_INF);
	neuron[260]=(unsigned char)((temp & 0xFF00) >> 8);
	neuron[261]=(unsigned char)(temp & 0x00FF);

	temp = cognimem_read(CM_REG_CATEGORY);
	neuron[262]=(unsigned char)((temp & 0xFF00) >> 8);
	neuron[263]=(unsigned char)(temp & 0x00FF);
}


// Change the Network Mode to Save-and-Restore and restore the contents of the network (committed neurons)
// It is assumed that the neurons array is initialized to the correct length (ncount * 264)
int cognimem_writeNeurons(unsigned char neurons[], int ncount)
{
	unsigned char neuron[CM_NEURON_BYTE_LENGTH];

	int tempNsr = cognimem_read(CM_REG_NETWORK_STATUS);
	int tempGcr = cognimem_read(CM_REG_GLOBAL_CONTEXT);

 
	cognimem_write(CM_REG_FORGET, 0x00);
	cognimem_write(CM_REG_NETWORK_STATUS, 0x0010);
	cognimem_write(CM_REG_RESET_CHAIN, 0x00);

	for (int i = 0; i < ncount; i++)
	{
		memcpy(neuron, neurons + (i * CM_NEURON_BYTE_LENGTH), CM_NEURON_BYTE_LENGTH);
		
		cognimem_writeNeuron(neuron);
	}

	//to update the nselect of the newly loaded neurons
	cognimem_write(CM_REG_NETWORK_STATUS, tempNsr);
	cognimem_write(CM_REG_GLOBAL_CONTEXT, tempGcr); 
	
	return (cognimem_getCommittedNeuronCount());
}

// Write the content of the RTL neuron and store in a 264 unsigned char array
// It is assumed that the network is already in SR mode and positioned on the proper neuron
void cognimem_writeNeuron(unsigned char neuron[])
{
	cognimem_write(CM_REG_GLOBAL_CONTEXT, neuron[1]);

	for (int i = 0; i < CM_MAX_VECTOR_LENGTH; i++) 
	{ 
		cognimem_write(CM_REG_COMPONENT, neuron[2 + i]);
	}

	cognimem_write(CM_REG_ACTIVE_INF,  (neuron[258] << 8) + neuron[259]);
	cognimem_write(CM_REG_MINIMUM_INF, (neuron[260] << 8) + neuron[261]);
	cognimem_write(CM_REG_CATEGORY,    (neuron[262] << 8) + neuron[263]);
}

// ***************************************************************************************
//                           End CogniSoft - Arduino API  (do not edit this code)
// ***************************************************************************************



// ***************************************************************************************
//                           Begin Arduino Application Code (edit this code)
// ***************************************************************************************

// This method is called once following power-up/reset
void setup()
{
    // initialize the B_BUSY pin as an input, we need to know when the CM1K is not processing
    pinMode(PIN_BUS_BUSY, INPUT);
    
    // initialize the B_BUSYLED pin as an output, to indicate when the CM1K is processing
    pinMode(PIN_LED, OUTPUT);
    
    //attachInterrupt(PIN_BUS_BUSY, processInterupt, CHANGE);
    
    // ensure the CM1K is plugged into the primary I2C pins (20 and 21)
    Wire.begin();

    // initialize the UART bus for printing to the monitor
    Serial.begin(9600);
    
    Serial.println("\n\n================================");
    Serial.println("    CogniSoft - Arduino Due  v1.1 - Ricardo Calix modified  ");
    Serial.println("================================\n\n");
}

// use this interupt to update the status of the CM1K chip, so we know when we can talk to it or not
void processInterupt()
{
    vBusBusy = digitalRead(PIN_BUS_BUSY);
}

// This method is called repeatedly as long as the board is powered up
void loop()
{
    int result = 0;
    int controlRC = 0; //represents a switch that could be on or off to train model
   
    // update the busy indicator LED
    digitalWrite(PIN_LED, vBusBusy);    

    // reset the chip to its defaults
    cognimem_clear(); //ricardo
    
    printf("---------- Startin.g 'Single Read and Write' Test ---------------\n");
    result = testSingleReadAndWrite();
    printf("----------------------------------------------------------------");
    printf(" Test Results: %s\n\n", (result == 0 ? "PASSED" : " FAILED")); 
    
    /////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////
    if (controlRC == 0)
    {
         cognimem_clear(); //ricardo
         RicardoTrain();
         controlRC = 1;
    }
    
    ////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////
    printf("---------- Starting 'Simple Learn And Recognize' Test ----------\n");
    //result = testSimpleLearnAndRecognizeRicardo();
    result = RicardoTest();
    printf("----------------------------------------------------------------");
    printf(" Test Results: %s\n\n", (result == 0 ? "PASSED" : " FAILED")); 
        
    printf("...\n");
    
    // wait 1 minute and then re-run the test 
    delay(1000 * 60);
}

/////////////////////////////////////////////////////////////////////////
// Perform basic input/output functions on global network registers
int testSingleReadAndWrite()
{
    int result = 0;
    
    
    // get the default MINIF
    result = cognimem_read(CM_REG_MINIMUM_INF);  
    printf("minif: %02X\n", result, 2);
    if (result != 2) return -1;
    
    // update the MINIF (increment the default +1)
    cognimem_write(CM_REG_MINIMUM_INF, (result + 1));  
    
    // re-read the MINIF to confirm it was updated correctly
    result = cognimem_read(CM_REG_MINIMUM_INF);  
    printf("minif: %02X\n", result, 2);
    if (result != 3) return -1;
    
    // get the default MAXIF
    result = cognimem_read(CM_REG_MAXIMUM_INF);  
    printf("maxif: %02X\n", result, 2);
    if (result != 0x4000) return -1;
    
    // update the MAXIF (increment the default +1)
    cognimem_write(CM_REG_MAXIMUM_INF, (result + 1));  
    
    // re-read the MAXIF to confirm it was updated correctly
    result = cognimem_read(CM_REG_MAXIMUM_INF);  
    printf("maxif: %02X\n", result, 2);
    if (result != 0x4001) return -1;
    
    return 0;
}

////////////////////////////////////////////////////////////////////////////////////
//Train the cognimem chip model
int RicardoTrain()
{
  Serial.println("Starting Training ");
  
  unsigned char MatrixFromSD[NumColsGlobal];
  unsigned char current_vector[NumColsGlobal-1];
  int cell = 0;
   ////////////////////////////////////////////////////////////////////////////////////////////////////
  initialize_SD_card();
  myFile = SD.open("TR4096.TXT");
  int my_count = 0;
  int result;
  if (myFile) {
    Serial.println("the file was opened");
    //printf("Total Neuron Count: %u\n", result);
    cognimem_clear();
    int num_cols = FeaturesGlobal;
    int the_class;
    int aurora_count = 0;
    if ( EN_KNN == 1 ){cognimem_write(CM_REG_NETWORK_STATUS, 16);}

    startTime = millis();
    while (myFile.available()) {
        cell = myFile.parseInt();
        MatrixFromSD[aurora_count] = cell;
        aurora_count = aurora_count + 1;
        
        if (aurora_count > num_cols)
        {
          aurora_count = 0;
          
          for (int j=0; j<num_cols; j++)
          {
           
              current_vector[j] = MatrixFromSD[j];
              //Serial.println(MatrixFromSD[j]);
              
          }
         //Serial.println("********************");
          the_class = MatrixFromSD[num_cols];
          //Serial.println(the_class);
          //Serial.println("********************");
          cognimem_learn(current_vector, sizeof(current_vector), the_class);
          my_count = my_count + 1;
          Serial.println(my_count);
          
        }
    
    }//end while
    //Serial.println(my_count);
    elapsedTime = millis() - startTime;
    if ( EN_KNN == 1 ){cognimem_write(CM_REG_NETWORK_STATUS, 0);}     
    
    // close the file:
    myFile.close();
    Serial.print("End of Training ... ... ...");
  } else {
  	// if the file didn't open, print an error:
    Serial.println("error opening the file ricardo");
  }
    result = cognimem_getCommittedNeuronCount();
    printf("Committed Neuron Count: %u\n\n", result);
    
    Serial.print("Finished Training.. Elasped Time : ");
    Serial.print(elapsedTime);
    Serial.println("  milli Seconds");
 
    return 0;
  
  
}




///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Test the input sample given the cognimem model on the chip
int RicardoTest()
{
   Serial.println("Loading Testing Data....");
  unsigned char MatrixFromSD2[NumRowsGlobalTest][NumColsGlobal];
   ////////////////////////////////////////////////////////////////////////////////////////////////////
  initialize_SD_card();
  myFile = SD.open("TEST2K.TXT");
  if (myFile) {
    Serial.println("the file was opened");
    
    int cell = 0x00; 
    int super_index = 0;
    int col_hex = 0;
    int row_hex = 0;
    int control = 0;
    int green_i = 0;
    while (myFile.available()) {
   
        cell = myFile.parseInt();
        //Serial.println(cell);
        row_hex = round(super_index/NumColsGlobal);   
        col_hex = (super_index - (row_hex*NumColsGlobal));  
       
        if (row_hex > NumRowsGlobalTest)
        {
          break;
        }
        
        MatrixFromSD2[row_hex][col_hex] = cell;
        super_index = super_index + 1;
    }
    // close the file:
    myFile.close();
  } else {
  	// if the file didn't open, print an error:
    Serial.println("error opening the file ricardo");
  }
     ///////////////////////////////////////////////////////////////////////////////////////////////////
   //to view data
   /*
  for (int i =0; i<NumRowsGlobalTest; i++)
    {
      Serial.print(i);
      Serial.print(" = Vector  : [ ");
      for (int j=0; j<NumColsGlobal; j++)
      {
        
        Serial.print(MatrixFromSD2[i][j]);
        Serial.print(",");
      }
      Serial.println("]");
    }
    */
  ////////////////////////////////////////////////////////////////////////////////////////////////////

  Serial.println("Starting Testing "); 
  String PrintString;
  int result = 0;
    
    // we are retrieving a "k" of two (asking for the first two neurons that fired)
    int k = 1; //2;
    unsigned short dsts[] = { 0x0000, 0x0000 };  // distances
    unsigned short cats[] = { 0x0000, 0x0000 };  // categories
    unsigned short nids[] = { 0x0000, 0x0000 };  // neuron ids
   ////////////////////////////////////////////////////////////////////////////////////////////////////

    int num_samples = NumRowsGlobalTest;
    int num_cols = FeaturesGlobal;
    int the_class;

    unsigned char current_vector[num_cols];
    for (int i =0; i<num_cols; i++){ current_vector[i] = 0;}
    
    startTime = millis();
    for (int n =0; n<num_samples; n++)
    {
      for (int j=0; j<num_cols; j++)
      {
        current_vector[j] = MatrixFromSD2[n][j];
      }
      
      the_class = MatrixFromSD2[n][num_cols];
      
      if ( EN_KNN == 1 ){ cognimem_write(CM_REG_NETWORK_STATUS, 32);}

      result = cognimem_recognize(current_vector, sizeof(current_vector));
      
      for (int i = 0; i < k; i++)
      {
            dsts[i] = cognimem_read(CM_REG_DISTANCE);
            cats[i] = cognimem_read(CM_REG_CATEGORY);
            nids[i] = cognimem_read(CM_REG_NEURON_ID);
         //printf("Result %02u: ; Dist: %02u; Cat: %02u; Nid: %02u\n", (n + 1), dsts[i], cats[i], nids[i]);
      }
    }

    elapsedTime = millis() - startTime;
    Serial.print("Finished Testing.. Elasped Time : ");
    Serial.print(elapsedTime);
    Serial.println("   milli Seconds");
    
    return 0;
  
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void initialize_SD_card()
{
 
  
  pinMode(10, OUTPUT);
   
  if (!SD.begin(4)) {
    Serial.println("SD initialization failed!");
    return;
  }
   
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Perform sample learn and recognize functions on network to view/confirm behavior
int testSimpleLearnAndRecognizeRicardo()
{
    int result = 0;
    
    // we are retrieving a "k" of two (asking for the first two neurons that fired)
    int k = 2;
    unsigned short dsts[] = { 0x0000, 0x0000 };  // distances
    unsigned short cats[] = { 0x0000, 0x0000 };  // categories
    unsigned short nids[] = { 0x0000, 0x0000 };  // neuron ids

    unsigned char vector1[] = { 0x0B, 0x0B, 0x0B, 0x0B };
    unsigned char vector2[] = { 0x0F, 0x0F, 0x0F, 0x0F };
    unsigned char vector3[] = { 0x14, 0x14, 0x14, 0x14 };  
    unsigned char vector4[] = { 0x0C, 0x0C, 0x0C, 0x0C };
    unsigned char vector5[] = { 0x0E, 0x0E, 0x0E, 0x0E };
    unsigned char vector6[] = { 0x0D, 0x0D, 0x0D, 0x0D };
    unsigned char vector7[] = { 0x1E, 0x1E, 0x1E, 0x1E };
    unsigned char vector8[] = { 0x0D, 0x0D, 0x0D, 0x0D };
    unsigned char vector9[] = { 0x0C, 0x0C, 0x0C, 0x0C };
    unsigned char vectorRC[] = { 0x16, 0x16, 0x16, 0x16 };


    printf("Total Neuron Count: %u\n", result);
    cognimem_clear();

    printf("Learning vector { 0x0B, 0x0B, 0x0B, 0x0B } as Category 55...\n"); // 11,11,11,11
    cognimem_learn(vector1, sizeof(vector1), 55);
    printf("Learning vector { 0x0F, 0x0F, 0x0F, 0x0F } as Category 33...\n"); // 15,15,15,15
    cognimem_learn(vector2, sizeof(vector2), 33);
    printf("Learning vector { 0x14, 0x14, 0x14, 0x14 } as Category 100...\n");// 20,20,20,20
    cognimem_learn(vector3, sizeof(vector3), 100);
    
    //result = cognimem_getCommittedNeuronCount();
    //printf("Committed Neuron Count: %u\n\n", result);
    //if (result != 3) return -1;
    //printf("Observe the contents of the existing knowledgebase (KB)...\n");
    //cognimem_displayNeurons(sizeof(vector8));		

    //recognition after training
    printf("\nRecognize non equi-distant vector { 0x0C, 0x0C, 0x0C, 0x0C }...\n"); // 12,12,12,12
    result = cognimem_recognize(vector4, sizeof(vector4));
    
    printf("Network Status: %02X\n", result);
    for (int i = 0; i < k; i++)
    {
        dsts[i] = cognimem_read(CM_REG_DISTANCE);
        cats[i] = cognimem_read(CM_REG_CATEGORY);
        nids[i] = cognimem_read(CM_REG_NEURON_ID);
        printf("Result %02u: ; Dist: %02u; Cat: %02u; Nid: %02u\n", (i + 1), dsts[i], cats[i], nids[i]);
    }
    if ((result & 0x0004) != 4 || dsts[0] != 4 || dsts[1] != 12 || cats[0] != 55 || cats[1] != 33 || nids[0] != 1 || nids[1] != 2) return -1;
    
    
    printf("\nRecognize non equi-distant vector { 0x0E, 0x0E, 0x0E, 0x0E }...\n"); // 14,14,14,14
    result = cognimem_recognize(vector5, sizeof(vector5));
    printf("Network Status: %02X\n", result);
    for (int i = 0; i < k; i++)
    {
        dsts[i] = cognimem_read(CM_REG_DISTANCE);
        cats[i] = cognimem_read(CM_REG_CATEGORY);
        nids[i] = cognimem_read(CM_REG_NEURON_ID);
        printf("Result %02u: ; Dist: %02u; Cat: %02u; Nid: %02u\n", (i + 1), dsts[i], cats[i], nids[i]);
    }
    if ((result & 0x0004) != 4 || dsts[0] != 4 || dsts[1] != 12 || cats[0] != 33 || cats[1] != 55 || nids[0] != 2 || nids[1] != 1) return -1;
    
    
    printf("\nRecognize non equi-distant vector { 0x16, 0x16, 0x16, 0x16 }...\n"); // 22,22,22,22
    result = cognimem_recognize(vectorRC, sizeof(vectorRC));
    printf("Network Status: %02X\n", result);
    for (int i = 0; i < k; i++)
    {
        dsts[i] = cognimem_read(CM_REG_DISTANCE);
        cats[i] = cognimem_read(CM_REG_CATEGORY);
        nids[i] = cognimem_read(CM_REG_NEURON_ID);
        printf("Result %02u: ; Dist: %02u; Cat: %02u; Nid: %02u\n", (i + 1), dsts[i], cats[i], nids[i]);
    }
    if ((result & 0x0004) != 4 || dsts[0] != 4 || dsts[1] != 12 || cats[0] != 33 || cats[1] != 55 || nids[0] != 2 || nids[1] != 1) return -1;
    
    return 0;
}


// ***************************************************************************************
//                           End Arduino Application Code 
// ***************************************************************************************





