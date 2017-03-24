/*
 ****************************************************************************************
 *     
 *     COMPANY: Purdue University Calumet Department of Computer Information Technology 
 *
 * DESCRIPTION: Cognimem Hardware Interfacing with Arduino CM1K Parallel Library 
 *
 * 	   VERSION: 1.0 (05/7/2015) 
 *
 ***************************************************************************************
 */

#ifndef CognimemPARALLEL_H
#define CognimemPARALLEL_H


// Define the constants needed for CogniMem hardware APIs
#define CM_REG_NEURON_CONTEXT		0x00
#define CM_REG_COMPONENT	        0x01
#define CM_REG_LAST_COMPONENT		0x02
#define CM_REG_DISTANCE				0x03 
#define CM_REG_COMPONENT_INDEX		0x03 
#define CM_REG_CATEGORY				0x04
#define CM_REG_ACTIVE_INF			0x05
#define CM_REG_MINIMUM_INF			0x06
#define CM_REG_MAXIMUM_INF			0x07
#define CM_REG_TEST_COMPONENT		0x08
#define CM_REG_TEST_CATEGORY		0x09
#define CM_REG_NEURON_ID			0x0A
#define CM_REG_GLOBAL_CONTEXT		0x0B
#define CM_REG_RESET_CHAIN			0x0C
#define CM_REG_NETWORK_STATUS		0x0D
#define CM_REG_POWER_SAVE			0x0E	
#define CM_REG_NEURON_COUNT			0x0F	
#define CM_REG_FORGET				0x0F

// Specific Write Clock Constants 
#define W_LCOMP		3
#define W_CAT		19
#define W_COMP		1

// Specific Read Clock Constants 
#define R_NSR		1
#define R_CAT		3
#define R_DST		18
#define R_NID		1

#define CM_REG_POWER_SAVE			0x0E	
#define CM_REG_NEURON_COUNT			0x0F	
#define CM_REG_FORGET				0x0F


// Generic SDK constants
#define CM_MAX_VECTOR_LENGTH		256
#define CM_NEURON_BYTE_LENGTH		264


class CognimemClass {
public:
	CognimemClass() { };
  
	// Reset the chip to its defaults, including clearing all committed neurons, and resetting the global MINIF and MAXIF
	void clear();

	// Learn a vector using the current context value
	int learn(unsigned char vector[], int length, int category) ;
	
	// Learn a vector, in a specific context
	int learn(unsigned char vector[], int length, int category, int context) ;
  
	// Broadcast the passed in vector pattern to the current neuron context
	int broadcast(unsigned char vector[], int length) ;
  
	// Recognize a vector, assume the current context
	int recognize(unsigned char vector[], int length);

	// Recognize a vector, in a specific context
	int recognize(unsigned char vector[], int length, int context);
  
	// Retrieve the contents of the network and write them to the screen
	// We are asking for the vectorLength so that we only display the interesting memory bytes 
	void displayNeurons(int vectorLength);
	
	// Retrieve the number of neurons that are being used from the CM1K
	int getCommittedNeuronCount();

	// Change the Network Mode to Save-and-Restore and read out the contents of the network (committed neurons)
	int readNeurons(unsigned char neurons[], int ncount);
	
	// Read the content of the currently pointed at neuron and store in a 264 unsigned char array
	// It is assumed that the network is already in SR mode and positioned on the proper neuron
	void readNeuron(unsigned char neuron[]);
  
	// Change the Network Mode to Save-and-Restore and restore the contents of the network (committed neurons)
	// It is assumed that the neurons array is initialized to the correct length (ncount * 264)
	int writeNeurons(unsigned char neurons[], int ncount);
	
	// Write the content of the RTL neuron and store in a 264 unsigned char array
	// It is assumed that the network is already in SR mode and positioned on the proper neuron
	void writeNeuron(unsigned char neuron[]);

};

extern CognimemClass cognimem;

#endif
